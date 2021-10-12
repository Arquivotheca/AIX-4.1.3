static char sccsid[] = "@(#)88	1.9.1.3  src/bos/kernext/tm/tmscsidd.c, sysxtm, bos411, 9428A410j 5/26/94 15:21:06";
/*
 * COMPONENT_NAME: (SYSXTM) IBM SCSI Target Mode Device Driver
 *
 * FUNCTIONS:	tmscsi_config, tmscsi_open, tmscsi_close, tmscsi_read,
 *		tmscsi_recv_bufs, tmscsi_free_buf, tmscsi_async_func,
 *		tmscsi_write, tmscsi_ioctl, tmscsi_pass_thru,
 *		tmscsi_iodone, tmscsi_sleep, tmscsi_bldcmd,
 *		tmscsi_watchdog, tmscsi_select	
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/************************************************************************/
/*                                                                      */
/* COMPONENT:   SYSXTM                                                  */
/*                                                                      */
/* NAME:  tmscsidd.c                                                    */
/*                                                                      */
/*                                                                      */
/* FUNCTION:  SCSI Target Mode device driver.                           */
/*                                                                      */
/*       This device head acts as an interface to SCSI attached         */
/*       processor devices.  Data received from a remote initiator      */
/*       is passed from the SCSI Adapter Device Driver to this          */
/*       driver and is queued to be passed when the calling program     */
/*       issues a read system call.  In this sense, the read system     */
/*       call makes use of the SCSI target mode function.               */
/*                                                                      */
/*       Write system calls from a calling program are converted into   */
/*       SCSI Send commands and passed to the SCSI Adapter Device       */
/*       Driver for execution.  In this sense, the write system         */
/*       call makes use of the SCSI initiator mode function.            */
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

/* INCLUDED SYSTEM FILES */
#include <sys/types.h>
#include <sys/user.h>
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
#include <sys/priv.h>
#include <sys/watchdog.h>
#include <sys/dump.h>
#include <sys/time.h>
#include <sys/err_rec.h>
#include <sys/poll.h>
#include <sys/timer.h>
#include <sys/errids.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/trcmacros.h>
#include <sys/adspace.h>
#include <sys/xmem.h>
#include <sys/sysdma.h>
#include <sys/scsi.h>
#include <sys/tmscsi.h>
#include <sys/tmscsidd.h>
#include <sys/lockname.h>
#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#endif


/* END OF INCLUDED SYSTEM FILES  */

/* GLOBAL STATIC DECLARATIONS */

/* tm_list is a list to store defined target mode instances */
struct tm_dev_def *tm_list[TM_HASH_SIZE] = {NULL};
int     tms_configed = 0;	/* num instances configed */
int     tm_open_cnt = 0;	/* num instances opened */
int     tmd_lock = LOCK_AVAIL;	/* driver global lock */
struct cdt *tm_cdt = NULL;	/* driver component dump table pointer */
#ifdef DEBUG
int     tm_trace = 0;
#endif

/************************************************************************/
/* Function Declarations                                                */
/************************************************************************/
#ifndef	_NO_PROTO
int
tmscsi_config(dev_t devno, int cmd, struct uio * uiop);
int
tmscsi_open(dev_t devno, ulong devflag, int chan, int ext);
int
tmscsi_close(dev_t devno, int chan, int ext);
int
tmscsi_read(dev_t devno, struct uio * uiop, int chan, int ext);
void
tmscsi_recv_bufs(struct tm_buf * buf);
void
tmscsi_free_buf(struct tm_dev_def * tdp, struct tm_buf * buf);
void
tmscsi_async_func(struct sc_event_info * eventp);
int
tmscsi_write(dev_t devno, struct uio * uiop, int chan, int ext);
int
tmscsi_ioctl(dev_t devno, int op, int arg, int devflag,
	     int chan, int ext);
int
tmscsi_passthru(struct tm_dev_def * tdp, int op,
		int arg, int devflag);
void
tmscsi_iodone(struct buf * buf_ptr);
void
tmscsi_sleep(struct tm_cmd * cmd_ptr);
void
tmscsi_bldcmd(struct tm_dev_def * tdp, uchar opcode,
	      uchar resume_flag);
int
tmscsi_req_sens(struct tm_dev_def * tdp, uchar resume_flag);
void
tmscsi_log(struct tm_dev_def * tdp, uint errnum, uint errid,
	   uint error_type, uint data_ptr, uint data1,
	   uint data2);
struct tm_dev_def *
tmscsi_getptr(dev_t devno);
int
tmscsi_pin(caddr_t buf_addr, int *length, short seg_flag);
int
tmscsi_retry(struct tm_cmd * cmd_ptr);
int
tmscsi_watchdog(struct watchdog * tmw);
int
tmscsi_select(dev_t devno, ushort events, ushort * reventp, int chan);
struct cdt *
tmscsi_cdt_func(int arg);
#else
int     tmscsi_config();
int     tmscsi_open();
int     tmscsi_close();
int     tmscsi_read();
void    tmscsi_recv_bufs();
void    tmscsi_free_buf();
void    tmscsi_free_buf();
int     tmscsi_write();
int     tmscsi_ioctl();
int     tmscsi_passthru();
void    tmscsi_iodone();
void    tmscsi_sleep();
void    tmscsi_bldcmd();
int     tmscsi_req_sens();
void    tmscsi_log();
struct tm_dev_def *tmscsi_getptr();
int     tmscsi_pin();
int     tmscsi_retry();
int     tmscsi_watchdog();
int     tmscsi_select();
struct cdt *tmscsi_cdt_func();
#endif	_NO_PROTO

extern  nodev();



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_config							  */
/*                                                                        */
/* FUNCTION:  Target Mode Device Driver Initialization Routine.           */
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
/*	tdp		- target device info structure pointer		  */
/*	local_ddi	- local dds structure				  */
/*	devsw_struct	- devsw table structure				  */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    cmd    - CFG_INIT/CFG_TERM					  */
/*    uiop   - pointer to uio structure.				  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion			  */
/*			      ERRNO value otherwise			  */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*	EIO	- lock failed/uiomove error/devswadd err		  */
/*	EINVAL	- already inited/invalid configuration information.	  */
/*	ENOMEM	- xmalloc failed to allocate the required storage.        */
/*	EBUSY	- device busy						  */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  xmalloc                                   */
/*                              xmfree                                    */
/*                              lockl                                     */
/*                              unlockl                                   */
/*                              uiomove                                   */
/*				devswadd				  */
/*				devswdel				  */
/*                                                                        */
/**************************************************************************/
int
tmscsi_config(
	      dev_t devno,	/* major and minor device numbers */
	      int cmd,	/* dds operation to perform */
	      struct uio * uiop)
{
    int     i;
    uint    rc;
    uint    tm_offset;
    struct tm_ddi local_ddi;
    struct devsw devsw_struct;
    struct tm_dev_def *tdp, *back_ptr;
    struct tm_dev_def *temp_p, *tback_ptr;
    uint    cdt_size;
    struct cdt *old_cdt;

    DDHKWD2(HKWD_DD_TMSCSIDD, DD_ENTRY_CONFIG, 0, devno, cmd);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    tm_offset = minor(devno) & TM_HASH_NUMBER;
    tdp = tm_list[tm_offset];
    back_ptr = tdp;
    while (tdp != NULL) {
	if (tdp->devno == devno)
	    break;
	back_ptr = tdp;
	tdp = tdp->next;
    }
    switch (cmd) {
      case (CFG_INIT):	/* initializing device */
	if (tdp != NULL) {	/* device already initialized */
	    unlockl(&tmd_lock);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EINVAL, devno);
	    return (EINVAL);
	}
	/* get the dds info into local structure	 */
	rc = uiomove((caddr_t) & local_ddi, sizeof(struct tm_ddi),
		     UIO_WRITE, uiop);
	if (rc != 0) {
	    unlockl(&tmd_lock);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EIO, devno);
	    return (EIO);
	}

	/* allocate memory for tm_dev_def structure(target mode)	 */
	/* actually need to allocate two structures. one for target */
	/* mode and one for initiator mode. devno is initiator mode */
	/* device and devno+1 is target mode device.		 */


	for (i = 0; i < 2; i++) {
	    /* malloc on word boundary */
	    tdp = (struct tm_dev_def *) xmalloc(sizeof(struct tm_dev_def),
						2, pinned_heap);
	    if (tdp == NULL) {
		unlockl(&tmd_lock);
		DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, ENOMEM, devno);
		return (ENOMEM);
	    }
	    bzero(tdp, sizeof(struct tm_dev_def));

	    /* If this is the first device to be added to the system, */
	    /* add the target mode dd entry to the device switch table. */
	    if (tms_configed == 0) {
		devsw_struct.d_open = tmscsi_open;
		devsw_struct.d_close = tmscsi_close;
		devsw_struct.d_read = tmscsi_read;
		devsw_struct.d_write = tmscsi_write;
		devsw_struct.d_ioctl = tmscsi_ioctl;
		devsw_struct.d_strategy = nodev;
		devsw_struct.d_ttys = NULL;
		devsw_struct.d_select = tmscsi_select;
		devsw_struct.d_config = tmscsi_config;
		devsw_struct.d_print = nodev;
		devsw_struct.d_dump = nodev;
		devsw_struct.d_mpx = nodev;
		devsw_struct.d_revoke = nodev;
		devsw_struct.d_dsdptr = NULL;
		devsw_struct.d_selptr = 0;
#ifdef _POWER_MP
		devsw_struct.d_opts = DEV_MPSAFE;
#else
		devsw_struct.d_opts = 0;
#endif
		if ((rc = devswadd(devno, &devsw_struct)) != 0) {
		    (void) xmfree((caddr_t) tdp, pinned_heap);
		    unlockl(&tmd_lock);
		    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EIO, devno);
		    return (EIO);
		}
	    }

	    /* Allocate memory for component dump table.  One entry is needed
	       for the static device list, and then one additional entry for
	       each target mode structure.  */

	    cdt_size = (uint) sizeof(struct cdt_head) + (1 +
		      (1 + tms_configed)) * (uint) sizeof(struct cdt_entry);
	    old_cdt = tm_cdt;
	    tm_cdt = (struct cdt *) xmalloc(cdt_size, 2,
					    (heapaddr_t) pinned_heap);
	    if (tm_cdt == NULL) {
		/* can't build dump table; back out of config */
		tm_cdt = old_cdt;	/* retain old table */
		if (tms_configed == 0) {
		    (void) devswdel(devno);
		}
		(void) xmfree((caddr_t) tdp, pinned_heap);
		unlockl(&tmd_lock);
		DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, ENOMEM,
			devno);
		return (ENOMEM);
	    }
	    /* Free the old component dump table */
	    (void) xmfree((caddr_t) old_cdt, (heapaddr_t) pinned_heap);

	    bzero((caddr_t) tm_cdt, cdt_size);
	    tm_cdt->_cdt_head._cdt_magic = DMP_MAGIC;
	    strcpy(tm_cdt->_cdt_head._cdt_name, "tmscsi");
	    tm_cdt->_cdt_head._cdt_len = cdt_size;

	    /* copy ddi info into device structure	 */
	    bcopy(&local_ddi, &tdp->ddi, sizeof(struct tm_ddi));

	    /* initialize flags  */
	    tdp->opened = FALSE;
	    tdp->lock_word = LOCK_AVAIL;
	    tdp->next = NULL;
#ifdef _POWER_MP
            lock_alloc(&tdp->tm_mp_lock,LOCK_ALLOC_PIN, TMSCSI_LOCK_CLASS,
            minor(devno));
            simple_lock_init(&tdp->tm_mp_lock);
#endif


	    /* save device major/minor number */
	    tdp->devno = devno + i;
	    if (i == 0) {	/* initiator mode device  */
		if (back_ptr == NULL)
		    tm_list[tm_offset] = tdp;
		else
		    back_ptr->next = tdp;
	    }
	    else {	/* target mode device  */
		tdp->num_to_free = (tdp->ddi.num_bufs >> 2);
		tdp->num_to_wakeup = tdp->num_to_free;	/* 25% */
		temp_p = tm_list[tm_offset + i];
		back_ptr = temp_p;
		while (temp_p != NULL) {
		    back_ptr = temp_p;
		    temp_p = temp_p->next;
		}
		if (back_ptr == NULL)
		    tm_list[tm_offset + i] = tdp;
		else
		    back_ptr->next = tdp;
	    }
	    tms_configed++;
	}	/* end of for */
	break;
      case (CFG_TERM):	/* delete device */
	/* we need to look for both initiator and target mode devices */
	/* being not in use before we terminate the device driver and */
	/* release all its structures.				  */

	temp_p = tm_list[tm_offset + 1];
	tback_ptr = temp_p;
	while (temp_p != NULL) {
	    if (temp_p->devno == (devno + 1))
		break;
	    tback_ptr = temp_p;
	    temp_p = temp_p->next;
	}
	if ((tdp == NULL) || (temp_p == NULL)) {
	    unlockl(&tmd_lock);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, 0, devno);
	    return (0);	/* already deleted */
	}
	if ((tdp->opened) || (temp_p->opened)) {	/* devices opened */
	    unlockl(&tmd_lock);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EBUSY, devno);
	    return (EBUSY);
	}
	/* delete all structure associated with this device.   */
	if (back_ptr == tdp) /* removing the first structure in the list */
	    tm_list[tm_offset] = tdp->next;
	else
	    back_ptr->next = tdp->next;

	(void) xmfree((caddr_t) tdp, pinned_heap);

	if (tback_ptr == temp_p) /* removing first structure in the list */
	    tm_list[tm_offset + 1] = temp_p->next;
	else
	    tback_ptr->next = temp_p->next;

	(void) xmfree((caddr_t) temp_p, pinned_heap);

	/* subtract 2 to reflect fact that 2 (not 1) entry is gone */
	tms_configed -= 2;
	/* If this is the last tm_device to be deleted from the system */
	/* remove the device switch entry from the device switch table. */
	if (tms_configed == 0) {
	    (void) devswdel(devno);
	    /* Free the component dump table completely */
	    (void) xmfree((caddr_t) tm_cdt, (heapaddr_t) pinned_heap);
	}
	else {
	    /* Allocate memory for component dump table.  One entry is needed
	       for the static device list, and then one additional entry for
	       each target mode structure.  */

	    cdt_size = (uint) sizeof(struct cdt_head) + (1 +
			    tms_configed) * (uint) sizeof(struct cdt_entry);
	    old_cdt = tm_cdt;
	    tm_cdt = (struct cdt *) xmalloc(cdt_size, 2,
					    (heapaddr_t) pinned_heap);
	    if (tm_cdt == NULL) {
		/* can't build dump table; revert to old table */
		tm_cdt = old_cdt;	/* retain old table */
	    }
	    else {
		/* Free the old component dump table */
		(void) xmfree((caddr_t) old_cdt, (heapaddr_t) pinned_heap);
	    }

	    /* use the newly found size regardless if old or new table */
	    bzero((caddr_t) tm_cdt, cdt_size);
	    tm_cdt->_cdt_head._cdt_magic = DMP_MAGIC;
	    strcpy(tm_cdt->_cdt_head._cdt_name, "tmscsi");
	    tm_cdt->_cdt_head._cdt_len = cdt_size;
	}

	break;

      default:	/* invalid operation */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, EINVAL, devno);
	return (EINVAL);
    }
    unlockl(&tmd_lock);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CONFIG, 0, devno);
    return (0);
}  /* end tmscsi_config */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_open							  */
/*                                                                        */
/* FUNCTION:  Target Mode Device Open routine.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will open the target mode device to begin         */
/*         processing user requests.                                      */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*	tdp		- tm_dev_def structure pointer			  */
/*	stgt		- sc_strt_tgt structure				  */
/*	sc_event	- sc_event_struct				  */
/*                                                                        */
/* INPUTS:                                                                */
/*	devno		- major/minor number                              */
/*	devflag		- DREAD for read only, DWRITE for read/write      */
/*	chan		- not used (for multiplexed devices)              */
/*	ext		- defines mode required for open                  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, else ERRNO value	  */
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*	EINVAL - invalid open flag/not inited/already opened/wrong device */
/*	EAGAIN - lock failure						  */
/*	EPERM  - no dev config authority				  */
/*	ENOMEM - lacking memory resources				  */
/*	EIO    - i/o error occured					  */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              pin                                       */
/*                              unpin                                     */
/*                              pincode                                   */
/*                              unpincode                                 */
/*                              fp_opedev                                 */
/*                              fp_ioctl                                  */
/*                              fp_close                                  */
/*                                                                        */
/**************************************************************************/
int
tmscsi_open(
	    dev_t devno,	/* major/minor device number */
	    ulong devflag,	/* DREAD for read, DWRITE for write */
	    int chan,	/* channel number */
	    int ext)
{
    int     rc;
    uint    i, j;
    struct tm_dev_def *tdp;
    struct sc_strt_tgt stgt;
    struct sc_stop_tgt stop_tgt;
    struct sc_event_struct sc_event;
    struct devinfo info_struct;

    DDHKWD4(HKWD_DD_TMSCSIDD, DD_ENTRY_OPEN, 0, devno, devflag, chan, ext);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL) {	/* if not initialized */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EINVAL, devno);
	return (EINVAL);
    }

    rc = lockl(&tdp->lock_word, LOCK_NDELAY);
    if (rc == LOCK_FAIL) {
	(void) unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EBUSY, devno);
	return (EBUSY);
    }
    (void) unlockl(&tmd_lock);

    if (tdp->opened) {	/* if already opened */
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EBUSY, devno);
	return (EBUSY);
    }
    if (minor(devno) & 1) {
	if (devflag & DWRITE) {	/* attempting to open target for output */
	    unlockl(&tdp->lock_word);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EINVAL, devno);
	    return (EINVAL);
	}
    }
    else {
	if (devflag & DREAD) {	/* attempting to open initiator for input */
	    unlockl(&tdp->lock_word);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EINVAL, devno);
	    return (EINVAL);
	}
    }
    /* initialize tm device structure  */
    tdp->cmd_state = TM_IDLE;
    tdp->cmd_pb.scb.bufstruct.b_event = EVENT_NULL;
#ifdef _POWER_MP
    tdp->cmd_pb.scb.bufstruct.b_flags  |= B_MPSAFE;
    tdp->rscmd_pb.scb.bufstruct.b_flags |= B_MPSAFE;
#endif
    tdp->rscmd_pb.scb.bufstruct.b_event = EVENT_NULL;
    tdp->cmd_pb.scb.bp = NULL;
    tdp->rscmd_pb.scb.bp = NULL;
    tdp->previous_error = 0;
    tdp->event_flag = 0;
    tdp->async_flag = 0;
    tdp->async_events = 0;
    tdp->num_reads = 0;
    tdp->num_free_bufs = 0;
    tdp->num_bufs_qued = 0;
    /* num_to_free set in config_init */
    /* num_to_wakeup set in config_init */
    tdp->head_free = NULL;
    tdp->tail_free = NULL;
    tdp->head_rdbufs = NULL;
    tdp->tail_rdbufs = NULL;
    tdp->recv_event = EVENT_NULL;
    tdp->retry_delay = TM_DEFAULT_DELAY;
    tdp->retry_pending = FALSE;
    tdp->timeout_type = TM_SCALED_TIMEOUT;
    tdp->timeout_value = TM_DEFAULT_TIMEOUT;
    bzero(&tdp->iostatus, sizeof(struct tm_get_stat));

    /* pin the code so that a page fault doesn't occur    */
    /* while the code is executing in the interrupt level */
    rc = pincode((void *) tmscsi_iodone);
    if (rc != 0) {
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EIO, devno);
	return (EIO);
    }
    /* open the scsi adapter driver for this device	 */
    rc = fp_opendev(tdp->ddi.adapter_devno, FWRITE, NULL, NULL, &tdp->fp);
    if (rc != 0) {
	(void) unpincode((void *) tmscsi_iodone);
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, EIO, devno);
	return (EIO);
    }
    rc = fp_ioctl(tdp->fp, SCIOGTHW, NULL, NULL);
    if (rc != 0) {
       return(rc);
    }
    /* determine max transfer size for writes using IOCINFO	  */
    /* ioctl command. if ioctl call fails, use SC_MAXREQUEST. */

    rc = fp_ioctl(tdp->fp, IOCINFO, &info_struct, NULL);
    if (rc == 0)
	tdp->max_xfer = info_struct.un.scsi.max_transfer;
    else
	tdp->max_xfer = SC_MAXREQUEST;

    bzero(&sc_event, sizeof(struct sc_event_struct));

    /* start the scsi device to prepare for command reception */
    if (minor(devno) & 1) {	/* target */
	bzero(&stgt, sizeof(struct sc_strt_tgt));
	stgt.tm_correlator = (uint) tdp;
	stgt.id = tdp->ddi.scsi_id;
	stgt.lun = 0;
	stgt.num_bufs = tdp->ddi.num_bufs;
	stgt.buf_size = tdp->ddi.buf_size;
	stgt.recv_func = (void (*) ()) tmscsi_recv_bufs;
	rc = fp_ioctl(tdp->fp, SCIOSTARTTGT, &stgt, NULL);

	/* if starttgt is successful, save buf_free func address	 */

	if (rc == 0) {
	    if (stgt.free_func != NULL) {
		tdp->buf_free = stgt.free_func;
		sc_event.lun = 0;
		sc_event.mode = SC_TM_MODE;
	    }
	    else {
		rc = EIO;
	    }
	}
    }
    else {	/* initiator */
	rc = fp_ioctl(tdp->fp, SCIOSTART, IDLUN(tdp->ddi.scsi_id,
						tdp->ddi.lun_id), NULL);
	sc_event.lun = tdp->ddi.lun_id;
	sc_event.mode = SC_IM_MODE;
    }
    /* if SCIOSTART/SCIOSTARTTGT fails, fail tmscsi open also */
    if (rc != 0) {
	(void) fp_close(tdp->fp);
	(void) unpincode((void *) tmscsi_iodone);
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, rc, devno);
	return (rc);
    }

    /* Now register for async event notification	 */

    sc_event.id = tdp->ddi.scsi_id;
    sc_event.async_correlator = (uint) tdp;
    sc_event.async_func = (void (*) ()) tmscsi_async_func;
    rc = fp_ioctl(tdp->fp, SCIOEVENT, &sc_event, NULL);
    if (rc != 0) {
	if (minor(devno) & 1) {	/* minor number is odd - target */
	    bzero((caddr_t) & stop_tgt, sizeof(struct sc_stop_tgt));
	    stop_tgt.id = tdp->ddi.scsi_id;
	    stop_tgt.lun = 0;
	    (void) fp_ioctl(tdp->fp, SCIOSTOPTGT, &stop_tgt, NULL);
	}
	else {
	    (void) fp_ioctl(tdp->fp, SCIOSTOP,
			    IDLUN(tdp->ddi.scsi_id, tdp->ddi.lun_id), NULL);
	}
	(void) fp_close(tdp->fp);
	(void) unpincode((void *) tmscsi_iodone);
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, rc, devno);
	return (rc);
    }

    /* Register component dump table function on first open */
    if (tm_open_cnt == 0) {
	(void) dmp_add(tmscsi_cdt_func);
    }

    /* if initiator mode, initialize tm_timer for retry delays  */
    if ((minor(devno) & 1) == 0) {
	tdp->tm_wdog.dog.next = NULL;
	tdp->tm_wdog.dog.prev = NULL;
	tdp->tm_wdog.dog.func = tmscsi_watchdog;
	tdp->tm_wdog.dog.count = 0;
	tdp->tm_wdog.dog.restart = 0;
	tdp->tm_wdog.tdp = tdp;
#ifdef _POWER_MP
	while(w_init(&tdp->tm_wdog.dog));
#else
	w_init(&tdp->tm_wdog.dog);
#endif
    }

    tdp->opened = TRUE;	/* mark it opened */
    tm_open_cnt++;
    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_OPEN, 0, devno);
    return (0);
}  /* end tmscsi_open */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_close							  */
/*                                                                        */
/* FUNCTION:  Target Mode Device Driver Close routine.                    */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will close the target device. If the device is	  */
/*         in buffered mode, then an attempt will be made to flush the	  */
/*	   device's data buffers.  An error will be returned if this      */
/*         fails.							  */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*	tdp		- tm_dev_def structure pointer			  */
/*	stop_tgt	- sc_stop_tgt structure				  */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - defines mode required for open.                            */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not configured/not opened                           */
/*    EIO    - kernel service failure.                                    */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
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
tmscsi_close(
	     dev_t devno,	/* major/minor device number */
	     int chan,	/* channel number */
	     int ext)
{
    int     i, rc, saverc;
    struct tm_dev_def *tdp;
    struct sc_stop_tgt stop_tgt;

    DDHKWD1(HKWD_DD_TMSCSIDD, DD_ENTRY_CLOSE, 0, devno);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CLOSE, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL) {	/* device not initialized */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CLOSE, EINVAL, devno);
	return (EINVAL);
    }
    (void) lockl(&tdp->lock_word, LOCK_SHORT);
    unlockl(&tmd_lock);

    if ((tdp->devno != devno) || (!tdp->opened)) {	/* not opened */
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CLOSE, EINVAL, devno);
	return (EINVAL);
    }
    /* close scsi adapter device driver.     */
    if (minor(devno) & 1) {	/* minor number is odd - target */
	bzero((caddr_t) & stop_tgt, sizeof(struct sc_stop_tgt));
	stop_tgt.id = tdp->ddi.scsi_id;
	stop_tgt.lun = 0;
	(void) fp_ioctl(tdp->fp, SCIOSTOPTGT, &stop_tgt, NULL);
    }
    else {
	(void) fp_ioctl(tdp->fp, SCIOSTOP,
			IDLUN(tdp->ddi.scsi_id, tdp->ddi.lun_id), NULL);
#ifdef _POWER_MP
	while(w_clear(&tdp->tm_wdog.dog));
#else
	w_clear(&tdp->tm_wdog.dog);
#endif
    }
    (void) fp_close(tdp->fp, NULL);
    tdp->opened = FALSE;
    tdp->cmd_state = TM_IDLE;
    tdp->event_flag = 0;
    tdp->async_events = 0;
    tm_open_cnt--;
    if (tm_open_cnt == 0) {
	(void) dmp_del(tmscsi_cdt_func);
    }
    /* don't unpin the code until all events affecting the code have been
       quiesced */
    (void) unpincode((void *) tmscsi_iodone);

    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_CLOSE, 0, devno);
    return (0);
}  /* end tmscsi_close */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_read							  */
/*                                                                        */
/* FUNCTION:  Target Mode Driver read routine.                            */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will accept user commands to read from targets.   */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*    tdp	- tm_dev_def structure pointer				  */
/*    bp	- tm_buf structure pointer				  */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno  - major/minor number                                         */
/*    uiop   - pointer to uio structure which contains command information*/
/*    chan   - not used (for multiplexed devices).                        */
/*    ext    - extension parameter--not used.                             */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  0 for good completion, ERRNO value otherwise*/
/*                                                                        */
/* ERROR DESCRIPTION:                                                     */
/*    EINVAL - device not configured.                                     */
/*           - device not OPEN.                                           */
/*           - invalid paramter received.                                 */
/*    EINTR  - signal received						  */
/*    EIO    - kernel service failure.                                    */
/*           - target had a hard failure (can't continue).		  */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              xmattach                                  */
/*                              xmdetach                                  */
/*                              unpin                                     */
/*                              disable_lock                              */
/*                              unlock_enable                             */
/*                                                                        */
/**************************************************************************/
int
tmscsi_read(
	    dev_t devno,	/* major/minor device number */
	    struct uio * uiop,	/* pointer to the uio vector */
	    int chan,	/* channel number */
	    int ext)
{
    ulong   resid;
    uint    old_level;
    uint    more_data;
    int     i, rc, rcx;
    struct tm_buf *bp;
    struct tm_dev_def *tdp;
    uint    signal_occurred;
    uint    bytes_transferred;

    DDHKWD1(HKWD_DD_TMSCSIDD, DD_ENTRY_READ, 0, devno);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    if (!(minor(devno) & 1)) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EINVAL, devno);
	return (EINVAL);	/* invalid--not target minor number */
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EINVAL, devno);
	return (EINVAL);	/* invalid--device not initialized */
    }
    if ((rc = lockl(&tdp->lock_word, LOCK_SHORT)) != LOCK_SUCC) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    unlockl(&tmd_lock);
    if (!tdp->opened) {
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EINVAL, devno);
	return (EINVAL);	/* invalid--device not opened */
    }

    /* clear uiop->uio_offset every time to work around  2 GByte */
    /* filesize limitation.                                      */
    uiop->uio_offset = 0;

    tdp->head_free = NULL;	/* head freelist to be passed to buf_free */
    tdp->tail_free = NULL;	/* tail freelist to be passed to buf_free */
    tdp->num_free_bufs = 0;	/* num of buffers in the free list */
    tdp->num_reads++;	/* no. of reads to this device */
    tdp->previous_error = 0;	/* clear pending error */

    signal_occurred = FALSE;
    bytes_transferred = FALSE;

    /* main read logic loop */
    for (;;) {
	/* must disable to INTCLASS2 because this driver's receive buf */
	/* routine runs on that level when called from adapter driver! */
	old_level = disable_lock(tdp->ddi.int_prior, &tdp->tm_mp_lock);
	while (tdp->head_rdbufs == NULL) {	/* no bufs qued */
	    /* if no data received, we sleep,  	 */
	    /* unless DNDELAY is specified.	 */

	    /* if DNDELAY specified, return */
	    if (uiop->uio_fmode & FNDELAY) {
		/* check for tm_bufs which need freeing */
		if (tdp->head_free != NULL)
		    tmscsi_free_buf(tdp, NULL);
        	/* must re-enable prior to unlock */
		unlock_enable(old_level, &tdp->tm_mp_lock);
		tdp->cmd_state = TM_IDLE;	/* reset cmd state */
		if (bytes_transferred == FALSE) {
		    /* a non-blocking read would block as no data  */
		    /* has been received, so return with EAGAIN    */
		    unlockl(&tdp->lock_word);
		    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EAGAIN, devno);
		    return (EAGAIN);
		}
		/* if got here, some data was received for a non-blocking */
		/* read, so check for error status and exit.  Note that   */
		/* we give back what data has been received even though   */
		/* the Send command may not have ended yet.  This is done */
		/* to be consistent with the general definition of a un*x */
		/* non-blocking read system call.  If Send cmd not ended, */
		/* another read must be sent from caller to get that data */
		rc = 0;
		if (tdp->previous_error) {
		    switch (tdp->previous_error) {
		      case TM_SFW_ERR:
			rc = EIO;	/* setup error return */
			break;
		      case TM_SYS_ERR:
			rc = EFAULT;	/* setup error return */
			break;
		      default:
			rc = EIO;	/* setup error return */
			break;
		    }	/* end switch */
		    tdp->previous_error = 0;	/* clear pending error */
		}
		unlockl(&tdp->lock_word);
		DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, rc, devno);
		return (rc);
	    }
	    /* set up initial cmd state for a waiting read */
	    tdp->cmd_state = TM_READ_ACTIVE;
	    rc = e_sleep_thread(&tdp->recv_event, &tdp->tm_mp_lock, 
                                LOCK_HANDLER | INTERRUPTIBLE);
	    if (rc == THREAD_INTERRUPTED) {	/* if signal occurred */
		signal_occurred = TRUE;
	    }
	    if (signal_occurred) {
		if (tdp->head_free != NULL)
		    tmscsi_free_buf(tdp, NULL);
		unlock_enable(old_level, &tdp->tm_mp_lock);
		tdp->cmd_state = TM_IDLE;	/* reset cmd state */
		unlockl(&tdp->lock_word);
		DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, EINTR, devno);
		return (EINTR);
	    }

	}	/* end while no buf queued */
	unlock_enable(old_level, &tdp->tm_mp_lock);

	/* if we get here, one or more buffers are available which */
	/* we now process.                                         */
	/* no need to do xmattach or to pin user buffer as we	 */
	/* are in the context of user process doing the read.	 */
	/* Just do a uiomove from buffers to the user buffers.	 */

	bp = tdp->head_rdbufs;

	more_data = bp->user_flag & TM_MORE_DATA;

	/* default return code */
	rc = 0;

	if (!(bp->user_flag & TM_HASDATA)) {
	    /* head of rdbuf is an invalid buffer.   */
	    /* log as unknown SFW err */
	    tmscsi_log(tdp, 1, ERRID_TMSCSI_UNKN_SFW_ERR, SC_TM_MODE,
		       (uint) bp, (uint) bp->user_flag, 0);

	    bp->data_len = 0;	/* invalid buf--throw away data */
	    tdp->cmd_state |= TM_CMD_ERROR;
	    /* check for end of Send Command */
	    if (!more_data) {
		/* this is end of Send Cmd, so free tm_buf */
		/* and return error.                       */
		rc = EIO;
	    }
	    else {
		/* save the fact that we had this error so that it */
		/* can be reported on the end of this Send command */
		if (!(tdp->previous_error)) {
		    tdp->previous_error = TM_SFW_ERR;
		}
	    }
	}

	/* check for error reported on this Send cmd */
	if (bp->status_validity == SC_ADAPTER_ERROR) {
	    switch (bp->general_card_status) {
	      case SC_SCSI_BUS_FAULT:
	      case SC_NO_DEVICE_RESPONSE:
		/* log as temp read i/o err */
		tmscsi_log(tdp, 2, ERRID_TMSCSI_READ_ERR, SC_TM_MODE,
			   (uint) bp, 0, 0);
		break;
	      case SC_CMD_TIMEOUT:	/* not possible for target mode */
	      case SC_HOST_IO_BUS_ERR:
	      case SC_ADAPTER_HDW_FAILURE:
	      case SC_ADAPTER_SFW_FAILURE:
	      case SC_FUSE_OR_TERMINAL_PWR:
	      case SC_SCSI_BUS_RESET:
		/* these are logged by the adapter driver, no further action
		   required */
		break;
	      default:
		break;
	    }	/* end switch */
	    bp->data_len = 0;	/* throw away data */
	    tdp->cmd_state |= TM_CMD_ERROR;
	    rc = EIO;
	}

	bytes_transferred = TRUE;
	resid = uiop->uio_resid;	/* amount of data left to xfer */

	/* uiomove also updates fields in the uio structure  */
	/* viz. uio_resid, uio_offset etc. It also moves min */
	/* of bp->data_len and the uio_resid data.           */
	/* If bp->data_len is zero, uiomove returns 0.       */
	rcx = uiomove(bp->data_addr, bp->data_len, UIO_READ, uiop);
	if (rcx != 0) {
	    /* handle failure of uiomove */
	    /* log temp failed system call */
	    tmscsi_log(tdp, 3, ERRID_TMSCSI_UNKN_SFW_ERR, SC_TM_MODE,
		       (uint) bp, (uint) rcx, 0);

	    bp->data_len = 0;	/* throw away data */
	    tdp->cmd_state |= TM_CMD_ERROR;

	    if (!more_data) {
		/* this is end of Send Cmd, so free tm_buf */
		/* and return error.                       */
		rc = EFAULT;
	    }
	    else {
		/* save the fact that we had this error so that it */
		/* can be reported on the end of this Send command */
		if (!(tdp->previous_error)) {
		    tdp->previous_error = TM_SYS_ERR;
		}
	    }
	}	/* end if uiomove rcx != 0 */

	if (resid < bp->data_len) {	/* bcount < data_len */
	    bp->data_addr += resid;	/* update offset */
	    bp->data_len -= resid;	/* update data len */
	}
	else {	/* bcount >= data_len */
	    bp->data_len = 0;
	    bp->user_flag &= ~(TM_HASDATA);	/* can be reenabled */
	}

	if (bp->data_len == 0) {
	    bp->user_flag = 0;
	    old_level = disable_lock(tdp->ddi.int_prior, &tdp->tm_mp_lock);
	    tdp->head_rdbufs = tdp->head_rdbufs->next;
	    if (tdp->head_rdbufs == NULL)
		tdp->tail_rdbufs = NULL;
	    bp->next = NULL;
            unlock_enable(old_level, &tdp->tm_mp_lock);
	    (void) tmscsi_free_buf(tdp, bp);
	}
	if ((rc) || (more_data == 0) || (uiop->uio_resid <= 0))
	    break;
    }	/* end of for-ever */

    /* free remaining buffers on the free list  */
    if (tdp->head_free != NULL)
	tmscsi_free_buf(tdp, NULL);

    if (tdp->previous_error) {
	switch (tdp->previous_error) {
	  case TM_SFW_ERR:
	    rc = EIO;	/* setup error return */
	    break;
	  case TM_SYS_ERR:
	    rc = EFAULT;	/* setup error return */
	    break;
	  default:
	    rc = EIO;	/* setup error return */
	    break;
	}	/* end switch */
	tdp->previous_error = 0;	/* clear pending error */
    }
    tdp->cmd_state = TM_IDLE;	/* reset cmd state */
    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_READ, rc, devno);
    return (rc);
}  /* end tmscsi_read */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_recv_bufs						  */
/*                                                                        */
/* FUNCTION:  This function is directly called by the adapter driver	  */
/*	      with a list of buffers filled with send command data.	  */
/*	      This function adds these buffers to their respective 	  */
/*	      device's read_buf list.					  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine called by adapter driver   */
/*	with a list of send command data buffers.			  */
/*									  */
/* DATA STRUCTURES:							  */
/*	tdp - tm_dev_def structure pointer				  */
/* INPUTS:                                                                */
/*	buf : list buffers with send command data for a device		  */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
void
tmscsi_recv_bufs(struct tm_buf * buf)
{
    uint     old_level;
    uchar   end_of_send;
    struct tm_dev_def *tdp;

    tdp = (struct tm_dev_def *) buf->tm_correlator;
    old_level = disable_lock(tdp->ddi.int_prior, &tdp->tm_mp_lock);
    if (tdp->opened == TRUE) {
	end_of_send = FALSE;
	while (buf != NULL) {
	    if (tdp->head_rdbufs == NULL)
		tdp->head_rdbufs = buf;
	    else
		tdp->tail_rdbufs->next = buf;
	    tdp->tail_rdbufs = buf;
	    tdp->num_bufs_qued++;	/* bufs qued after last wakeup */
	    if (!(buf->user_flag & TM_MORE_DATA)) {
		/* end of send command */
		end_of_send = TRUE;
	    }
	    buf = buf->next;
	    tdp->tail_rdbufs->next = NULL;
	}	/* end while */

	/* do a selnotify if POLLIN pending */

	if (tdp->head_rdbufs) {
	   if (tdp->event_flag & POLLIN) {
	       tdp->event_flag &= ~POLLIN;
	       selnotify(tdp->devno, 0, POLLIN);
	   }
        }

	if ((end_of_send) || (tdp->num_bufs_qued >= tdp->num_to_wakeup)) {
	    tdp->num_bufs_qued = 0;
	    end_of_send = FALSE;
	    e_wakeup(&tdp->recv_event);
	}
    }
    else {
	/* Since the device is not opened, we should not get data yet. */
	/* Cannot call adapter's buf_free routine, since that addr may */
	/* not be valid if we aren't open.  Do not log, as this is not */
	/* an explicit error.  Count occurrences of such and go on.    */
	tdp->count_non_open_bufs++;
    }
    unlock_enable(old_level, &tdp->tm_mp_lock);
}  /* end tmscsi_recv_bufs */

/************************************************************************/
/* NAME    : tmscsi_free_buf					 	*/
/*									*/
/* FUNCTION:  This function is an internal function called by read	*/
/*	      to free buffers after read. It builds a free list and	*/
/*	      calls adapter buffer free function			*/
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*      This routine is an internal subroutine called by device read	*/
/*	function in process level only.					*/
/*									*/
/* DATA STRUCTURES:							*/
/*									*/
/* INPUTS:                                                              */
/*	tdp	- tm_dev_def structure pointer				*/
/*	buf	- buffer to be freed					*/
/*                                                                      */
/* RETURN VALUE DESCRIPTION:  none                                      */
/*                                                                      */
/* EXTERNAL PROCEDURES CALLED: none                                     */
/************************************************************************/
void
tmscsi_free_buf(
		struct tm_dev_def * tdp,
		struct tm_buf * buf)
{

    if (buf != NULL) {
	if (tdp->head_free == NULL)
	    tdp->head_free = buf;
	else
	    tdp->tail_free->next = buf;
	tdp->tail_free = buf;
	tdp->num_free_bufs++;
    }

    /* wake up if enough bufs accumlated or end of read */
    if ((tdp->num_free_bufs > tdp->num_to_free) ||
	(buf == NULL)) {
	(tdp->buf_free) (tdp->head_free);
	tdp->head_free = NULL;
	tdp->tail_free = NULL;
	tdp->num_free_bufs = 0;
    }
}  /* end tmscsi_free_buf */

/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_async_func						  */
/*                                                                        */
/* FUNCTION:  This function is directly called by the adapter driver	  */
/*	      with a pointer to sc_event_info structure, upon occurance	  */
/*	      any asynchronus event to be notified to tmscsidd driver.	  */
/*	      This function saves these async events and passes on to	  */
/*	      the application on request.				  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine called by adapter driver   */
/*	with a pointer to the sc_event_info structure.			  */
/*									  */
/* DATA STRUCTURES:							  */
/*	tdp - tm_dev_def structure pointer				  */
/* INPUTS:                                                                */
/* 	eventp - sc_event_info structure pointer			  */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
void
tmscsi_async_func(struct sc_event_info * eventp)
{
    struct tm_dev_def *tdp;

    tdp = (struct tm_dev_def *) eventp->async_correlator;
    if (tdp->opened == TRUE) {
	tdp->async_events |= eventp->events;

	/* selnotify if POLLPRI pending	 */

	if (tdp->event_flag & POLLPRI) {
	    tdp->event_flag &= ~POLLPRI;	/* clear tdp->event_flag bit */
	    selnotify(tdp->devno, 0, POLLPRI);
	}
    }
}  /* tmscsi_async_func */

/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_write							  */
/*                                                                        */
/* FUNCTION:  Target Mode Driver write routine.                           */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine can be called by a process.                          */
/*      It can page fault only if called under a process and the stack    */
/*      is not pinned.                                                    */
/*                                                                        */
/* NOTES:  This routine will accept user commands to write to target.     */
/*                                                                        */
/* DATA STRUCTURES:                                                       */
/*	tdp		- tm_dev_def structure pointer			  */
/*	iovp		- iovec structure pointer			  */
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
/*    EINVAL - device not configured                                      */
/*           - invalid paramter received.                                 */
/*           - device not OPEN.                                           */
/*	     - transfer length too long					  */
/*	     - can not pin memory					  */
/*    EIO    - kernel service failure.                                    */
/*           - target had a hard failure (can't continue).		  */
/*           - hardware or software error SCSI command failure.           */
/*           - SCSI adapter device driver failure.                        */
/*           - device power failure.                                      */
/*           - general error case for SCSI command failure.               */
/*    ENOMEM - not enough memory during data gathering			  */
/*    EFAULT - uiomove err/xmattach err/xmdetach err			  */
/*    ENXIO  - SCSI Check condition occurred                              */
/*    EBUSY  - scsi reservation conflict				  */
/*    EINTR  - signal received						  */
/*    ETIMEDOUT  - a SCSI command has timed out before completion.        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:  lockl                                     */
/*                              unlockl                                   */
/*                              xmattach                                  */
/*                              xmdetach                                  */
/*                              unpin                                     */
/*                                                                        */
/**************************************************************************/
tmscsi_write(
	     dev_t devno,	/* major/minor device number */
	     struct uio * uiop,	/* pointer to the uio vector */
	     int chan,	/* channel number */
	     int ext)
{  /* extended parameter--not used */
    caddr_t buffer_addr;
    struct iovec *iovp;
    struct tm_dev_def *tdp;
    int     i, rc, count_written, saved_iovcnt;

    DDHKWD1(HKWD_DD_TMSCSIDD, DD_ENTRY_WRITE, 0, devno);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    if (minor(devno) & 1) {	/* attempting write to target minor */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	return (EINVAL);
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL) {	/* device not initialized */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	return (EINVAL);
    }
    if ((rc = lockl(&tdp->lock_word, LOCK_SHORT)) != LOCK_SUCC) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    unlockl(&tmd_lock);
    if (!tdp->opened) {	/* device not opened */
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	return (EINVAL);
    }

    /* clear uiop->uio_offset to work around 2 Gbytes file size      */
    /* limitations.						      */

    uiop->uio_offset = 0;

    /* Setup pointers to the iovec structure to obtain data transfer  */
    /* length and the user buffer address.                            */
    rc = 0;
    iovp = uiop->uio_iov;
    buffer_addr = iovp->iov_base;
    if (uiop->uio_resid > tdp->max_xfer) {
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	return (EINVAL);
    }
    tdp->cmd_pb.scb.resvd1 = 0;
    /* if doing a gather write pass uio ptr in sc_buf resvd1 field */
    saved_iovcnt = uiop->uio_iovcnt;
    if (uiop->uio_iovcnt > 1) {
	tdp->cmd_pb.scb.resvd1 = (uint) uiop;
    }
    else {	/* not a gather write */
	tdp->cmd_pb.scb.bufstruct.b_xmemd.aspace_id = XMEM_INVAL;
	rc = xmattach(iovp->iov_base, iovp->iov_len,
		      &tdp->cmd_pb.scb.bufstruct.b_xmemd, uiop->uio_segflg);
	if (rc != XMEM_SUCC) {
	    unlockl(&tdp->lock_word);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	    return (EINVAL);
	}

	/* pin user buffer 	 */
	rc = pinu(buffer_addr, uiop->uio_resid, uiop->uio_segflg);
	if (rc != 0) {
	    (void) xmdetach(&tdp->cmd_pb.scb.bufstruct.b_xmemd);
	    unlockl(&tdp->lock_word);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EINVAL, devno);
	    return (EINVAL);
	}
    }
    tmscsi_bldcmd(tdp, SCSI_WRITE, SC_RESUME);
    tdp->cmd_state = TM_WRT_ACTIVE;

    tdp->cmd_pb.scb.bufstruct.b_flags &= ~(B_READ | B_DONE);
    tdp->cmd_pb.scb.bufstruct.b_bcount = uiop->uio_resid;
    tdp->cmd_pb.scb.bufstruct.b_un.b_addr = (char *) buffer_addr;
    tdp->cmd_pb.scb.scsi_command.scsi_cmd.scsi_bytes[0]
	= ((uiop->uio_resid >> 16) & 0xff);
    tdp->cmd_pb.scb.scsi_command.scsi_cmd.scsi_bytes[1]
	= ((uiop->uio_resid >> 8) & 0xff);
    tdp->cmd_pb.scb.scsi_command.scsi_cmd.scsi_bytes[2]
	= (uiop->uio_resid & 0xff);

    if (tdp->timeout_type == TM_SCALED_TIMEOUT) {
	tdp->cmd_pb.scb.timeout_value = (((uiop->uio_resid / 65536) + 1)
					 * (tdp->timeout_value));
    }
    else {
	tdp->cmd_pb.scb.timeout_value = tdp->timeout_value;
    }
    (void) devstrat(&tdp->cmd_pb.scb.bufstruct);

    tmscsi_sleep(&tdp->cmd_pb);

    /* Done OR TIMEOUT	 */

    if (saved_iovcnt == 1) {
	rc = unpinu(tdp->cmd_pb.scb.bufstruct.b_un.b_addr,
		    tdp->cmd_pb.scb.bufstruct.b_bcount, uiop->uio_segflg);
	if (rc != 0) {
	    /* must attempt to continue here, since xmdetach must run, log
	       and continue */
	    /* log temp failed system call */
	    tmscsi_log(tdp, 4, ERRID_TMSCSI_UNKN_SFW_ERR, SC_IM_MODE,
		       (uint) & tdp->cmd_pb, (uint) rc, 0);
	}

	/* calculate number of bytes written */
	count_written = tdp->cmd_pb.scb.bufstruct.b_bcount -
	    tdp->cmd_pb.scb.bufstruct.b_resid;

	/* The total resid is decremented by the length  */
	/* of this transfer			      */
	uiop->uio_resid -= count_written;

	rc = xmdetach(&tdp->cmd_pb.scb.bufstruct.b_xmemd);
	if (rc != XMEM_SUCC) {
	    unlockl(&tdp->lock_word);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, EFAULT, devno);
	    return (EFAULT);
	}
    }
    rc = tdp->cmd_pb.scb.bufstruct.b_error;
    if ((uiop->uio_resid > 0) && (rc == 0)) {
	rc = EIO;
    }
    if (rc == 0) {
	if (tdp->cmd_pb.retry_flag == TRUE) {
	    /* command was successful, but retries were required-- log the
	       error as temporary */
	    /* log temp recovered err */
	    tmscsi_log(tdp, 5, ERRID_TMSCSI_RECVRD_ERR, SC_IM_MODE,
		       (uint) & tdp->cmd_pb,
		       (uint) tdp->cmd_pb.retry_count, 0);
	}
    }
    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_WRITE, rc, devno);
    return (rc);
}  /* end tmscsi_write */



/************************************************************************/
/*                                                                      */
/* NAME:   tmscsi_ioctl						        */
/*                                                                      */
/* FUNCTION: SCSI Target Device Driver IOCTL Routine.                   */
/*              This routine supports the following ioctl commands:     */
/*              IOCINFO   - returns info about the device such as	*/
/*		 	    no. of read buffers and size		*/
/*              TMGETSENS - does request sense and			*/
/*			    returns the sense data			*/
/*              TMIOCMD   - issues the scsi cmd described in the arg	*/
/*			    and returns the cmd status.			*/
/*		TMIOSTAT  - get detail status of previous cmd		*/
/*		TMCHGIMPARM - change IM parameters			*/
/*		TMIOEVENT - get status of async events			*/
/*              TMIORESET - issue bus device reset to target            */
/*              TMIOASYNC - switch to asynchronous mode for intiator    */
/*                          mode transfers                              */
/*                                                                      */
/* EXECUTION ENVIRONMENT:                                               */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/* DATA STRUCTURES:                                                     */
/*	tdp		- tm_dev_def pointer				*/
/*	scp		- sc_buf pointer				*/
/*	tmscinfo	- devinfo structure				*/
/*	sens_info	- tm_get_sens structure				*/
/*	im_parm		- tm_chg_im_parm structure			*/
/*	event_info	- tm_event_info structure			*/
/*                                                                      */
/* INPUTS: 								*/
/*	devno   - device major/minor number                     	*/
/*	op      - operation to be performed                     	*/
/*	arg     - address of user argument structure            	*/
/*	flag    - File parameter word                           	*/
/*	chan    - unused (will be zero)		                        */
/*	ext     - extended parameter                	                */
/*                                                                      */
/* RETURN VALUE DESCRIPTION: 	                                        */
/*              The errno values listed in the 'error description'      */
/*              will be returned to the caller if there is an error.    */
/*              Otherwise a value of zero will be returned to indicate  */
/*              successful completion.                                  */
/*                                                                      */
/* ERROR DESCRIPTION:                                                   */
/*	EINVAL  - not inited/not opened/ invalid parameter		*/
/*	EIO     - i/o error occured					*/
/*	EPERM	- no dev config authority				*/
/* 	EFAULT  - copyin err/copyout err/xmattach err/xmdetach err	*/
/*	ETIMEDOUT - cmd timed out					*/
/*									*/
/* EXTERNAL PROCEDURES CALLED:                                          */
/*              lockl           unlockl                                 */
/*              copyin          copyout                                 */
/*              pin             unpin                                   */
/*              xmattach        xmdetach                                */
/*                                                                      */
/************************************************************************/
int
tmscsi_ioctl(
	     dev_t devno,
	     int op,
	     int arg,
	     int devflag,
	     int chan,
	     int ext)
{
    int     errno = 0;
    int     rc, dev, hash_key, sens_len;
    struct tm_dev_def *tdp;
    struct sc_buf *scp;
    struct devinfo tmscinfo;
    struct tm_get_sens sens_info;
    struct tm_chg_im_parm im_parm;
    struct tm_event_info event_info;

    DDHKWD5(HKWD_DD_TMSCSIDD, DD_ENTRY_IOCTL, 0, devno, op, devflag,
	    chan, ext);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IOCTL, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL) {	/* not initialized */
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IOCTL, EINVAL, devno);
	return (EINVAL);
    }
    if ((rc = lockl(&tdp->lock_word, LOCK_SHORT)) != LOCK_SUCC) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IOCTL, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    unlockl(&tmd_lock);
    if (!tdp->opened) {	/* not opened */
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IOCTL, EINVAL, devno);
	return (EINVAL);
    }

    /* Determine operation to perform */
    switch (op) {
      case IOCINFO:	/* get device information */
	tmscinfo.devtype = DD_TMSCSI;	/* say this is a scsi device */
	tmscinfo.flags = 0;
	tmscinfo.devsubtype = DS_TM;	/* SCSI target mode device */
	tmscinfo.un.tmscsi.scsi_id = tdp->ddi.scsi_id;
	tmscinfo.un.tmscsi.lun_id = tdp->ddi.lun_id;
	tmscinfo.un.tmscsi.num_bufs = tdp->ddi.num_bufs;
	tmscinfo.un.tmscsi.buf_size = tdp->ddi.buf_size;
	tmscinfo.un.tmscsi.adapter_devno = tdp->ddi.adapter_devno;
	tmscinfo.un.tmscsi.max_transfer = tdp->max_xfer;

	if (!(devflag & DKERNEL)) {	/* for a user process */
	    if (copyout(&tmscinfo, (char *) arg, sizeof(struct devinfo)))
		errno = EFAULT;
	}
	else {	/* for a kernel process *//* s, d, l */
	    bcopy(&tmscinfo, (char *) arg, sizeof(struct devinfo));
	}
	break;

      case TMGETSENS:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
	/* Copyin arg structure	 */
	if (devflag & DKERNEL)
	    bcopy((caddr_t) arg, (caddr_t) & sens_info,
		  sizeof(struct tm_get_sens));
	else {
	    if (copyin(arg, &sens_info, sizeof(struct tm_get_sens)) != 0) {
		errno = EFAULT;
		break;
	    }
	}

	/* always do request sense command with 255 bytes	 */
	/* but transfer to application minimum of what it	 */
	/* asked and what we got.				 */
	(void) tmscsi_req_sens(tdp, SC_RESUME);

	scp = &tdp->rscmd_pb.scb;
	errno = scp->bufstruct.b_error;
	if (errno != 0) {
	    break;	/* failed */
	}
	else {
	    if (tdp->rscmd_pb.retry_flag == TRUE) {
		/* command was successful, but retries were required-- log
		   the error as temporary */
		/* log temp recovered err */
		tmscsi_log(tdp, 6, ERRID_TMSCSI_RECVRD_ERR, SC_IM_MODE,
			   (uint) & tdp->rscmd_pb,
			   (uint) tdp->rscmd_pb.retry_count, 0);
	    }
	}
	sens_len = 255 - scp->bufstruct.b_resid;
	if (sens_info.len < sens_len)
	    sens_len = sens_info.len;

	if (!(devflag & DKERNEL)) {	/* for a user process */
	    if (copyout(tdp->rs_buf, (char *) sens_info.addr, sens_len))
		errno = EFAULT;
	}
	else {	/* for a kernel process *//* s, d, l */
	    bcopy(tdp->rs_buf, (char *) sens_info.addr, sens_len);
	}
	break;

      case TMIOCMD:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
	if (privcheck(DEV_CONFIG) != 0) {	/* need dev_config auth. */
	    errno = EPERM;
	    break;
	}
	errno = tmscsi_passthru(tdp, op, arg, devflag);
	break;

      case TMIOSTAT:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
	if (!(devflag & DKERNEL)) {	/* for a user process */
	    if (copyout(&tdp->iostatus, (char *) arg,
			sizeof(struct tm_get_stat)))
		errno = EFAULT;
	}
	else {	/* for a kernel process */
	    bcopy(&tdp->iostatus, (char *) arg,
		  sizeof(struct tm_get_stat));
	}
	break;

      case TMCHGIMPARM:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
	/* Copyin arg structure */
	if (devflag & DKERNEL)
	    bcopy((caddr_t) arg, (caddr_t) & im_parm,
		  sizeof(struct tm_chg_im_parm));
	else {
	    if (copyin(arg, &im_parm,
		       sizeof(struct tm_chg_im_parm)) != 0) {
		errno = EFAULT;
	    }
	}

	if (im_parm.chg_option & TM_CHG_RETRY_DELAY) {
	    tdp->retry_delay = im_parm.new_delay;
	}
	if (im_parm.chg_option & TM_CHG_SEND_TIMEOUT) {
	    tdp->timeout_value = im_parm.new_timeout;
	    tdp->timeout_type = im_parm.timeout_type;
	}
	break;

      case TMIOEVENT:
	bzero(&event_info, sizeof(struct tm_event_info));
	event_info.events = tdp->async_events;

	if (!(devflag & DKERNEL)) {	/* for a user process */
	    if (copyout(&event_info, (char *) arg,
			sizeof(struct tm_event_info)))
		errno = EFAULT;
	}
	else {	/* for a kernel process */
	    bcopy(&event_info, (char *) arg,
		  sizeof(struct tm_event_info));
	}
	tdp->async_events = 0;
	break;

      case TMIORESET:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
	if (privcheck(DEV_CONFIG) != 0) {	/* need dev_config auth. */
	    errno = EPERM;
	    break;
	}
        if (rc = fp_ioctl(tdp->fp, SCIORESET,
                 IDLUN(tdp->ddi.scsi_id, tdp->ddi.lun_id), NULL))
           errno = rc;
        break;

      case TMIOASYNC:
	if (minor(devno) & 1) {	/* must be initiator mode */
	    errno = EINVAL;
	    break;
	}
        tdp->async_flag = SC_ASYNC;
        break;

#ifdef TMIOBRKPT
	/* debug ioctl */
      case TMIOBRKPT:
	brkpoint();
	break;
#endif TMIOBRKPT

      default:
	errno = EINVAL;
    }
    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IOCTL, errno, devno);
    return (errno);
}  /* end tmscsi_ioctl */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_passthru						  */
/*                                                                        */
/* FUNCTION: This function performs pass through scsi commands		  */
/*		through TMIOCMD ioctl call				  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine.				  */
/*                                                                        */
/* DATA STRUCTURES:							  */
/* 	iocmd	- sc_iocmd structure					  */
/*	scsi	- scsi structure pointer				  */
/*	bp	- buf structure pointer					  */
/*	scb	- scb sc_buf structure pointer				  */
/*	cmd_ptr	- tm_cmd structure pointer				  */
/* INPUTS:                                                                */
/*	tdp	- tm_dev_def structure pointer				  */
/*	op	- operration						  */
/*	arg	- argument structure pointer				  */
/*	devflag	- device open mode flags				  */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
tmscsi_passthru(
		struct tm_dev_def * tdp,
		int op,
		int arg,
		int devflag)
{
    int     i, errno = 0;
    int     rc, dev, hash_key;
    struct tm_cmd *cmd_ptr;
    struct sc_buf *scb;
    struct buf *bp;
    struct scsi *scsi;
    struct sc_iocmd iocmd;

    /* Copyin arg structure */
    if (devflag & DKERNEL)
	bcopy((caddr_t) arg, (caddr_t) & iocmd,
	      sizeof(struct sc_iocmd));
    else {
	if (copyin(arg, &iocmd, sizeof(struct sc_iocmd)) != 0) {
	    return (EFAULT);
	}
    }

    /* Allocate an IOCTL cmd block for operation */
    cmd_ptr = &tdp->cmd_pb;
    scb = &cmd_ptr->scb;
    scsi = &scb->scsi_command;
    bp = &scb->bufstruct;
    bp->b_flags &= ~(B_DONE | B_ERROR);

    /* Initialize scb.bufstruct for the operation */
    bp->b_dev = tdp->ddi.adapter_devno;
    if (iocmd.data_length > 0) {	/* data transfer required */
	if (iocmd.flags & B_READ)
	    bp->b_flags |= B_READ;	/* is read allowed here */
	else
	    bp->b_flags &= ~(B_READ);
	bp->b_xmemd.aspace_id = XMEM_INVAL;

	if (devflag & DKERNEL) {
	    rc = xmattach((caddr_t) & iocmd.buffer, iocmd.data_length,
			  &bp->b_xmemd, SYS_ADSPACE);
	}
	else {
	    rc = xmattach((caddr_t) & iocmd.buffer, iocmd.data_length,
			  &bp->b_xmemd, USER_ADSPACE);
	}
	if (rc != XMEM_SUCC) {	/* xmattach failed */
	    return (EFAULT);
	}
	if (devflag & DKERNEL) {
	    rc = pinu((caddr_t) iocmd.buffer, iocmd.data_length,
		      (short) UIO_SYSSPACE);
	}
	else {
	    rc = pinu((caddr_t) iocmd.buffer, iocmd.data_length,
		      (short) UIO_USERSPACE);
	}
	if (rc != 0) {
	    (void) xmdetach(&(bp->b_xmemd));
	    return (rc);
	}
    }
    else {
	bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    }
    bp->b_bcount = iocmd.data_length;
    bp->b_work = 0x00;
    bp->b_un.b_addr = iocmd.buffer;
    bp->b_blkno = 0x00;
    bp->b_iodone = (void *) tmscsi_iodone;
    bp->b_event = EVENT_NULL;
    bp->b_error = 0;

    /* Initialize SCSI cmd for operation	 */
    scsi->scsi_length = iocmd.command_length;
    scsi->scsi_id = tdp->ddi.scsi_id;
    scsi->flags = (iocmd.flags & (SC_ASYNC | SC_NODISC));
    scsi->scsi_cmd.scsi_op_code = iocmd.scsi_cdb[0];
    scsi->scsi_cmd.lun = (tdp->ddi.lun_id << 5) | iocmd.scsi_cdb[1];
    for (i = 2; i < iocmd.command_length; i++) {
	scsi->scsi_cmd.scsi_bytes[i - 2] = iocmd.scsi_cdb[i];
    }

    /* Initialize the scb for the operation	 */
    scb->bp = NULL;
    scb->timeout_value = iocmd.timeout_value;
    scb->flags = SC_RESUME;
    scb->status_validity = 0;
    scb->scsi_status = 0;
    scb->general_card_status = 0;

    cmd_ptr->tdp = tdp;
    cmd_ptr->retry_flag = FALSE;
    tdp->cmd_state = TM_PASS_THRU;
    cmd_ptr->type = TM_OTHER_CMD;	/* Not Req_Sense cmd */

    /* Start execution of operation and sleep till done */
    (void) devstrat(bp);
    tmscsi_sleep(cmd_ptr);

    if (bp->b_bcount > 0) {	/* detach and unpin user memory */
	if (devflag & DKERNEL) {
	    rc = unpinu(bp->b_un.b_addr, bp->b_bcount, (short) UIO_SYSSPACE);
	}
	else {
	    rc = unpinu(bp->b_un.b_addr, bp->b_bcount, (short) UIO_USERSPACE);
	}
	if (rc != 0) {
	    return (rc);
	}
	if (xmdetach(&(bp->b_xmemd)) != XMEM_SUCC) {
	    return (EFAULT);
	}
    }
    /* retrieve command status and put it in user struct */
    iocmd.status_validity = scb->status_validity;
    iocmd.scsi_bus_status = scb->scsi_status;
    iocmd.adapter_status = scb->general_card_status;
    errno = bp->b_error;

    /* Return arg stucture to user	 */
    if (devflag & DKERNEL) {
	bcopy((caddr_t) & iocmd, (caddr_t) arg, sizeof(struct sc_iocmd));
    }
    else {
	if (copyout((caddr_t) & iocmd, (caddr_t) arg, sizeof(struct sc_iocmd))
	    != 0) {
	    return (EFAULT);
	}
    }
    return (errno);
}  /* end tmscsi_passthru */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_iodone							  */
/*                                                                        */
/* FUNCTION:  Target Mode Driver iodone routine(write/ioctl path only).   */
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
/* DATA STRUCTURES:							  */
/*	tdp	- tm_dev_def structure pointer				  */
/*	cmd_ptr	- tm_cmd structure pointer				  */
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
tmscsi_iodone(struct buf * buf_ptr)
{
    uchar   temp_error_save;
    struct tm_dev_def *tdp;
    uint    ret_flag, prev_tail, rc, old_level;
    struct tm_cmd *cmd_ptr;
    struct sc_error_log_df err_record;

    cmd_ptr = (struct tm_cmd *) buf_ptr;
    tdp = (struct tm_dev_def *) cmd_ptr->tdp;
    old_level = disable_lock(INTIODONE, &tdp->tm_mp_lock);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_ENTRY_IODONE, 0, tdp->devno);

    /* save io status info for TMIOSTAT ioctl */

    if ((tdp->cmd_state == TM_WRT_ACTIVE) ||
	(tdp->cmd_state == TM_RS_ACTIVE)) {
	tdp->iostatus.status_validity = cmd_ptr->scb.status_validity;
	tdp->iostatus.scsi_status = cmd_ptr->scb.scsi_status;
	tdp->iostatus.general_card_status
	    = cmd_ptr->scb.general_card_status;
	tdp->iostatus.b_error = cmd_ptr->scb.bufstruct.b_error;
	tdp->iostatus.b_resid = cmd_ptr->scb.bufstruct.b_resid;
    }

    /* Check for a good return status	 */
    if ((buf_ptr->b_error == 0) && ((buf_ptr->b_flags & B_ERROR) == 0)) {
	switch (tdp->cmd_state & 0x3f) {
	  case (TM_RS_ACTIVE):
	    tdp->iostatus.b_resid = 0;
	  case (TM_WRT_ACTIVE):
	  case (TM_PASS_THRU):
	  case (TM_INTERRUPTED):
	    buf_ptr->b_flags |= B_DONE;
	    tdp->cmd_state |= TM_CMD_DONE;
	    e_wakeup(&buf_ptr->b_event);
	    break;
	  default:
	    /* unknown iodone--software error */
	    /* log unknown SW error */
	    tmscsi_log(tdp, 7, ERRID_TMSCSI_UNKN_SFW_ERR, SC_IM_MODE,
		       (uint) cmd_ptr, (uint) tdp->cmd_state, 0);
	    cmd_ptr->scb.bufstruct.b_error = EIO;
	    cmd_ptr->scb.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scb.bufstruct.b_event);
	    break;
	}
    }
    else {	/* error completion */
	tdp->cmd_state |= TM_CMD_ERROR;
	/* save the current error status for error logging later */
	cmd_ptr->old_status_validity = cmd_ptr->scb.status_validity;
	cmd_ptr->old_scsi_status = cmd_ptr->scb.scsi_status;
	cmd_ptr->old_general_card_status = cmd_ptr->scb.general_card_status;
	cmd_ptr->old_b_error = cmd_ptr->scb.bufstruct.b_error;

	/* if halted or broken by signal */
	if ((cmd_ptr->scb.bufstruct.b_error == ENXIO) ||
	    (cmd_ptr->scb.bufstruct.b_error == EINTR)) {
	    cmd_ptr->scb.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scb.bufstruct.b_event);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IODONE, 0, tdp->devno);
	    return;
	}
	if (cmd_ptr->scb.status_validity == 0) {
	    cmd_ptr->scb.bufstruct.b_error = EIO;
	    cmd_ptr->scb.bufstruct.b_flags |= B_DONE;
	    e_wakeup(&cmd_ptr->scb.bufstruct.b_event);
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IODONE, 0, tdp->devno);
	    return;
	}

	/* scsi adapter error or scsi error occurred.	 */
	/* look for check condition and return proper err.	 */
	rc = TM_WAKEUP;
	if (cmd_ptr->scb.status_validity & SC_SCSI_ERROR) {
	    switch (cmd_ptr->scb.scsi_status & SCSI_STATUS_MASK) {
	      case (SC_BUSY_STATUS):
		/* retry in the case of writes, REQUEST SENSE */
		/* no retry on gathered writes		 */
		if ((cmd_ptr->scb.resvd1 == NULL) &&
		    (!(tdp->cmd_state & TM_PASS_THRU))) {
		    /* logged via tmscsi_retry or calling cmd */
		    rc = tmscsi_retry(cmd_ptr);
		}
		else {
		    /* gather write or not write/req sense. No retry.  */
		    cmd_ptr->scb.bufstruct.b_error = EIO;
		    if (!(tdp->cmd_state & TM_PASS_THRU)) {
			/* log temp error */
			tmscsi_log(tdp, 8, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
				   (uint) cmd_ptr, 0, 0);
		    }
		}
		break;
	      case (SC_RESERVATION_CONFLICT):
		cmd_ptr->scb.bufstruct.b_error = EBUSY;
		/* log temp scsi error */
		tmscsi_log(tdp, 9, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
			   (uint) cmd_ptr, 0, 0);
		break;
	      case (SC_CHECK_CONDITION):
		/* do not log this, as it is not recovered in this driver */
		cmd_ptr->scb.bufstruct.b_error = ENXIO;
		break;
	      case (SC_GOOD_STATUS):
	      case (SC_INTMD_GOOD):
		cmd_ptr->scb.bufstruct.b_error = EIO;
		/* log temp scsi error */
		tmscsi_log(tdp, 10, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
			   (uint) cmd_ptr, 0, 0);
		break;
	      default:
		cmd_ptr->scb.bufstruct.b_error = EIO;
		/* log temp scsi error */
		tmscsi_log(tdp, 11, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
			   (uint) cmd_ptr, 0, 0);
		break;
	    }	/* end switch on scsi_status */
	}
	else {
	    if (cmd_ptr->scb.status_validity & SC_ADAPTER_ERROR) {
		switch (cmd_ptr->scb.general_card_status) {
		  case (SC_SCSI_BUS_FAULT):
		    cmd_ptr->scb.bufstruct.b_error = ENOCONNECT;
		    /* log temp error as it is not retried in the driver */
		    tmscsi_log(tdp, 12, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
			       (uint) cmd_ptr, 0, 0);
		    break;
		  case (SC_CMD_TIMEOUT):
		    cmd_ptr->scb.bufstruct.b_error = ETIMEDOUT;
		    /* log temp error as it is not retried in the driver */
		    tmscsi_log(tdp, 13, ERRID_TMSCSI_CMD_ERR, SC_IM_MODE,
			       (uint) cmd_ptr, 0, 0);
		    break;
		  case (SC_NO_DEVICE_RESPONSE):
		    if ((cmd_ptr->scb.resvd1 == NULL) &&
			(!(cmd_ptr->tdp->cmd_state & TM_PASS_THRU))) {
			/* logged via tmscsi_retry or calling cmd */
			rc = tmscsi_retry(cmd_ptr);
		    }
		    else {
			/* gather write or pass thru.  No retry.  */
			cmd_ptr->scb.bufstruct.b_error = EIO;
			if (!(tdp->cmd_state & TM_PASS_THRU)) {
			    /* log temp error as it is not retried in the
			       driver */
			    tmscsi_log(tdp, 14, ERRID_TMSCSI_CMD_ERR,
				       SC_IM_MODE, (uint) cmd_ptr, 0, 0);
			}
		    }
		    break;
		  case (SC_HOST_IO_BUS_ERR):
		  case (SC_SCSI_BUS_RESET):
		  case (SC_ADAPTER_HDW_FAILURE):
		  case (SC_FUSE_OR_TERMINAL_PWR):
		    /* these are logged in the adapter driver */
		    cmd_ptr->scb.bufstruct.b_error = EIO;
		    break;
		  case (SC_ADAPTER_SFW_FAILURE):
		    /* this is logged in the adapter driver */
		    /* retain passed b_error value */
		    break;
		  default:
		    cmd_ptr->scb.bufstruct.b_error = EIO;
		    /* log unknown sfw error */
		    tmscsi_log(tdp, 15, ERRID_TMSCSI_UNKN_SFW_ERR, SC_IM_MODE,
			       (uint) cmd_ptr,
			       (uint) cmd_ptr->scb.general_card_status, 0);
		    break;
		}	/* end switch on general_card_status */
	    }
	}
	if (rc == TM_NOWAKEUP) {
	    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IODONE, 0, tdp->devno);
	    return;
	}
	cmd_ptr->scb.bufstruct.b_flags |= B_DONE;
	e_wakeup(&cmd_ptr->scb.bufstruct.b_event);
    }
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_IODONE, 0, tdp->devno);
    unlock_enable(old_level, &tdp->tm_mp_lock);
    return;
}  /* end tmscsi_iodone */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_sleep							  */
/*                                                                        */
/* FUNCTION:  Target Mode Driver sleep routine(write/ioctl path only).	  */
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
/* EXTERNAL PROCEDURES CALLED:  unlock_enable                             */
/*                              disable_lock                              */
/*                              e_sleep_thread                            */
/*                                                                        */
/**************************************************************************/
void
tmscsi_sleep(struct tm_cmd * cmd_ptr)
{
    uint    rc;
    struct tm_dev_def *tdp;
    int     old_level;
    char    signal_occurred = FALSE;

    tdp = cmd_ptr->tdp;
    old_level = disable_lock(INTIODONE, &tdp->tm_mp_lock);
    while ((cmd_ptr->scb.bufstruct.b_flags & B_DONE) == 0) {
	rc = e_sleep_thread(&cmd_ptr->scb.bufstruct.b_event,
                            &tdp->tm_mp_lock, LOCK_HANDLER | INTERRUPTIBLE);
	if (rc == THREAD_INTERRUPTED) {	/* if signal occurred */
	    signal_occurred = TRUE;
	    tdp->cmd_state = TM_INTERRUPTED;

            /* if the current command is still at the head waiting to be  */
            /* retried then halt the command here, else the command is at */
            /* the adapter so issue an SCIOHALT to terminate the command  */
            if (tdp->retry_pending) {
		w_stop(&tdp->tm_wdog.dog);
                tdp->retry_pending = FALSE;
                cmd_ptr->scb.bufstruct.b_flags |= B_DONE;
	        tdp->cmd_state |= TM_CMD_DONE;
            } 
            else {
		unlock_enable(old_level, &tdp->tm_mp_lock);
	        (void) fp_ioctl(tdp->fp, SCIOHALT, 
                           IDLUN(tdp->ddi.scsi_id, tdp->ddi.lun_id), NULL);
                old_level = disable_lock(INTIODONE, &tdp->tm_mp_lock);
            }
	}
    }
    unlock_enable(old_level, &tdp->tm_mp_lock);
    if (signal_occurred) {
	cmd_ptr->scb.bufstruct.b_error = EINTR;
    }
}  /* end tmscsi_sleep */


/**************************************************************************/
/* 									  */
/* name	: tmscsi_bld_cmd						  */
/**************************************************************************/
void
tmscsi_bldcmd(
	      struct tm_dev_def * tdp,
	      uchar opcode,
	      uchar resume_flag)
{
    struct tm_cmd *cmd_ptr;
    struct buf *cmd_bp;
    struct scsi *scsi;

    cmd_ptr = &tdp->cmd_pb;
    cmd_bp = &cmd_ptr->scb.bufstruct;
    scsi = &cmd_ptr->scb.scsi_command;
    /* setup common flags for io to the device */
    cmd_ptr->retry_flag = FALSE;
    cmd_ptr->retry_count = 0;
    cmd_ptr->type = TM_OTHER_CMD;	/* NOT Req_Sense cmd */

    /* setup buffer for call to SCSI device driver */
    cmd_ptr->scb.flags = resume_flag;
    scsi->scsi_length = 6;
    /* set scsi flags to async_flag value as maintained by target mode */
    /* device definition.  SC_NODISC is never set.                     */ 
    scsi->flags = tdp->async_flag;	/* SC_ASYNC | SC_NODISC; */
    scsi->scsi_cmd.scsi_op_code = opcode;
    scsi->scsi_cmd.lun = (tdp->ddi.lun_id << 5);
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    scsi->scsi_id = tdp->ddi.scsi_id;
    cmd_ptr->tdp = tdp;
    cmd_ptr->scb.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_error = 0;
    cmd_bp->b_resid = 0;
    cmd_bp->b_iodone = (void *) tmscsi_iodone;
    cmd_bp->b_dev = tdp->ddi.adapter_devno;
    cmd_bp->b_flags &= ~(B_ERROR);

}  /* end tmscsi_bldcmd */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_req_sens						  */
/*                                                                        */
/* FUNCTION: builds & issues request_sense command to the device	  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine an may not be called       */
/*      externally.                                                       */
/*                                                                        */
/* DATA STRUCTURES:							  */
/*	cmd_ptr	- tm_cmd structure pointer				  */
/*	cmd_bp	- buf structure pointer					  */
/*	scsi	- scsi structure pointer				  */
/* INPUTS:                                                                */
/*	tdp	- tm_dev_def structure pointer				  */
/*	resume_flag - flag indicating resume				  */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
int
tmscsi_req_sens(
		struct tm_dev_def * tdp,
		uchar resume_flag)
{
    struct tm_cmd *cmd_ptr;
    struct buf *cmd_bp;
    struct scsi *scsi;
    uint    old_level;

    tdp->cmd_state = TM_RS_ACTIVE;
    /* setup buffer for call to SCSI device driver */
    cmd_ptr = &tdp->rscmd_pb;
    cmd_ptr->type = TM_REQSENSE_CMD;
    cmd_ptr->retry_flag = FALSE;
    cmd_ptr->retry_count = 0;
    cmd_bp = &cmd_ptr->scb.bufstruct;
    scsi = &cmd_ptr->scb.scsi_command;
    cmd_ptr->scb.flags = resume_flag;
    cmd_ptr->scb.bp = NULL;
    cmd_bp->b_work = 0;
    cmd_bp->b_flags &= ~(B_DONE);
    cmd_bp->b_flags |= B_READ;
    cmd_bp->b_error = 0;
    cmd_bp->b_bcount = 255;
    cmd_bp->b_un.b_addr = (caddr_t) & tdp->rs_buf[0];
    cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
    cmd_bp->b_iodone = (void *) tmscsi_iodone;
    cmd_bp->b_dev = tdp->ddi.adapter_devno;
    /* setup scsi command for call to SCSI device driver */
    scsi->scsi_id = tdp->ddi.scsi_id;
    scsi->scsi_length = 6;
    /* set scsi flags to async_flag value as maintained by target mode */
    /* device definition.  SC_NODISC is never set.                     */ 
    scsi->flags = tdp->async_flag;	/* SC_ASYNC; */
    scsi->scsi_cmd.scsi_op_code = SCSI_REQUEST_SENSE;
    scsi->scsi_cmd.lun = tdp->ddi.lun_id << 5;
    scsi->scsi_cmd.scsi_bytes[0] = 0x00;
    scsi->scsi_cmd.scsi_bytes[1] = 0;
    scsi->scsi_cmd.scsi_bytes[2] = 255;	/* max. allocation length */
    scsi->scsi_cmd.scsi_bytes[3] = 0;
    cmd_ptr->tdp = tdp;
    cmd_ptr->scb.timeout_value = TM_RS_TIMEOUT;
    /* start the command		 */
    (void) devstrat(&tdp->rscmd_pb.scb.bufstruct);
    tmscsi_sleep(cmd_ptr);
    return (0);
}  /* end tmscsi_req_sens */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_log							  */
/*                                                                        */
/* FUNCTION:  Target Mode Driver error logging function			  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine may be called either on the process or interrupt     */
/*      level.                                                            */
/*                                                                        */
/* DATA STRUCTURES:							  */
/*                                                                        */
/* INPUTS:                                                                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED:                                            */
/*      bzero          bcopy                                              */
/*      errsave                                                           */
/*                                                                        */
/**************************************************************************/
void
tmscsi_log(
	   struct tm_dev_def * tdp,
	   uint errnum,
	   uint errid,
	   uint error_type,
	   uint data_ptr,
	   uint data1,
	   uint data2)
{
    struct sc_error_def *log;
    struct tm_cmd *cmd_ptr;
    struct tm_buf *bp;


    log = &tdp->error_rec;
    bzero(log, sizeof(struct sc_error_def));	/* init the logging struct */
    log->errhead.error_id = (uint) errid;
    /* copy name to error log:  s, d, l */
    bcopy(tdp->ddi.resource_name, log->errhead.resource_name, ERR_NAMESIZE);

    /* errnum is a unique code location identifier */
    log->data.reserved1 = (uchar) errnum;
    /* contents of data1/data2 varies according to what is being logged */
    log->data.reserved2 = data1;
    log->data.reserved3 = data2;

    if (error_type == SC_IM_MODE) {
	/* handle data for initiator type error logs */
	cmd_ptr = (struct tm_cmd *) data_ptr;
	/* use structure assignment to get the scsi structure */
	log->data.scsi_command = cmd_ptr->scb.scsi_command;

	if (errid == ERRID_TMSCSI_RECVRD_ERR) {
	    /* this assumes that the recovered error errid is unique */
	    log->data.status_validity = cmd_ptr->old_status_validity;
	    log->data.scsi_status = cmd_ptr->old_scsi_status;
	    log->data.general_card_status = cmd_ptr->old_general_card_status;
	}
	else {
	    /* get status for all other initiator errors */
	    log->data.status_validity = cmd_ptr->scb.status_validity;
	    log->data.scsi_status = cmd_ptr->scb.scsi_status;
	    log->data.general_card_status = cmd_ptr->scb.general_card_status;
	}
    }
    else {
	/* handle data for target type error logs */
	bp = (struct tm_buf *) data_ptr;
	/* note that there is only the SCSI ID of the remote initiator to
	   keep track of, as there is no SCSI cmd (Send implied) */
	log->data.scsi_command.scsi_id = bp->user_id;
	log->data.status_validity = bp->status_validity;
	log->data.general_card_status = bp->general_card_status;
    }

    errsave(log, sizeof(struct sc_error_def));
}  /* end tmscsi_log */



/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_getptr                                                   */
/*                                                                        */
/* FUNCTION:  Target Mode Driver get device pointer routine.              */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine an may not be called       */
/*      externally.                                                       */
/*                                                                        */
/* NOTES:  This routine finds the device ptr for the given device         */
/*         number.                                                        */
/*                                                                        */
/* DATA STRUCTURES:  none                                                 */
/*                                                                        */
/* INPUTS:                                                                */
/*    devno - the major minor number for the device.                      */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
struct tm_dev_def *
tmscsi_getptr(dev_t devno)
{
    char    tm_offset;	/* offset into hash table */
    struct tm_dev_def *tm_ptr;

    tm_offset = minor(devno) & TM_HASH_NUMBER;
    tm_ptr = tm_list[tm_offset];
    while (tm_ptr != NULL) {
	if (tm_ptr->devno == devno)
	    break;
	tm_ptr = tm_ptr->next;
    }
    return (tm_ptr);
}  /* end tmscsi_getptr */

/**************************************************************************/
/*                                                                        */
/* NAME:  tmscsi_pin							  */
/*                                                                        */
/* FUNCTION:								  */
/*	this routine attempts to pin a specified length			  */
/*	of the buffer. If pin fails, it tries to pin			  */
/*	half of the length. this goes on till length			  */
/*	becomes zero.							  */
/*                                                                        */
/* EXECUTION ENVIRONMENT:                                                 */
/*      This routine is an internal subroutine an may not be		  */
/*      called externally.						  */
/*                                                                        */
/* DATA STRUCTURES:							  */
/*                                                                        */
/* INPUTS:                                                                */
/*                                                                        */
/* RETURN VALUE DESCRIPTION:  none                                        */
/*                                                                        */
/* EXTERNAL PROCEDURES CALLED: none                                       */
/*                                                                        */
/**************************************************************************/
tmscsi_pin(
	   caddr_t buf_addr,
	   int *length,
	   short seg_flag)
{
    int     rc;

    /* this routine attempts to pin a specified length */
    /* of the buffer. If pin fails, it tries to pin    */
    /* half of the length. this goes on till length    */
    /* becomes zero.				   */

    rc = pinu(buf_addr, *length, seg_flag);
    if (rc) {
	*length = (*length >> 1);
	while (*length != 0) {
	    rc = pinu(buf_addr, *length, seg_flag);
	    if (rc == 0)
		return (rc);
	    *length = (*length >> 1);
	}
	return (rc);
    }
    return (0);
}  /* end tmscsi_pin */

/************************************************************************/
/*	NAME: tmscsi_retry						*/
/*	FUNCTION : retry send and request sense commands.		*/
/*		   No retry for gathered writes				*/
/************************************************************************/
tmscsi_retry(struct tm_cmd * cmd_ptr)
{
    int     rc;
    struct tm_dev_def *tdp;

    /* N.B.: use of TM_WAKEUP and TM_NOWAKEUP precludes use of errnos which
       match their defined values ! */
    rc = TM_WAKEUP;
    tdp = cmd_ptr->tdp;
    if ((cmd_ptr->scb.resvd1 == NULL) &&
	(!(tdp->cmd_state & TM_PASS_THRU))) {
	if (cmd_ptr->retry_count < TM_MAXRETRY) {
	    /* get here for a retry attempt */
	    cmd_ptr->retry_count++;
	    cmd_ptr->retry_flag = TRUE;
	    cmd_ptr->scb.bufstruct.b_error = 0;
	    cmd_ptr->scb.bufstruct.b_flags &= ~(B_ERROR | B_DONE);
	    if (tdp->cmd_state & TM_RS_ACTIVE) {
		tdp->cmd_state = TM_RS_ACTIVE;
		cmd_ptr->scb.timeout_value = TM_RS_TIMEOUT;
	    }
	    else {
		if (tdp->cmd_state & TM_WRT_ACTIVE) {
		    tdp->cmd_state = TM_WRT_ACTIVE;
		    if (tdp->timeout_type == TM_SCALED_TIMEOUT) {
			cmd_ptr->scb.timeout_value =
			    (((cmd_ptr->scb.bufstruct.b_bcount / 65536) + 1)
			     * tdp->timeout_value);
		    }
		    else {
			cmd_ptr->scb.timeout_value = tdp->timeout_value;
		    }
		}
		else {
		    cmd_ptr->scb.bufstruct.b_error = EIO;
		    /* log unknown SFW err */
		    tmscsi_log(tdp, 16, ERRID_TMSCSI_UNKN_SFW_ERR, SC_IM_MODE,
			       (uint) cmd_ptr, (uint) tdp->cmd_state, 0);
		    return (rc);
		}
	    }
	    if (tdp->retry_delay == 0) {
		(void) devstrat(&cmd_ptr->scb.bufstruct);
	    }
	    else {
		tdp->tm_wdog.dog.restart = tdp->retry_delay + 1;
                /* set retry pending flag */
                tdp->retry_pending = TRUE;
		w_start(&tdp->tm_wdog.dog);
	    }
	    rc = TM_NOWAKEUP;
	}
	else {
	    /* log perm unrecovered err */
	    tmscsi_log(tdp, 17, ERRID_TMSCSI_UNRECVRD_ERR, SC_IM_MODE,
		       (uint) cmd_ptr,
		       (uint) cmd_ptr->retry_count, 0);
	    cmd_ptr->retry_flag = FALSE;
	    cmd_ptr->scb.bufstruct.b_error = EIO;
	}
    }
    return (rc);
}  /* end tmscsi_retry */

/************************************************************************/
/*	NAME: tmscsi_watchdog						*/
/************************************************************************/
tmscsi_watchdog(struct watchdog * tmw)
{
    int     old_pri;
    struct tm_dev_def *tdp;
    struct tm_timer *tdog;
    struct tm_cmd *cmd_ptr;

    tdog = (struct tm_timer *) tmw;
    tdp = tdog->tdp;
    old_pri = disable_lock(INTIODONE, &tdp->tm_mp_lock);
    /* clear retry pending flag */
    tdp->retry_pending = FALSE;
    if ((tdp->cmd_state & 0x3f) == TM_RS_ACTIVE) {
	cmd_ptr = &tdp->rscmd_pb;
    }
    else {
	if ((tdp->cmd_state & 0x3f) == TM_WRT_ACTIVE) {
	    cmd_ptr = &tdp->cmd_pb;
	}
	else {
	    /* nothing to retry */
            unlock_enable(old_pri, &tdp->tm_mp_lock);
	    return;
	}
    }
    (void) devstrat(&cmd_ptr->scb.bufstruct);
    unlock_enable(old_pri, &tdp->tm_mp_lock);
}  /* end tmscsi_watchdog */

/*****************************************************************************/
/*
 * NAME:     tmscsi_select
 *
 * FUNCTION: select entry point from kernel (user processes only)
 *
 * EXECUTION ENVIRONMENT: process level only. both user/kernel processes
 *		can call this entry point.
 *
 * RETURNS:  0 or errno
 *           if successful, also returns bits for requested events that
 *           are found to be true
 */
/*****************************************************************************/
int
tmscsi_select(
	      dev_t devno,	/* major and minor number */
	      unsigned short events,	/* requested events */
	      unsigned short *reventp,	/* addr for return detected events */
	      int chan)
{
    int     rc;
    int     old_level;
    struct tm_dev_def *tdp;

    DDHKWD1(HKWD_DD_TMSCSIDD, DD_ENTRY_SELECT, 0, devno);
    if ((rc = lockl(&tmd_lock, LOCK_SHORT)) != LOCK_SUCC) {
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, EIO, devno);
	return (EIO);	/* can not lock	 */
    }
    tdp = tmscsi_getptr(devno);
    if (tdp == NULL)  {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, EINVAL, devno);
	return (EINVAL);
    }
    rc = lockl(&tdp->lock_word, LOCK_SHORT);
    if (rc != LOCK_SUCC) {
	unlockl(&tmd_lock);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, EIO, devno);
	return (EIO);
    }
    unlockl(&tmd_lock);
    if (!tdp->opened) {
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, EINVAL, devno);
	return (EINVAL);
    }

    /* POLLOUT not supported */
    if (events & POLLOUT) {
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, EINVAL, devno);
	return (EINVAL);
    }

    *reventp = 0;	/* initialize return value */
    if ((events & ~POLLSYNC) == 0) {	/* no events requested */
	unlockl(&tdp->lock_word);
	DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, 0, devno);
	return (0);
    }

    /* set return status for all requested events that are true */

    old_level = disable_lock(tdp->ddi.int_prior, &tdp->tm_mp_lock);
    if (events & POLLIN) {
	if ((tdp->head_rdbufs) &&
	    (tdp->head_rdbufs->user_flag & TM_HASDATA)) {
	    events &= ~POLLIN;
	    *reventp |= POLLIN;
	}
    }
    if (events & POLLPRI) {
	if (tdp->async_events != 0) {
	    events &= ~POLLPRI;
	    *reventp |= POLLPRI;
	}
    }

    if (!(events & POLLSYNC)) {
	/* not synchronous call, so save the passed events (those not already
	   satisfied) so we'll know to do asynchronous notification */
	tdp->event_flag |= events;
    }
    unlock_enable(old_level, &tdp->tm_mp_lock);

    unlockl(&tdp->lock_word);
    DDHKWD1(HKWD_DD_TMSCSIDD, DD_EXIT_SELECT, 0, devno);
    return (0);
}  /* end tmscsi_select */

/*****************************************************************************/
/*
 * NAME:     tmscsi_cdt_func
 *
 * FUNCTION: This routine is called at system dump time.  The component 
 *	     dump table has been allocated at config time, so this routine
 *	     fills in the table and returns the address to the caller.
 *
 * EXECUTION ENVIRONMENT:
 *		This routine is called on an interrupt level.
 *		It cannot page fault.
 *
 * INPUTS:
 *		arg - 1 indicates the start of a dump
 *		      2 indicates the end of a dump
 *
 * RETURNS:
 *		This routine returns the address of the component
 *		dump table.
 */
/*****************************************************************************/
struct cdt *
tmscsi_cdt_func(int arg)
{
    struct tm_dev_def *tdp;
    struct cdt_entry *entry_ptr;
    int     entry_count, i;

    if (arg == 1) {
	/* only build the dump table on the initial dump call */

	/* First dump entry is for tm_list */
	entry_ptr = &tm_cdt->cdt_entry[0];
	strcpy(entry_ptr->d_name, "tm_list");
	entry_ptr->d_len = sizeof(caddr_t) * TM_HASH_SIZE;
	entry_ptr->d_ptr = (caddr_t) tm_list;
	entry_ptr->d_segval = (int) NULL;

	/* Rest of entries are target mode instance structures */
	entry_count = 1;
	for (i = 0; i < TM_HASH_SIZE; i++) {
	    tdp = tm_list[i];
	    while (tdp != NULL) {
		entry_ptr = &tm_cdt->cdt_entry[entry_count++];
		/* copy name to element:  s, d, l */
		bcopy(tdp->ddi.resource_name,
		      entry_ptr->d_name, 8);
		entry_ptr->d_len = sizeof(struct tm_dev_def);
		entry_ptr->d_ptr = (caddr_t) tdp;
		entry_ptr->d_segval = (int) NULL;
		tdp = tdp->next;
	    }	/* end while */
	}	/* end for */
    }

    return (tm_cdt);
}  /* end tmscsi_cdt_func */
