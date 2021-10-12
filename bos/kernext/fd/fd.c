#ifndef lint
static char sccsid[] = "@(#)96 1.80.2.11 src/bos/kernext/fd/fd.c, sysxfd, bos41J, 9516A_all 4/14/95 12:49:50";
#endif
/*
 * COMPONENT_NAME: (SYSXFD) Diskette Device Driver
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */

/*
 * FUNCTION: This device driver provides device support for the
 *           diskette controller and its attached drives.  Support
 *           is provided for one adapter and up to two drives.  This
 *           module is the top half of the device driver and may
 *           be paged out of memory.
 *
 * ROUTINES: fd_config, fd_open, fd_close, fd_read, fd_write, fd_ioctl
 *
 * INTERNAL SUBROUTINES: fd_qvpd_exit, fdconfig_motor_start,
 *           fdconfig_motor_stop, fdconfig_recalibrate, fdconfig_reset,
 *           fdconfig_seek, fdconfig_sense_interrupt, fdconfig_soft_reset,
 *           fdconfig_specify, fdconfig_vpd, fddoor_check, fdiocdseldrv,
 *           fdiocformat, fdiocreadid, fdiocrecal, fdiocreset, fdiocseek,
 *           fdiocseldrv, fdiocsettle, fdiocspeed, fdiocstatus, fdmotor_start,
 *           fdopen_exit, fdreset_check, fdtype, fdconfig_read_threshold
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/adspace.h>
#include <sys/buf.h>
#include <sys/ddtrace.h>
#include <sys/device.h>
#include <sys/devinfo.h>
#include <sys/dma.h>
#include <sys/errids.h>
#include <sys/errno.h>
#include <sys/except.h>
#include <sys/fd.h>
#include <sys/intr.h>
#include <sys/ioacc.h>
#include <sys/ioctl.h>
#include <sys/lockl.h>
#include <sys/malloc.h>
#include <sys/param.h>
#include <sys/pin.h>
#include <sys/pri.h>
#include <sys/priv.h>
#include <sys/proc.h>
#include <sys/sleep.h>
#include <sys/sysmacros.h>
#include <sys/syspest.h>
#include <sys/time.h>
#include <sys/timer.h>
#include <sys/trchkid.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/watchdog.h>

/* NVRAM-WA */
#ifdef _POWER_MP
#undef _POWER_MP
#endif /* _POWER_MP */

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif /* _POWER_MP */

#include "fd_local.h"

/*
 * allocate and initialize the lock word.
 */

lock_t fd_lock = LOCK_AVAIL;

extern	struct adapter_structure *fdadapter;

extern	struct fdconfig_parameters fdconfparms[FDMAXDRIVES];

/*int	nodev();*/


/*
 * NAME: fd_config
 *
 * FUNCTION: fd_config() initializes the diskette device driver.
 *  It will allocate and initialize all data structures for
 *  the diskette adapter and any diskette drives.
 *     - CFG_TERM
 *	 - Adapter
 *	   - xmfree trace table
 *	   - reset POS registers
 *	   - remove from device switch table
 *	   - free adapter structure
 *	 - Drive
 *	   - free floppy structure
 *
 *     - CFG_INIT
 *	 - Adapter
 *	   - allocate adapter structure
 *	   - add to device switch table
 *	   - initialize POS registers
 * 	   - allocate trace table
 *	 - Drive
 *	   - allocate floppy structure
 *
 *     - CFG_QVPD
 *	 - check what drives are present on the system
 *
 * EXECUTION ENVIRONMENT: process level
 *
 * NOTES:
 *
 * INPUTS:
 *  devno - the major and minor device numbers.
 *  cmd   - CFG_INIT if device is being defined.
 *        - CFG_TERM if device is being deleted.
 *        - CFG_QVPD if diskette device presence check is being run.
 *  uiop  - pointer to uio structure containing fd_config data.
 *
 * OUTPUTS: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  uiomove, lockl, unlockl, lock_alloc, lock_free, simple_lock_init
 *
 * ERROR CODES: The following errno values may be returned:
 *  EBUSY   - drive is not in the closed state.
 *  ENXIO   - invalid minor number.
 *  ENINVAL - invalid cmd argument.
 */

int	fd_config (dev_t devno, int cmd, register struct uio *uiop)
{
	union	fd_config configstruct;
	struct	uio	localuio;
	struct	iovec	*localuio_iovec;
	register union  fd_config *configptr;
	register struct floppy *fdp;
	struct	devsw fddevsw;
	int	drive_number, minor_number;
	int	i, rc = FDSUCCESS, trc;
	caddr_t	iocc_val;
	uchar   temp_val;


	DDHKWD5(HKWD_DD_FDDD, DD_ENTRY_CONFIG, 0, devno, cmd, uiop, 0, 0);
	lockl(&fd_lock, LOCK_SHORT);

	/*
	 * Use minor device number to determine which drive to use.
	 */

	minor_number = minor(devno);
	drive_number = minor_number >> 6;
	DEBUG_1 ("fd_config: minor no = %x\n", minor_number);
	if ( (drive_number < 0 || drive_number >= FDMAXDRIVES) && 
	      !(minor_number & FD_ADAP_MINOR_NUM) ) {
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENXIO, devno);
		unlockl(&fd_lock);
		return(ENXIO);
	}
	/*
	 * The drive number is valid.
	 */

	switch (cmd) {
	case CFG_TERM:
		DEBUG_0 ("fd_config: CFG_TERM\n");

		/* check if adapter or drive should be free'd */
		if (minor_number & FD_ADAP_MINOR_NUM) {
		 	/* If the adapter structure is not initialized  */
		 	/* then there is nothing to delete.		*/
			if (fdadapter == NULL) {
	                        DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, 
					ENXIO, devno);
				unlockl(&fd_lock);
				return(ENXIO);
                	}

			/* check if any other drives are still AVAILABLE */
			/* if so, don't delete the adapter		 */
			for (i = 0; i < FDMAXDRIVES; i++) {
				if (fdadapter->drive_list[i] != NULL) {
		                        DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, 
						ENXIO, devno);
					unlockl(&fd_lock);
					return(ENXIO);
				}
			}
#ifdef DEBUG
			/* delete the trace table */
#ifdef _POWER_MP
			lock_free(&fdadapter->trace_table->trace_lock);
#endif /* _POWER_MP */
			(void)xmfree((caddr_t)fdadapter->trace_table,
			    pinned_heap);
#endif
			/* Reset the POS registers		*/
			/* If this is a type 1 adapter and then */
			/* disable the dma channel.		*/
			DEBUG_0 ("fd_config: reset POS regs\n");
			if (fdadapter->adapter_type == FD_ADAPTER1) {
				iocc_val = IOCC_ATT(fdadapter->bus_id, 0);
				temp_val = BUSIO_GETC((ulong)iocc_val +
				    FDARBREG + (fdadapter->slot_num << 16));
				/* turn off dma enable and fairness */
				temp_val &= 0xaf; 
				BUSIO_PUTC((ulong)iocc_val + FDARBREG +
				   (fdadapter->slot_num << 16), temp_val);
				IOCC_DET(iocc_val);
			}
			
			/* delete the driver from the switch table */
			DEBUG_0 ("fd_config: devswdel\n");
			(void)devswdel(devno);

			/* free the adapter structure */
			DEBUG_0 ("fd_config: free adap. struct\n");
			(void)free((caddr_t)fdadapter);
			fdadapter = NULL;
		}
		else {
			/* delete a floppy structure */
			fdp = fdadapter->drive_list[drive_number];
			if (fdp == NULL) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, 
					ENXIO, devno);
				unlockl(&fd_lock);
				return(ENXIO);
			}

			/*     
			 * If drive is not closed, return with error.
			 */

			DEBUG_0 ("fd_config: drive check CLOSED\n");
			if (fdp->drive_state != FDCLOSED) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, 
					EBUSY, devno);
				unlockl(&fd_lock);
				return(EBUSY);
			}

			/*     
			 * Otherwise delete the drive.
			 */
#ifdef _POWER_MP
			lock_free(&fdp->intr_lock);
#endif /* _POWER_MP */

			DEBUG_0 ("fd_config: free floppy structure\n");
			(void)free((caddr_t)fdp);
			fdadapter->drive_list[drive_number] = NULL;
		} 
		break; /* end CFG_TERM case */

	case CFG_INIT:
		DEBUG_0 ("fd_config: CFG_INIT\n");

		/* check if adapter or drive configuration */
		if (minor_number & FD_ADAP_MINOR_NUM)
		{  /* configure the diskette adapter */
			DEBUG_0 ("fd_config: config adapter\n");

			/* check if adapter structure already exists */
			if (fdadapter != NULL) {
			        DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
			                EBUSY, devno);
			        unlockl(&fd_lock);
			        return(EBUSY);
			}

			/*
		 	 * Allocate the adapter structure and initialize
			 * global parameters.
		 	 */
			fdadapter = (struct adapter_structure *)
			    malloc(sizeof(struct adapter_structure ));
			if (fdadapter == NULL) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENOMEM, 
				    devno);
				unlockl(&fd_lock);
				return(ENOMEM);
			}
			bzero(fdadapter,sizeof(struct adapter_structure));
			fdadapter->adapter_busy = FALSE;
			fdadapter->error_value = 0;
			fdadapter->data_reg_excess = 0;
			fdadapter->sub_state = NULL;
			fdadapter->initialized = FALSE;
			fdadapter->int_init = FALSE;
			fdadapter->dma_init = FALSE;
			fdadapter->first_open = TRUE;
			fdadapter->reset_active = FALSE;
			fdadapter->active_drive = 0;
			fdadapter->motor_on = FDNO_DRIVE;
			fdadapter->inttimer.func = NULL;
			fdadapter->mottimer.func = NULL;
			fdadapter->fdstart_timer = NULL;
			fdadapter->fderrptr = NULL;
			fdadapter->data_rate = 0xff;
			fdadapter->reset_needed = FALSE;
			fdadapter->reset_performed = FALSE;
			fdadapter->fudge_factor = 3000000;    /* 3 msecs */
			fdadapter->actual_delay = 0xffffffff;
			fdadapter->sleep_anchor = EVENT_NULL;
			fdadapter->adapter_sleep_anchor = EVENT_NULL;
#ifdef DEBUG
			fdadapter->trace_table = NULL;
#endif
			for (i = 0; i < FDMAXDRIVES; i++)
				fdadapter->drive_list[i] = NULL;

			DEBUG_1 ("fd_config: adapter addr = %x\n", fdadapter);

			/* copy in dds */
			rc = uiomove((caddr_t)(&configstruct.fda), 
				sizeof(configstruct.fda), UIO_WRITE, uiop);
			if (rc != 0) {
				rc = free((caddr_t)fdadapter);
				ASSERT(rc == 0);
				fdadapter = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, rc,
				    devno);
				unlockl(&fd_lock);
				return(rc);
			}
			configptr = &configstruct;
			DEBUG_1 ("fd_config: config addr = %x\n", configptr);

			/* initialize adapter values that depend */
			/* on database info 			 */
			fdadapter->adapter_type  = configptr->fda.adapter_type;
			fdadapter->dma_level     = configptr->fda.dma_level;
			fdadapter->bus_int_level = configptr->fda.bus_int_level;
			fdadapter->int_class     = configptr->fda.int_class;
			fdadapter->slot_num      = configptr->fda.slot_num;
			fdadapter->io_address    = configptr->fda.io_address;
			fdadapter->bus_type      = configptr->fda.bus_type;
			fdadapter->bus_id        = ((configptr->fda.bus_id 
						    & 0x0ff00000) | 0x800c0000);
			DEBUG_0 ("fd_config: adapter values initialized\n");

			/* add the diskette to the device switch table */
			fddevsw.d_open = fd_open;
			fddevsw.d_close = fd_close;
			fddevsw.d_read = fd_read;
			fddevsw.d_write = fd_write;
			fddevsw.d_ioctl = fd_ioctl;
			fddevsw.d_strategy = fd_strategy;
			fddevsw.d_ttys = 0;
			fddevsw.d_select = nodev;
			fddevsw.d_config = fd_config;
			fddevsw.d_print = nodev;
			fddevsw.d_dump = nodev;
			fddevsw.d_mpx = nodev;
			fddevsw.d_revoke = nodev;
			fddevsw.d_dsdptr = NULL;
			fddevsw.d_selptr = NULL;
#ifdef _POWER_MP
			fddevsw.d_opts = DEV_MPSAFE;
#else
			fddevsw.d_opts = 0;
#endif /* _POWER_MP */
			rc = devswadd(devno, &fddevsw);
			if (rc != 0) {
				rc = free((caddr_t)fdadapter);
				ASSERT(rc == 0);
				fdadapter = NULL;
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, rc,
				    devno);
				unlockl(&fd_lock);
				return(rc);
			}
			DEBUG_0 ("fd_config:  device added to switch table\n");


#ifdef _POWER

		/*
		 * initialize the adapter hardware if not already done.
		 */


		/* initialize the SIO's POS registers */
		/* that effect the diskette 	      */
		DEBUG_0 ("fd_config: init POS reg. for fda\n");
		iocc_val = IOCC_ATT(fdadapter->bus_id, 0);

		/* enable standard I/O bus drivers */
		temp_val = BUSIO_GETC((ulong)iocc_val +
		    FDENABLEREG + (fdadapter->slot_num << 16));
		BUSIO_PUTC((ulong)iocc_val + FDENABLEREG +
		   (fdadapter->slot_num << 16), temp_val | 1);

		/* Set interrupt level and DMA arbitration level */
		/* For FD_ADAPTER1 enable DMA and disable diskette fairness */
		temp_val = BUSIO_GETC((ulong)iocc_val +
		    FDARBREG + (fdadapter->slot_num << 16)) & 0x70;
		if (fdadapter->adapter_type != FD_ADAPTER2)
			temp_val |= (fdadapter->bus_int_level & 1) << 7;
		else
			temp_val |= (!(fdadapter->bus_int_level & 1)) << 7;
		temp_val |= fdadapter->dma_level;

		/* turn on dma enable and disable diskette fairness */
		if (fdadapter->adapter_type == FD_ADAPTER1) {
			temp_val |= 0x40; 	/* turn on dma enable */ 
			temp_val &= 0xef;	/* disable diskette fairness */
		}
		BUSIO_PUTC((ulong)iocc_val + FDARBREG +
		   (fdadapter->slot_num << 16), temp_val);
		IOCC_DET(iocc_val);
#endif

#ifdef DEBUG
		fdadapter->trace_table = (struct fdtrace_struct *)
		    xmalloc(sizeof(struct fdtrace_struct ), 4, 
		    pinned_heap);
		if (fdadapter->trace_table == NULL) {
			rc = free((caddr_t)fdadapter);
			ASSERT(rc == 0);
			rc = devswdel(devno);
			ASSERT(rc == 0);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENOMEM,
			    devno);
			unlockl(&fd_lock);
			return(ENOMEM);
	   	}
		strcpy(fdadapter->trace_table->start, "FDSTART");
		strcpy(fdadapter->trace_table->end, "FDEND");
		fdadapter->trace_table->current = 0;
#ifdef _POWER_MP
		lock_alloc(&fdadapter->trace_table->trace_lock,
			LOCK_ALLOC_PIN, FDDD_LOCK_CLASS, 0);
		simple_lock_init(&fdadapter->trace_table->trace_lock);

#endif /* _POWER_MP */
                        
#endif
		} /* if */
		else
		{ /* configure a drive instead of adapter */
			DEBUG_0 ("fd_config: config drive\n");

			/* check if there is an adapter structure */
			if (fdadapter == NULL) {
			        DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENXIO,
			                devno);
			        unlockl(&fd_lock);
			        return(ENXIO);
			}

			/* copy in the dds about the drive */
			rc = uiomove((caddr_t)(&configstruct.fdd), 
				sizeof(configstruct.fdd), UIO_WRITE, uiop);
			if (rc != 0) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, rc,
				    devno);
				unlockl(&fd_lock);
				return(rc);
			}
			configptr = &configstruct;

			/* check if floppy structure already exists */
			if (fdadapter->drive_list[drive_number] != NULL) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, rc,devno);
				unlockl(&fd_lock);
				return(EBUSY);
			} else
		 	 { /* if not, create floppy structure for drive */
				fdadapter->drive_list[drive_number] = 
					(struct floppy *) malloc 
						(sizeof (struct floppy )); 
				if (fdadapter->drive_list[drive_number] == NULL)
				{
					DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, 
						ENOMEM, devno);
					unlockl(&fd_lock);
					return(ENOMEM);
				}
				fdp = fdadapter->drive_list[drive_number];
				fdp->device_number = devno;
				fdp->drive_state = FDCLOSED;
				fdp->initialized = FALSE;
				fdp->headptr = NULL;
				fdp->tailptr = NULL;
				fdp->retry_flag = TRUE;
				fdp->step_size = 1;
				fdp->rcount_bytes = 0;
				fdp->rcount_megabytes = 0;
				fdp->wcount_bytes = 0;
				fdp->wcount_megabytes = 0;
				fdp->last_error1 = 0;
				fdp->last_error2 = 0;
#ifdef 	_POWER_MP
				lock_alloc(&fdp->intr_lock, LOCK_ALLOC_PIN, 
					   FDDD_LOCK_CLASS, drive_number+1);
				simple_lock_init(&fdp->intr_lock);
#endif 	/* _POWER_MP */
			} 

			DEBUG_1 ("fd_config: floppy struct addr = %x\n",
				fdadapter->drive_list[drive_number]);
			DEBUG_1 ("fd_config: config addr = %x\n", configptr);

			/* check for improper drive type config */
			if ((configptr->fdd.type == D_1354H) &&
			   (fdadapter->adapter_type == FD_ADAPTER0)) {
#ifdef _POWER_MP
				lock_free(&fdp->intr_lock);
#endif /* _POWER_MP */
				rc = free((caddr_t)fdp);
				ASSERT(rc == 0);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, EINVAL,
					devno);
				unlockl(&fd_lock);
				return(EINVAL);
			}

			/* copy drive specific information into */
			/* the floppy structure 		*/
			fdp->drive_type = configptr->fdd.type;
			fdconfparms[drive_number].type = fdp->drive_type;
			fdconfparms[drive_number].head_settle = 
						configptr->fdd.head_settle;
			fdp->head_settle_time = 
				fdconfparms[drive_number].head_settle;
			fdconfparms[drive_number].step_rate = 
						configptr->fdd.step_rate;
			fdp->step_rate_time = 
				fdconfparms[drive_number].step_rate;
			fdconfparms[drive_number].motor_start = 
						configptr->fdd.motor_start;
			fdp->motor_start = 
				fdconfparms[drive_number].motor_start;
			for (i = 0; i < 8; i++) 
				fdp->resource_name[i] = 
					configptr->fdd.resource_name[i];
			fdp->initialized = TRUE;
		} /* else */
		break; /* end CFG_INIT case */

	case CFG_QVPD:
		/* CFG_QVPD checks for the presence of diskette drives     */
		/* It performs the following sequence of events:           */
		/*    - pin code and adapter structure                     */
		/*    - xmalloc "fd_err_rec"			           */
		/*    - for each possible drive on the system              */
		/*       - malloc and init floppy structure 	           */
		/*       - touch drive (through "fdconfig_vpd")		   */
		/*       - store drive information in "drive_info"         */
         	/*    - unpin code and adapter structure 		   */
		/*    - free "fd_err_rec" and any floppy structure alloc'd */
		/*    - copy info about the drives back to config method   */

		DEBUG_0 ("fd_config: CFG_QVPD\n");

		/* check if there is an adapter structure */
		if (fdadapter == NULL) {
		        DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENXIO,
		                devno);
		        unlockl(&fd_lock);
		        return(ENXIO);
		}

		/*
		 * must make local copy of uio structure before doing uiomove
		 * since the same uio structure is used to get data from the
		 * user and to send data to the user.
		 */

		localuio_iovec = (struct iovec *)xmalloc
		    ((uint)uiop->uio_iovcnt * sizeof(struct iovec),
		    (uint)3, pinned_heap);
		if (localuio_iovec == NULL) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, ENOMEM, 
			    devno);
			unlockl(&fd_lock);
			return(ENOMEM);
		}
		bcopy(uiop, &localuio, sizeof(struct uio));
		bcopy(uiop->uio_iov, localuio_iovec, uiop->uio_iovcnt * 
		    sizeof(struct iovec));
		rc = uiomove((caddr_t)(&configstruct), 
			     sizeof(configstruct.drive_info), UIO_WRITE, uiop);
		if (rc != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, rc, 
			    devno);
			trc = xmfree((caddr_t)localuio_iovec,
			    pinned_heap);
			ASSERT(trc == 0);
			unlockl(&fd_lock);
			return(rc);
		}
		configptr = &configstruct;

		/* pin the fd_intr and the data structures */
		/* so that we can 'touch' the drives       */
		if (fdadapter->initialized == FALSE) {
			rc = pincode(fd_intr);
			if (rc != 0) {
				trc = xmfree((caddr_t)localuio_iovec,
				    pinned_heap);
				ASSERT(trc == 0);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
				    rc, devno);
				unlockl(&fd_lock);
				return(rc);
			}
			DEBUG_0 ("fd_config: intr pinned\n");

			rc = pin((caddr_t)fdadapter,
			    sizeof(struct adapter_structure ));
			if (rc != 0) {
				rc = unpincode(fd_intr);
				ASSERT(rc == 0);
				trc = xmfree((caddr_t)localuio_iovec,
				    pinned_heap);
				ASSERT(trc == 0);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
				    rc, devno);
				unlockl(&fd_lock);
				return(rc);
			}
			DEBUG_0 ("fd_config: adapter pinned\n");

			fdadapter->fderrptr = (struct fd_err_rec *)
	    			xmalloc(sizeof(struct fd_err_rec ), 3,
				    pinned_heap);
			if (fdadapter->fderrptr == NULL) {
				rc = unpin((caddr_t)fdadapter,
				     sizeof(struct adapter_structure ));
				ASSERT(rc == 0);
				rc = unpincode(fd_intr);
				ASSERT(rc == 0);
				trc = xmfree((caddr_t)localuio_iovec,
				    pinned_heap);
				ASSERT(trc == 0);
				unlockl(&fd_lock);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN,
				    ENOMEM, devno);
				return(ENOMEM);
			}
		}

		/* check which drives are present on the system */
		/* the status of each drive will be marked in "drive_info" */
		/* and this information will be passed back to "cfgfda"    */
		for (i = 0; i < FDMAXDRIVES; i++) {
			DEBUG_1 ("fd_config: touch drive %d\n", i);
			configptr->drive_info[i] = 0;
		/*
	 	 * If the floppy structure has not already been allocated, then
	 	 * allocate it and initialize needed parameters.
	 	 */

			/* allocate a temporary floppy structure to touch */
			/* the drive.  The permanent structure will be    */
			/* created when "cfgfdd" calls CFG_INIT.          */
			/* If the structure already exists, mark the      */
			/* drive as already configured			  */
			if (fdadapter->drive_list[i] == NULL) {
				fdadapter->drive_list[i] =
				    (struct floppy *)xmalloc
				    (sizeof (struct floppy ), 3,
				    pinned_heap);
				if (fdadapter->drive_list[i] == NULL) {
					fd_qvpd_exit(configptr, localuio_iovec);
					DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
					    ENOMEM, devno);
					unlockl(&fd_lock);
					return(ENOMEM);
				}
				fdp = fdadapter->drive_list[i];
#ifdef _POWER_MP
				lock_alloc(&fdp->intr_lock,
					LOCK_ALLOC_PIN, FDDD_LOCK_CLASS, i+1);
				simple_lock_init(&fdp->intr_lock);
#endif /* _POWER_MP */
				fdp->head_load = 0xf; /* max value */
				fdp->head_unload = 0x7f; /* max value */
				fdp->step_rate = 0; /* max value */
				fdp->headptr = NULL;
				fdp->device_number = makedev(major(devno),
				    i << 6); 
			} else
			 {
				fdp = fdadapter->drive_list[i];
				configptr->drive_info[i] |= FD_CONFIGURED;
				configptr->drive_info[i] |= fdp->drive_type;
			}

			rc = fdlock_adapter(i);
			if (rc != FDSUCCESS) {
				fd_qvpd_exit(configptr, localuio_iovec);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
				    rc, devno);
				unlockl(&fd_lock);
				return(rc);
			}

			/* see if the drive exists */
			DEBUG_0 ("fd_config:  call fdconfig_vpd\n");
			rc = fdconfig_vpd(fdp, configptr, (uchar)i);
			if (rc != FDSUCCESS) {
				fdtop_unlock_adapter(i);
				fd_qvpd_exit(configptr, localuio_iovec);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG,
				    rc, devno);
				unlockl(&fd_lock);
				return(rc);
			}

			DEBUG_2 ("fd_config:  diskette %d drive_info = %x\n",
				i, configptr->drive_info[i]);
			fdtop_unlock_adapter(i);
		}

		/* copy original information back to the config routine */
		/* but substitute in the new "fda_config"               */
		/* only care about "drive_info"				*/
		bcopy(&localuio, uiop, sizeof(struct uio));
		bcopy(localuio_iovec, uiop->uio_iov, uiop->uio_iovcnt * 
		    sizeof(struct iovec));
		rc = uiomove((caddr_t)(configptr), 
			sizeof(configstruct.drive_info), UIO_READ, &localuio);

		fd_qvpd_exit(configptr, localuio_iovec);
		break;

	default:
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, EINVAL, devno);
		return(EINVAL);
		break;
	} /* end switch on cmd */
	unlockl(&fd_lock);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CONFIG, FDSUCCESS, devno);
	return(FDSUCCESS);
} /* end fd_config routine */


/*
 * NAME: fd_open
 *
 * FUNCTION: The open entry point.
 *  This routine prepares a diskette device for use.  This includes
 *  making sure the interrupt and dma services are properly set up
 *  and that the proper device characteristics are loaded for the
 *  drive type and diskette type being used.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It 
 *  can be called only by a process and can page fault. 
 *
 * DATA STRUCTURES: 
 *  fdadapter struct - Per adapter information.
 *  floppy    struct - Per device information.
 *  intr      struct - Interrupt handler structure.
 *
 * INPUTS:
 *  devno  - the major & minor device numbers.
 *  devflag - the operation to perform and flags ORed together. 
 *
 * RETURN VALUE DESCRIPTION: FDSUCCESS (0) or error code. 
 *
 * ERROR CODES: The following errno values may be returned: 
 *  ENOTREADY  - no diskette in drive or drive door open.
 *  EBUSY      - drive already opened by another process.
 *  EINVAL     - invalid minor number. (drive or diskette type).
 *  ENXIO      - drive has been deleted.
 *  EIO        - unable to initialize dma or interrupt services.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  i_init, pincode, w_init, talloc, d_init, delay, lockl, unlockl
 *
 * NOTES:
 *  The 'devno' parameter always indicates which drive to
 *  open, and may indicate the diskette characteristics to
 *  use.  The 'minor' macro is used to extract the minor
 *  number from 'devno'.  If the minor number portion of
 *  'devno' is a 0 or a 1, then the fd_open() routine will
 *  attempt to determine the drive type and diskette type by
 *  reading from the drive with different drive
 *  characteristics.  This is done by calling the fdtype()
 *  routine.  If the minor number is 4 or greater, then the
 *  diskette characteristics implied by that format specific
 *  special file are used.  The determination of the drive
 *  type can be suppressed by ORing the O_NDELAY flag into
 *  the 'devflag' parameter.  This is mainly used by the
 *  format command since the open will otherwise fail for an
 *  unformatted diskette. 
 */

int	fd_open (dev_t devno, ulong devflag)
{
	register struct floppy *fdp;
	int	i, drive_number, rc;

	FD_TRACE1("in open", devno);
	DDHKWD5(HKWD_DD_FDDD, DD_ENTRY_OPEN, 0, devno, devflag, 0, 0, 0);
	lockl(&fd_lock, LOCK_SHORT);

	/*
	 * Use minor device number to determine which drive to use.
	 */

	drive_number = minor(devno) >> 6;
	if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, EINVAL, devno);
		FD_TRACE1("bad drive #", devno);
		FD_TRACE1("out open", devno);
		return(EINVAL);
	}

	/*
	 * Set up a pointer to the proper diskette data structure.
	 */

	fdp = fdadapter->drive_list[drive_number];

	/*
	 * See if this drive has been deleted.  If so, return.
	 */

	if (fdp == NULL) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, ENXIO, devno);
		FD_TRACE1("deleted drv", devno);
		FD_TRACE1("out open", devno);
		return(ENXIO);
	}

	/*
	 * If drive is already open or is being opened, set rc to EBUSY
	 * and return.
	 */

	if (fdp->drive_state != FDCLOSED) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, EBUSY, devno);
		FD_TRACE1("busy drive", devno);
		FD_TRACE1("out open", devno);
		return(EBUSY);
	}
	FD_TRACE1("new state", fdp->drive_state);
	fdp->drive_state = FDOPENING;

	/* 
	 * If the device driver has not been initialized, then do so.
	 */

	if (fdadapter->initialized == FALSE) {
		rc = pincode(fd_intr);
		if (rc != 0) {
			FD_TRACE1("new state", fdp->drive_state);
			fdp->drive_state = FDCLOSED;
			unlockl(&fd_lock);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, rc, devno);
			FD_TRACE1("pin failed", devno);
			FD_TRACE1("out open", devno);
			return(rc);
		}
		fdadapter->initialized = TRUE;

		/* 
		 * If the adapter structure has not been pinned, then do so.
		 */

		if (fdadapter->pinned == FALSE) {
			rc = pin((caddr_t)fdadapter,
			    sizeof(struct adapter_structure ));
			if (rc != 0) {
				FD_TRACE1("new state", fdp->drive_state);
				fdp->drive_state = FDCLOSED;
				fdcleanup(drive_number);
				unlockl(&fd_lock);
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, rc, devno);
				FD_TRACE1("out open", devno);
				return(rc);
			}
			fdadapter->pinned = TRUE;
		}

		/*  
		 * Initialize the timers.
		 */

		/* 5 second timeout condition */
		fdadapter->inttimer.restart = 5;
		fdadapter->inttimer.func = fdwatchdog;
		fdadapter->inttimer.next = NULL;
		fdadapter->inttimer.prev = NULL;
		fdadapter->inttimer.count = 0;

		/* 10 second timeout condition */
		fdp->motor_off_time = 10;
		fdadapter->mottimer.restart = fdp->motor_off_time;
		fdadapter->mottimer.func = fdmotor_timer;
		fdadapter->mottimer.next = NULL;
		fdadapter->mottimer.prev = NULL;
		fdadapter->mottimer.count = 0;

		fdadapter->fdstart_timer = talloc();
		fdadapter->fdstart_timer->ipri = fdadapter->int_class;
		fdadapter->fdstart_timer->func = fdtimeout;

#ifdef _POWER_MP
		while(w_init(&fdadapter->inttimer));
		while(w_init(&fdadapter->mottimer));
		fdadapter->fdstart_timer->flags = T_MPSAFE;
#else
		w_init(&fdadapter->inttimer);
		w_init(&fdadapter->mottimer);
		fdadapter->fdstart_timer->flags = 0;
#endif /* _POWER_MP */

		/*
		 * Allocate the error log structure.
		 */

		fdadapter->fderrptr = (struct fd_err_rec *)
		    xmalloc(sizeof(struct fd_err_rec ), 3, pinned_heap);
		if (fdadapter->fderrptr == NULL) {
			FD_TRACE1("new state", fdp->drive_state);
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, EIO, devno);
			FD_TRACE1("out open", devno);
			return(EIO);
		}

		/*
		 * Initialize the interrupt handler.
		 */

		fdadapter->fdhandler = (struct intr *)
		    xmalloc(sizeof(struct intr ), 3, pinned_heap);
		if (fdadapter->fdhandler == NULL) {
			FD_TRACE1("new state", fdp->drive_state);
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, ENOMEM, devno);
			FD_TRACE1("out open", devno);
			return(ENOMEM);
		}
		fdadapter->fdhandler->handler = fd_intr;
		fdadapter->fdhandler->bus_type = fdadapter->bus_type;
#ifdef _POWER_MP
		fdadapter->fdhandler->flags = INTR_MPSAFE;
#else
		fdadapter->fdhandler->flags = 0;
#endif /* _POWER_MP */
		fdadapter->fdhandler->level = fdadapter->bus_int_level;
		fdadapter->fdhandler->priority = INTCLASS3;
		fdadapter->fdhandler->bid = fdadapter->bus_id;
		if (i_init(fdadapter->fdhandler) != INTR_SUCC) {
			FD_TRACE1("new state", fdp->drive_state);
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, EIO, devno);
			FD_TRACE1("out open", devno);
			return(EIO);
		}
		fdadapter->int_init = TRUE;

		/*
		 * Initialize the dma services.
		 */

		fdadapter->dma_id = d_init(fdadapter->dma_level, DMA_SLAVE,
		    fdadapter->bus_id);
		if (fdadapter->dma_id == DMA_FAIL) {
			FD_TRACE1("new state", fdp->drive_state);
			fdp->drive_state = FDCLOSED;
			fdcleanup(drive_number);
			unlockl(&fd_lock);
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, EIO, devno);
			FD_TRACE1("out open", devno);
			return(EIO);
		}
		fdadapter->dma_init = TRUE;
	} /* end of adapter and structure initializations. */

	/*
	 * Pin the floppy structure for this drive.
	 */

	rc = pin((caddr_t)fdp, sizeof(struct floppy ));
	if (rc != 0) {
		fdcleanup(drive_number);
		FD_TRACE1("new state", fdp->drive_state);
		fdp->drive_state = FDCLOSED;
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, rc, devno);
		FD_TRACE1("out open", devno);
		return(rc);
	}

	/*
	 * Load the structure 'floppy' with the default values and the minor
	 * device number.
	 */

	fdp->device_number = devno;
	fdadapter->error_value = 0;
	rc = fdload_floppy(minor(devno) & FDTYPEMASK, fdp);
	if (rc != FDSUCCESS) {
		ASSERT(unpin((caddr_t)fdp, sizeof(struct floppy)) == 0);		
		fdcleanup(drive_number);
		FD_TRACE1("new state", fdp->drive_state);
		fdp->drive_state = FDCLOSED;
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, rc, devno);
		FD_TRACE1("out open", devno);
		return(rc);
	}

	/*
	 * If this is the first open after config, clear the reset and
	 * enable interrupts to catch the reset interrupt before
	 * anything else is done with the hardware.
	 */

	fdadapter->reset_performed = FALSE;
	if (fdadapter->first_open == TRUE) {
		fdadapter->error_value = 0;
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS) {
			fdadapter->error_value = rc;
			unlockl(&fd_lock);
			FD_TRACE1("out open", devno);
			return(fdopen_exit(fdp));
		}
		fdadapter->state = FD_INITIAL_INTERRUPT;
		w_start(&(fdadapter->inttimer));
		fdadapter->error_value = 0;
		rc = fdexecute_int_cmd(fdreset);
		if (rc != FDSUCCESS) {
			fdadapter->error_value = rc;
			unlockl(&fd_lock);
			FD_TRACE1("out open", devno);
			return(fdopen_exit(fdp));
		}
		fdtop_unlock_adapter(drive_number);
		fdadapter->state = FD_NO_STATE;
		fdadapter->first_open = FALSE;
	}
	fdp->first_move = TRUE;
		
	/*
	 * Call unlockl to release open lock.
	 */

	unlockl(&fd_lock);

	/*
	 * If the DNDELAY flag is set, the open is done.  Set drive state to
	 * FDOPEN and return.
	 */

	if (devflag & DNDELAY) {
		FD_TRACE1("new state", fdp->drive_state);
		fdp->drive_state = FDOPEN;
		fdadapter->reset_needed = TRUE;
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, FDSUCCESS, devno);
		FD_TRACE1("out open", devno);
		return(FDSUCCESS);
	}

	/*
	 * Lock the adapter while accessing it
	 */

	fdadapter->error_value = 0;
	rc = fdlock_adapter(drive_number);
	if (rc != FDSUCCESS) {
		fdadapter->error_value = rc;
		FD_TRACE1("out open", devno);
		return(fdopen_exit(fdp));
	}


	/*
	 * Check for open drive door.  If so, set rc to ENOTREADY, set
	 * state to FDCLOSED,  and return.  If the door is
	 * closed, just continue.
	 */

	fdadapter->reset_needed = TRUE;
	fdadapter->motor_on = FDNO_DRIVE;
	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value != 0) {
		FD_TRACE1("out open", devno);
		return(fdopen_exit(fdp));
	}
	fdadapter->state = FD_TYPE2_WAKEUP;
	for (i = 0; i < 3; i++) {
		rc = fdexecute_int_cmd(fdrecalibrate);
		if (rc != FDSUCCESS) {
			FD_TRACE1("out open", devno);
			return(fdopen_exit(fdp));
		}
		if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL)
		    == 0)
			break;
	}
	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0) {
		fdadapter->error_value = EIO;
		FD_TRACE1("out open", devno);
		return(fdopen_exit(fdp));
	}
	fdadapter->state = FD_NO_STATE;
	fdadapter->error_value = 0;
	rc = fddoor_check();
	if (rc != FDSUCCESS) {
		fdadapter->error_value = rc;
		FD_TRACE1("out open", devno);
		return(fdopen_exit(fdp));
	}

	/*
	 * Check for write protect if opening for write.
	 */

	if ((devflag & DWRITE) || (devflag & DAPPEND)) {
		fdadapter->error_value = 0;
		fdsense_drive_status();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out open", devno);
			return(fdopen_exit(fdp));
		}
		if (fdadapter->results.un1.names.byte0.status3 & 0x40) {
			fdadapter->error_value = EWRPROTECT;
			FD_TRACE1("out open", devno);
			return(fdopen_exit(fdp));
		} /* end of SENSE_DRIVE_STATUS command processing */
	} /* end of write protect check */

	/*
	 * Call the fdtype() routine. The fdtype() routine will 
	 * either succeed and return FDSUCCESS or fail and return an errno.
	 * If it succeeds, it will load the 'floppy' structure with the correct
	 * diskette characteristics.  See the fdtype() routine comments for
	 * details.
	 */

	fdadapter->error_value = 0;
	rc = fdtype();
	if (rc != FDSUCCESS) {
		fdadapter->error_value = rc;
		FD_TRACE1("out open", devno);
		return(fdopen_exit(fdp));
	}

	/*
	 * Unlock the adapter.
	 */

	fdtop_unlock_adapter(drive_number);

	FD_TRACE1("new state", fdp->drive_state);
	fdp->drive_state = FDOPEN;
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, FDSUCCESS, devno);
	FD_TRACE1("out open", devno);
	return(FDSUCCESS);
} /* end of fd_open */



/*
 * NAME: fd_close
 *
 * FUNCTION: The close entry point.
 *  This routine removes a diskette device from use.  This includes
 *  making sure the interrupt and dma services are properly cleared
 *  and that all outstanding i/o requests have been processed.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *  intr      struct - Interrupt handler structure.
 *
 * INPUTS:
 *  devno - the major & minor device numbers.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: The following errno values may be returned:
 *  EINVAL - invalid minor number. (drive or diskette type).
 *
 * EXTERNAL PROCEDURES CALLED:
 *  e_sleep, d_clear, lockl, unlockl
 */

int	fd_close (dev_t devno)
{
	register struct floppy *fdp;
	int	drive_number, rc;

	DDHKWD1(HKWD_DD_FDDD, DD_ENTRY_CLOSE, 0, devno);
	FD_TRACE1("in close", devno);
	lockl(&fd_lock, LOCK_SHORT);
	rc = FDSUCCESS;
	drive_number = minor(devno) >> 6;
	if (drive_number < 0 || drive_number >= FDMAXDRIVES) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CLOSE, ENXIO, devno);
		FD_TRACE1("bad drive #", devno);
		FD_TRACE1("out close", devno);
		return(ENXIO);
	}
	fdp = fdadapter->drive_list[drive_number];

	/*
	 * See if this drive has been deleted.  If so, return.
	 */

	if (fdp == NULL) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CLOSE, ENXIO, devno);
		FD_TRACE1("deleted drv", devno);
		FD_TRACE1("out close", devno);
		return(ENXIO);
	}

	if ((fdp->drive_state == FDCLOSING) || (fdp->drive_state == FDCLOSED)) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CLOSE, EBUSY, devno);
		FD_TRACE1("drive busy", devno);
		FD_TRACE1("out close", devno);
		return(EBUSY);
	}

	if (devno != fdp->device_number) {
		unlockl(&fd_lock);
		DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CLOSE, EBUSY, devno);
		FD_TRACE1("wrong devno", devno);
		FD_TRACE1("out close", devno);
		return(EBUSY);
	}


	FD_TRACE1("new state", fdp->drive_state);
	fdp->drive_state = FDCLOSING;
	unlockl(&fd_lock);

	/* 
	 * Call fdqueue_check() to check if queue is empty. It
	 * will sleep until processing is complete.
	 */

	fdqueue_check(fdp, drive_number);

	/*
         * Get control of the adapter, turn off the drive, and
         * call fdcleanup to finish processing.
         */

	fdlock_adapter(drive_number);
	fddisable_controller();
	fdcleanup(drive_number);
	fdtop_unlock_adapter(drive_number);

	rc = unpin((caddr_t)fdp, sizeof(struct floppy));
	ASSERT(rc == 0);

	/* 
	 * Mark this drive as FDCLOSED and return.
	 */

	FD_TRACE1("new state", fdp->drive_state);
	fdp->drive_state = FDCLOSED;
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_CLOSE, FDSUCCESS, devno);
	FD_TRACE1("out close", devno);
	return(FDSUCCESS);
} /* end of fd_close() */



/*
 * NAME: fd_read
 *
 * FUNCTION: The raw i/o read entry point.
 *  This routine handles raw i/o read requests.  The uphysio() kernel
 *  service handles all of the processing necessary to transform the
 *  raw i/o request to a block i/o request including calling the
 *  strategy routine.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  floppy struct - Per device information.
 *
 * INPUTS:
 *  devno - the major & minor device numbers.
 *  uiop  - a pointer to a uio structure specifying caller's data.
 *
 * RETURN VALUE DESCRIPTION: returns value returned by uphysio().
 *
 * ERROR CODES: The error codes are all generated by uphysio().
 *
 * EXTERNAL PROCEDURES CALLED: uphysio
 */

int	fd_read (dev_t  devno, register struct uio *uiop)
{
	extern	int	fdnull();
	int	rc;

	DDHKWD1(HKWD_DD_FDDD, DD_ENTRY_READ, 0, devno);
	FD_TRACE1("in read", devno);
	rc = uphysio(uiop, B_READ, 2, devno, fd_strategy, fdnull, NULL);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_READ, rc, devno);
	FD_TRACE1("out read", devno);
	return(rc);
}



/*
 * NAME:fd_write
 *
 * FUNCTION: The raw i/o write entry point.
 *  This routine handles raw i/o write requests.  The uphysio() kernel
 *  service handles all of the processing necessary to transform the
 *  raw i/o request to a block i/o request including calling the
 *  strategy routine.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  floppy struct - Per device information.
 *
 * INPUTS:
 *   devno - the major & minor device numbers.
 *   uiop  - a pointer to a uio structure specifying caller's data.
 *
 * RETURN VALUE DESCRIPTION: returns value returned by uphysio().
 *
 * ERROR CODES: The error codes are all generated by uphysio().
 *
 * EXTERNAL PROCEDURES CALLED: uphysio
 */

int	fd_write (dev_t  devno, register struct uio *uiop)
{
	extern	int	fdnull();
	int	rc;

	DDHKWD1(HKWD_DD_FDDD, DD_ENTRY_WRITE, 0, devno);
	FD_TRACE1("in write", devno);
	rc = uphysio(uiop, B_WRITE, 2, devno, fd_strategy, fdnull, NULL);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_WRITE, rc, devno);
	FD_TRACE1("out write", devno);
	return(rc);
}




/*
 * NAME: fd_ioctl
 *
 * FUNCTION: The ioctl entry point.
 *  This routine handles most all of the i/o besides data reads and
 *  writes and provides the diagnostics interface to the diskette
 *  drives.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only by a process and can page fault.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *  devinfo   struct - A standard system structure used with the
 *                     IOCINFO ioctl.
 *  fdinfo    struct - Used to pass information for the
 *                     FDIOCGINFO and the FDIOCSINFO ioctls.
 *  fd_status struct - Used to pass the status of the drive and
 *                     the device driver with the FDIOCSTATUS
 *                     ioctl.
 *  fdparms   struct - Used to pass information for the
 *                     FDIOCGETPARMS and FDIOCSETPARMS ioctls.
 *
 * INPUTS:
 *  devno - a pointer to a buffer header structure.
 *  op    - the ioctl operation to perform.
 *  arg   - a parameter used to pass data to and from user space.
 *          The actual use is dependent on the operation.
 *
 * RETURN VALUE DESCRIPTION: none.
 *
 * ERROR CODES: The following errno values may be returned:
 *  EINVAL - either the 'op' parameter, the 'arg' parameter, or
 *           data pointed to by the 'arg' parameter is invalid.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  lockl, unlockl, pin, unpin,copyin, copyout, xmalloc, xmfree,
 *  xmemat, xmemdt, xmemin, malloc, free
 */

int	fd_ioctl (dev_t devno, int op, register long arg, ulong devflag)
{
	register struct floppy *fdp;
	struct devinfo devinfo;
	struct fdinfo fdinfo;
	struct fd_status fdstatus;
	struct fdparms fdparms;
	int	drive_number, rc = FDSUCCESS, local_error_value;

	DDHKWD5(HKWD_DD_FDDD, DD_ENTRY_IOCTL, 0, devno, op, 0, 0, 0);
	FD_TRACE1("in ioctl", devno);
	lockl(&fd_lock, LOCK_SHORT);
	drive_number = minor(devno) >> 6;
	fdp = fdadapter->drive_list[drive_number];

	switch (op) {

	/* 
	 * The following ioctl operation is defined for every device
	 * that uses the ioctl interface.
	 *
	 *
	 *  IOCINFO - returns some information about the diskette.
	 *      This is a standard ioctl option that can be issued
	 *      to find out information about any device that uses
	 *      ioctls.  A pointer to a structure of type devinfo
	 *      should be passed in the 'arg' parameter.  The
	 *      information about the diskette will be loaded
	 *      into the devinfo structure.
	 */

	case IOCINFO:
		FD_TRACE1("IOCINFO", devno);
		devinfo.devtype = DD_DISK;
		devinfo.flags = DF_RAND;
		devinfo.un.dk.bytpsec = (short)(fdp->bytes_per_sector);
		devinfo.un.dk.secptrk = (short)(fdp->sectors_per_track);
		devinfo.un.dk.trkpcyl = (short)(fdp->tracks_per_cylinder);
		devinfo.un.dk.numblks = (long)(fdp->number_of_blocks);

		/* 
		 * Call copyout to copy the devinfo structure
		 * to the user.
		 */

		rc = copyout((char *)(&devinfo), (char *)(arg),
		    sizeof(struct devinfo ));
		if (rc != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL, rc, devno);
			unlockl(&fd_lock);
			FD_TRACE1("out ioctl", devno);
			return(rc);
		}
		break; /* end of IOCINFO */

	/*
	 * The following ioctl operations are provided for the
	 * format command.
	 *
	 * 
	 *  FDIOCFORMAT - formats a diskette track.  A track is
	 *      formatted using values passed in an array of bytes 
	 *      pointed to by the 'arg' parameter.  The buffer should
	 *      be 4 times the number of sectors per track long.  Four
	 *      bytes of data are needed for each sector on the track.  
	 *      The following shows the structure of the data buffer:
	 *                        _________________ 
	 *                       |                 |
	 *                       | cylinder number | - byte 0
	 *               S       |_________________|
	 *               E       |                 |
	 *               C       |   side number   | - byte 1
	 *               T       |_________________|
	 *               O       |                 |
	 *               R       |  sector number  | - byte 2
	 *                       |_________________|
	 *               0       |                 |
	 *                       | # bytes/sector  | - byte 3
	 *                       |_________________|
	 * 
	 *                                .
	 *                                .
	 *                                .
	 *                        _________________
	 *                       |                 |
	 *                       | cylinder number | - byte (n * 4) - 4
	 *               S       |_________________|
	 *               E       |                 |
	 *               C       |   side number   | - byte (n * 4) - 3
	 *               T       |_________________|
	 *               O       |                 |
	 *               R       |  sector number  | - byte (n * 4) - 2
	 *                       |_________________|
	 *               n       |                 |
	 *                       | # bytes/sector  | - byte (n * 4) - 1
	 *                       |_________________|
	 *
	 *              where n is the number of sectors per track.  In
	 *      general, the cylinder number, side number, and number
	 *      of bytes per sector should not change from sector to
	 *      sector.  The sector number should be different for each
	 *      sector.  Usually the sector numbers will correspond to
	 *      the physical sector numbers, but they can be different
	 *      if some special copy protection scheme is being used.
	 */

	case FDIOCFORMAT:
		FD_TRACE1("FDIOCFORMAT", devno);
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocformat(arg);
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCFORMAT case */

	/*
	 *  FDIOCGINFO - gets the current diskette characteristics.
 	 *      A pointer to a structure of type fdinfo must be passed
	 *      in the 'arg' parameter.  This operation will load the
	 *      fdinfo structure with the current diskette parameters.
	 */

	case FDIOCGINFO:
		FD_TRACE1("FDIOCGINFO", devno);
		fdinfo.type = (short)(fdp->drive_type);
		fdinfo.nsects = (int)(fdp->sectors_per_track);
		fdinfo.sides = (int)(fdp->tracks_per_cylinder);
		fdinfo.ncyls = (int)(fdp->cylinders_per_disk);

		/*   
		 * Call copyout to copy the fdinfo structure to the
		 * user.
		 */

		rc = copyout((char *)(&fdinfo), (char *)(arg),
		    sizeof(struct fdinfo ));
		if (rc != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL, rc, devno);
			unlockl(&fd_lock);
			FD_TRACE1("out ioctl", devno);
			return(rc);
		}
		break; /* end of FDIOCGINFO case */

	/*
	 *  FDIOCSINFO - sets the current diskette characteristics.  A
	 *      structure of type fdinfo must be loaded with the
	 *      desired values and a pointer to this structure must be
	 *      passed to the device driver in the 'arg' parameter.
	 */

	case FDIOCSINFO:
		FD_TRACE1("FDIOCSINFO", devno);

		/*   
		 * Call copyin to get the fdinfo structure from the user.
		 */

		rc = copyin((char *)(arg), (char *)(&fdinfo),
		    sizeof(struct fdinfo ));
		if (rc != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL, rc, devno);
			unlockl(&fd_lock);
			FD_TRACE1("out ioctl", devno);
			return(rc);
		}
		switch (fdp->drive_type = (uchar)fdinfo.type) {
		case D_96: /* 1.2M, 5.25" drive */
			switch (fdinfo.ncyls) {
			case 40: /* 360K diskette */
				switch (fdinfo.sides) {
				case 2:
					switch (fdinfo.nsects) {
					case 9:
						rc = fdload_floppy(FD360_5,
						    fdp);
						break;
					default:
						rc = EINVAL;
						break;
					} /* end of switch on nsects */
					break; /* end of case 2 */
				default:
					rc = EINVAL;
					break;
				} /* end of switch on sides */
				break; /* end of case 40 */
			case 80: /* 1.2M diskette */
				if ((fdinfo.nsects != 15) ||
				    (fdinfo.sides != 2)) {
					rc = EINVAL;
				} else 
					rc = fdload_floppy(FD1200_5, fdp);
				break; /* end of case 80 */
			default:
				rc = EINVAL;
				break;
			} /* end of switch on ncyls */
                         break;
	                  /* end of case D_96 */
		case D_135H: /* 1.44M, 3.5" drive */
			if ((fdinfo.sides != 2) || (fdinfo.ncyls != 80)) {
				rc = EINVAL;
			} else
			 {
				switch (fdinfo.nsects) {
				case 9: /* 720K diskette */
					rc = fdload_floppy(FD720_3, fdp);
					break; /* end of case 9 */
				case 18: /* 1.44M diskette */
					rc = fdload_floppy(FD1440_3, fdp);
					break; /* end of case 18 */
				default:
					rc = EINVAL;
					break;
				} /* end of switch on nsects */
			}
		                 /* end of D_135H case */
	        case D_1354H: /* 2.88M, 3.5" drive */
			if ((fdinfo.sides != 2) || (fdinfo.ncyls != 80)) {
				rc = EINVAL;
			} else
		 	{	
				switch (fdinfo.nsects) {
				case 9: /* 720K diskette */
					rc = fdload_floppy(FD720_3, fdp);
					break; /* end of case 9 */
				case 18: /* 1.44M diskette */
					rc = fdload_floppy(FD1440_3, fdp);
					break; /* end of case 18 */
				case 36: /* 2.88M diskette */
					rc = fdload_floppy(FD2880_3, fdp);
					break; /* end of case 36 */
				default:
					rc = EINVAL;
					break;
				} /* end of switch on nsects */
			}
			break; /* end of D_1354H case */
		default:
			rc = EINVAL;
			break;
		} /* end of switch on drive_type */
		break; /* end of FDIOCSINFO case */
	/*
	 * The following ioctl operations are provided for the
	 * diagnostic commands and utilities.
	 */

	/*
	 *  FDIOCDSELDRV - deselects and turns off the motor of the 
	 *      drive.
	 */

	case FDIOCDSELDRV:
		FD_TRACE1("FDIOCDSELDR", devno);

		/*
		 * This leaves dma and interrupts enabled but deselects
		 * all drives.
		 */

		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocdseldrv();
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCDSELDRV case */

	/*
	 *  FDIOCNORETRY - disables retries on errors.
	 *      This operation requires RAS_CONFIG authority to
	 *      execute it.
	 */

	case FDIOCNORETRY:
		FD_TRACE1("FDIOCNORTRY", devno);
		rc = privcheck(RAS_CONFIG);
		if (rc == 0)
			fdp->retry_flag = FALSE;
		break; /* end of FDIOCNORETRY case */

	/*
	 *  FDIOCREADID - causes the diskette controller to read the
	 *      first address field that it finds under the drive head
	 *      whose number is passed in the 'arg' parameter.
	 */

	case FDIOCREADID:
		FD_TRACE1("FDIOCREDID", devno);
		if ((arg < 0) || (arg > 1)) {
			rc = EINVAL;
		} else 
		 {
			rc = fdlock_adapter(drive_number);
			if (rc != FDSUCCESS)
				break;
			fdp->head_id = (uchar)arg;
			rc = fdiocreadid();
			fdtop_unlock_adapter(drive_number);
		}
		break; /* end of FDIOCREADID case */

	/*
	 *  FDIOCRECAL - recalibrates the diskette drive.  The 'arg' parameter
	 *      is unused for this operation since no data needs to be passed
	 *      to the driver and since the diskette controller RECALIBRATE
	 *      command has no result phase.
	 */

	case FDIOCRECAL:
		FD_TRACE1("FDIOCRECAL", devno);
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocrecal();
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCRECAL case */

	/*
	 *  FDIOCRESET - resets the diskette controller.
	 *      The 'arg' parameter is unused for this function since there is
	 *      no data to be passed.
	 */

	case FDIOCRESET:
		FD_TRACE1("FDIOCRESET", devno);
	/*
	 *  FDIOCRESET - reset the diskette controller chip.
	 *      A delay is done as the motor, if on, will be restarted.
	 */
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocreset();
		delay(fdp->motor_ticks);
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCRESET */

	/*
	 *  FDIOCRETRY - enables retries on errors.
	 *      This operation requires RAS_CONFIG authority to execute it.
	 */

	case FDIOCRETRY:
		FD_TRACE1("FDIOCRETRY", devno);
		rc = privcheck(RAS_CONFIG);
		if (rc == 0)
			fdp->retry_flag = TRUE;
		break; /* end of FDIOCNORETRY case */

	/*
	 *  FDIOCSEEK - moves the drive heads to the designated cylinder.  The
	 *      cylinder number that the drive heads are to be moved to should
	 *      be passed in the 'arg' parameter.
	 */

	case FDIOCSEEK:
		FD_TRACE1("FDIOCSEEK", devno);
		switch (fdp->drive_type) {
		case D_96: /* 1.2M, 5.25" drive */
			if (fdp->diskette_type & FD40CYLS) /* 360K diskette */
			    {
				if ((arg < 0 ) || (arg > 39)) {
					DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL,
					    EINVAL, devno);
					unlockl(&fd_lock);
					FD_TRACE1("out ioctl", devno);
					return(EINVAL);
				}
				fdp->cylinder_id = (uchar)arg;
			} else /* 1.2M diskette */
			 {
				if ((arg < 0 ) || (arg > 79)) {
					DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL,
					    EINVAL, devno);
					unlockl(&fd_lock);
					FD_TRACE1("out ioctl", devno);
					return(EINVAL);
				}
				fdp->cylinder_id = (uchar)arg;
			}
			break; /* end of D_96 case */
		case D_135H: /* 1.44M, 3.5" drive */
		case D_1354H: /* 2.88M, 3.5" drive */
			if ((arg < 0 ) || (arg > 79)) {
				DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL,
				    EINVAL, devno);
				unlockl(&fd_lock);
				FD_TRACE1("out ioctl", devno);
				return(EINVAL);
			}
			fdp->cylinder_id = (uchar)arg;
			break; /* end of D_1354H case */
		}
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;

		rc = fdiocseek();
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCSEEK case */

	/*
	 *  FDIOCSELDRV - selects and turns on the motor of the drive.
	 */

	case FDIOCSELDRV:
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocseldrv();
		delay(fdp->motor_ticks);
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCSELDRV case */

	/*
	 *  FDIOCSETTLE - tests the head settle time passed in the arg
	 *      parameter.  A negative arg value means to test using the
	 *      default value for the drive.  A positive arg value is a new
	 *      settle time (in milliseconds) to test.
	 */

	case FDIOCSETTLE:
		FD_TRACE1("FDIOCSETTLE", devno);
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocsettle(arg);
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCSETTLE case */

	/*
	 *  FDIOCSPEED - determines the rotational speed of the drive in rpms.
	 *      The value can be read using the FDIOCSTATUS ioctl.
	 */

	case FDIOCSPEED:
		FD_TRACE1("FDIOCSPEED", devno);
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocspeed();
		fdtop_unlock_adapter(drive_number);
		break; /* end of FDIOCSPEED case */

	/*
	 *  FDIOCSTATUS - returns the status of the diskette drive and device
	 *      driver.  A pointer to a structure of type fd_status should be
	 *      be passed in the 'arg' parameter.  The driver and device
	 *      driver status will be loaded into the fd_status structure.
	 *      The result phase portion of the status is only valid if the
	 *      device driver was opened in raw mode.  Otherwise, there is no
	 *      way of knowing to which i/o operation the result phase
	 *      corresponds.
	 */

	case FDIOCSTATUS:
		FD_TRACE1("FDIOCSTATUS", devno);
		rc = fdlock_adapter(drive_number);
		if (rc != FDSUCCESS)
			break;
		rc = fdiocstatus(&fdstatus);
		fdtop_unlock_adapter(drive_number);
		if (rc == FDSUCCESS) {

			/*   
	 		 * Call copyout to copy the fd_status structure
			 * to the user.
	 		 */

			rc = copyout((char *)(&fdstatus), (char *)(arg),
			    sizeof(struct fd_status ));
		}
		break; /* end of FDIOCSTATUS case */

	/* 
	 * The following two ioctls are designed for use by the PC simulator.
	 */

	/*
	 * FDIOCGETPARMS - gets various drive and diskette parameters and
	 *     returns them to the caller.
	 */

	case FDIOCGETPARMS:
		FD_TRACE1("FDIOCGETPMS", devno);
		fdparms.diskette_type = fdp->diskette_type;
		fdparms.sector_size = fdp->sector_size;
		fdparms.sectors_per_track = fdp->sectors_per_track;
		fdparms.sectors_per_cylinder = fdp->sectors_per_cylinder;
		fdparms.tracks_per_cylinder = fdp->tracks_per_cylinder;
		fdparms.cylinders_per_disk = fdp->cylinders_per_disk;
		fdparms.data_rate = fdp->data_rate;
		fdparms.head_settle_time = fdp->head_settle_time;
		fdparms.head_load = fdp->head_load;
		fdparms.fill_byte = fdp->fill_byte;
		fdparms.step_rate = fdp->step_rate;
		fdparms.step_rate_time = fdp->step_rate_time;
		fdparms.gap = fdp->gap;
		fdparms.format_gap = fdp->format_gap;
		fdparms.data_length = fdp->data_length;
		fdparms.motor_off_time = fdp->motor_off_time;
		fdparms.bytes_per_sector = fdp->bytes_per_sector;
		fdparms.number_of_blocks = fdp->number_of_blocks;
		fdparms.motor_start = fdp->motor_start;
		fdparms.motor_ticks = fdp->motor_ticks;

		/*   
		 * Call copyout to copy the fdparms structure to the user.
		 */

		rc = copyout((char *)(&fdparms), (char *)(arg),
		    sizeof(struct fdparms ));
		break; /* end of FDIOCGETPARMS case */

	/*
	 * FDIOCSETPARMS - sets various drive and diskette parameters from
	 *     values passed in from the caller.
	 */

	case FDIOCSETPARMS:
		FD_TRACE1("FDIOCSETPMS", devno);

		/*   
		 * Call copyin to get the fdparms structure from the user.
		 */

		rc = copyin((char *)(arg), (char *)(&fdparms),
		    sizeof(struct fdparms ));
		if (rc != 0) {
			DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL, rc, devno);
			unlockl(&fd_lock);
			FD_TRACE1("out ioctl", devno);
			return(rc);
		}
		fdp->diskette_type = fdparms.diskette_type;
		fdp->sector_size = fdparms.sector_size;
		fdp->sectors_per_track = fdparms.sectors_per_track;
		fdp->sectors_per_cylinder = fdparms.sectors_per_cylinder;
		fdp->tracks_per_cylinder = fdparms.tracks_per_cylinder;
		fdp->cylinders_per_disk = fdparms.cylinders_per_disk;
		fdp->data_rate = fdparms.data_rate;
		fdp->head_settle_time = fdparms.head_settle_time;
		fdp->head_load = fdparms.head_load;
		fdp->fill_byte = fdparms.fill_byte;
		fdp->step_rate = fdparms.step_rate;
		fdp->step_rate_time = fdparms.step_rate_time;
		fdp->gap = fdparms.gap;
		fdp->format_gap = fdparms.format_gap;
		fdp->data_length = fdparms.data_length;
		fdp->motor_off_time = fdparms.motor_off_time;
		fdp->bytes_per_sector = fdparms.bytes_per_sector;
		fdp->number_of_blocks = fdparms.number_of_blocks;
		fdp->motor_start = fdparms.motor_start;
		fdp->motor_ticks = fdparms.motor_ticks;
		fdadapter->mottimer.restart = fdparms.motor_off_time;
		break; /* end of FDIOCSETPARMS case */
	default:
		rc = EINVAL;
		break; /* end of default case */
	} /* end of switch on op */
	unlockl(&fd_lock);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_IOCTL, rc, devno);
	FD_TRACE1("out ioctl", devno);
	return(rc);
} /* end of fd_ioctl() */


/*
 * The following are the top half internal device driver routines.
 * All routines assume that the adapter lock has been secured by the 
 * calling routine.
 */



/*
 * NAME: fddoor_check
 *
 * FUNCTION: Checks to see if the drive door is open.
 *  This routine checks the diskette changed line to see if the door
 *  has been opened.  If it has, it attempts to clear the change line
 *  by moving the drive heads.  It then checks the change line again
 *  and reports success if the door is closed and failure if it is
 *  still open.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only from the process level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS: None.
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - the drive door is not currently open.
 *
 * ERROR CODES:
 *  ENOTREADY - the door is still open, or the hardware timed out.
 *  EIO       - possible return code from the hardware accesses.
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

int	fddoor_check ()
{
	uchar	change_line_value;
	struct floppy *fdp;
	int	rc;

	FD_TRACE1("in doorchk", FDA_DEV);
	fdp = fdadapter->drive_list[fdadapter->active_drive];
	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value == 0) {
		change_line_value = fdread_change_line();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out doorchk", FDP_DEV);
			return(fdadapter->error_value);
		}
		fdadapter->state = FD_TYPE2_WAKEUP;
		if (change_line_value & FDDISK_CHANGE) {
			rc = fdexecute_int_cmd(fdrecalibrate);
			if (rc != FDSUCCESS) {
				fdadapter->state = FD_NO_STATE;
				FD_TRACE1("out doorchk", FDP_DEV);
				return(rc);
			}
			fdp->cylinder_id = 1;
			rc = fdexecute_int_cmd(fdseek);
			if (rc != FDSUCCESS) {
				fdadapter->state = FD_NO_STATE;
				FD_TRACE1("out doorchk", FDP_DEV);
				return(rc);
			}
			fdp->first_move = FALSE;
			fdadapter->error_value = 0;
			change_line_value = fdread_change_line();
			if (fdadapter->error_value != 0) {
				fdadapter->state = FD_NO_STATE;
				FD_TRACE1("out doorchk", FDP_DEV);
				return(fdadapter->error_value);
			}
			if (change_line_value & FDDISK_CHANGE) {
				fdadapter->state = FD_NO_STATE;
				FD_TRACE1("out doorchk", FDP_DEV);
				return(ENOTREADY);
			}
		}
	}
	FD_TRACE1("out doorchk", FDP_DEV);
	return(FDSUCCESS);
} /* end of fddoor_check() */

int	fdiocdseldrv()
{
	FD_TRACE1("in iocdsel", FDA_DEV);
	fdadapter->error_value = 0;
	fdreset_check();
	if (fdadapter->error_value == 0)
	fdtop_enable_controller();
	FD_TRACE1("out iocdsel", FDA_DEV);
	return(fdadapter->error_value);
}


int 	fdiocformat (long arg)
{
	int	rc, trc, i;
	struct	floppy *fdp;
	uchar	status3;

	fdp = fdadapter->drive_list[fdadapter->active_drive];
	FD_TRACE1("in iocform", FDP_DEV);

	/*
	 * If the last thing done by the driver was a reset then the
	 * controller no longer remembers where this drive is and there is
	 * no way for the driver error detection code to know this since
	 * the diskette does not have valid address marks yet.  Set up 
	 * flags to force a reset, recal, data rate set , and specify to 
	 * get things back in order.
	 */

	if (fdadapter->reset_performed == TRUE) {
		fdadapter->reset_needed = TRUE;	
		fdadapter->motor_on = FDNO_DRIVE;
	}
	if ((rc = fddoor_check()) != FDSUCCESS) {
		FD_TRACE1("out iocform", FDP_DEV);
		return(rc);
	}
	if (fdadapter->reset_performed == TRUE) {
		fdadapter->state = FD_TYPE2_WAKEUP;
		for (i = 0; i < 3; i++) {
			rc = fdexecute_int_cmd(fdrecalibrate);
			if (rc != FDSUCCESS) {
				fdadapter->state = FD_NO_STATE;
				FD_TRACE1("out iocform", FDP_DEV);
				return(rc);
			}
		}
	}
	fdadapter->reset_performed = FALSE;	
	fdadapter->error_value = 0;
	fdsense_drive_status();
	if (fdadapter->error_value != 0) {
		FD_TRACE1("out iocform", FDP_DEV);
		return(fdadapter->error_value);
	}
	status3 = fdadapter->results.un1.names.byte0.status3;
	if (status3 & 0x40) {
		fdadapter->error_value = EWRPROTECT;
		FD_TRACE1("out iocform", FDP_DEV);
		return(fdadapter->error_value);
	}
	fdp->format_size = fdp->sectors_per_track * 4;

	/* 
	 * allocate a page for dma transfers.
	 */

	if ((fdadapter->format_buffer = (char *)xmalloc(PAGESIZE, PGSHIFT,
	     pinned_heap)) == NULL) {
		FD_TRACE1("out iocform", FDP_DEV);
		return(ENOMEM);
	}
	fdp->xmem.aspace_id = XMEM_GLOBAL;

	/*        
	 * copy in the buffer from user space.
	 */

	rc = copyin((char *)arg, fdadapter->format_buffer, fdp->format_size);
	if (rc != 0) {
		trc = xmfree((caddr_t)fdadapter->format_buffer,
		    (caddr_t)pinned_heap);
		ASSERT(trc == 0);
		FD_TRACE1("out iocform", FDP_DEV);
		return(rc);
	}
	fdp->current.head = *(fdadapter->format_buffer + 1);
	fdp->cylinder_id = *(fdadapter->format_buffer);
	if (fdp->cylinder_id == 0) {
	
		/*
		 * If drive heads are not at cylinder 0 and that is the
		 * cylinder to be formatted then recalibrate.
		 */

		if ((status3 & 0x10) != 0x10) {
			fdadapter->state = FD_TYPE2_WAKEUP;
			for (i = 0; i < 3; i++) {
				rc = fdexecute_int_cmd(fdrecalibrate);
				if (rc != FDSUCCESS) {
					fdadapter->state = FD_NO_STATE;
					rc = xmfree((caddr_t)
					    fdadapter->format_buffer,
					    (caddr_t)pinned_heap);
					ASSERT(rc == 0);
					FD_TRACE1("out iocform", FDP_DEV);
					return(rc);
				}
			}
		}
	}

	/*
	 * seek to cylinder to be formatted.
	 */

	fdadapter->state = FD_TYPE2_WAKEUP;
	rc = fdexecute_int_cmd(fdseek);
	if (rc != FDSUCCESS) {
		fdadapter->state = FD_NO_STATE;
		rc = xmfree((caddr_t)fdadapter->format_buffer,
		    (caddr_t)pinned_heap);
		ASSERT(rc == 0);
		FD_TRACE1("out iocform", FDP_DEV);
		return(rc);
	}

	/*
	 * If seek was not successful then recalibrate and retry it.
	 */

	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0) {
		if (fdp->retry_flag == FALSE) {
			fdadapter->state = FD_NO_STATE;
			rc = xmfree((caddr_t)fdadapter->format_buffer,
			    (caddr_t)pinned_heap);
			ASSERT(rc == 0);
			FD_TRACE1("out iocform", FDP_DEV);
			return(EIO);
		}
		for (i = 0; i < 3; i++) {
			rc = fdexecute_int_cmd(fdrecalibrate);
			if (rc != FDSUCCESS) {
				fdadapter->state = FD_NO_STATE;
				rc = xmfree((caddr_t)fdadapter->format_buffer,
				    (caddr_t)pinned_heap);
				ASSERT(rc == 0);
				FD_TRACE1("out iocform", FDP_DEV);
				return(rc);
			}
		}
		rc = fdexecute_int_cmd(fdseek);
		if (rc != FDSUCCESS) {
			fdadapter->state = FD_NO_STATE;
			rc = xmfree((caddr_t)fdadapter->format_buffer,
		    	(caddr_t)pinned_heap);
			ASSERT(rc == 0);
			FD_TRACE1("out iocform", FDP_DEV);
			return(rc);
		}
		if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL)
		    != 0) {
			fdadapter->state = FD_NO_STATE;
			rc = xmfree((caddr_t)fdadapter->format_buffer,
			    (caddr_t)pinned_heap);
			ASSERT(rc == 0);
			FD_TRACE1("out iocform", FDP_DEV);
			return(EIO);
		}
	}

	/*
	 * format the track.
	 */

	fdp->xmem.aspace_id = XMEM_GLOBAL;
#ifdef _IBMRT
	d_unmask(fdadapter->dma_id);
#endif
	d_slave(fdadapter->dma_id, 0, fdadapter->format_buffer,
 	    fdp->format_size, &fdp->xmem);
	fdadapter->state = FD_FORMAT;
	rc = fdexecute_int_cmd(fdformat_track);
	if (rc != FDSUCCESS) {
		if (fdadapter->d_comp_needed == TRUE) {
			trc = d_complete(fdadapter->dma_id, 0,
			    fdadapter->format_buffer,
			    fdp->format_size, &fdp->xmem, NULL);
			FD_TRACE1("call d_comp", trc);
			ASSERT(trc == 0);
		}
		fdadapter->error_value = 0;
		fdreset_check();
		rc = fdadapter->error_value;
		if (rc != 0) {
			trc = xmfree((caddr_t)fdadapter->format_buffer,
			    (caddr_t)pinned_heap);
			ASSERT(trc == 0);
			FD_TRACE1("out iocform", FDP_DEV);
			return(rc);
		} else
		 {
			fdadapter->state = FD_FORMAT;
			rc = fdexecute_int_cmd(fdformat_track);
			if (rc != FDSUCCESS) {
				fdadapter->state = FD_NO_STATE;
				if (fdadapter->d_comp_needed == TRUE) {
					trc = d_complete(fdadapter->dma_id, 0,
					    fdadapter->format_buffer,
					    fdp->format_size, &fdp->xmem, NULL);
					FD_TRACE1("call d_comp", trc);
					ASSERT(trc == 0);
				}
				trc = xmfree((caddr_t)fdadapter->format_buffer,
				    (caddr_t)pinned_heap);
				ASSERT(trc == 0);
				FD_TRACE1("out iocform", FDP_DEV);
				return(rc);
			}
		}
	}
	rc = xmfree((caddr_t)fdadapter->format_buffer, (caddr_t)pinned_heap);
	ASSERT(rc == 0);
	FD_TRACE1("out iocform", FDP_DEV);
	return(FDSUCCESS);
} /* end of fdiocformat() */



int	fdiocreadid ()
{
	int	rc;

	FD_TRACE1("in iocrdid", FDA_DEV);
	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value == 0) {
		fdadapter->state = FD_TYPE1_WAKEUP;
		rc = fdexecute_int_cmd(fdreadid);
		fdadapter->state = FD_NO_STATE;
	} else
	 {
		rc = fdadapter->error_value;
	}
	FD_TRACE1("out iocrdid", FDA_DEV);
	return(rc);
}




int	fdiocrecal ()
{
	int	i, rc;

	FD_TRACE1("in iocrecl", FDA_DEV);
	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value == 0) {
		fdadapter->state = FD_TYPE2_WAKEUP;
		for (i = 0; i < 3; i++) {
			rc = fdexecute_int_cmd(fdrecalibrate);
			if (rc != FDSUCCESS)
				break;
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) == 0) 
				break;
		}
		fdadapter->state = FD_NO_STATE;
	} else
	 {
		rc = fdadapter->error_value;
	}
	FD_TRACE1("out iocrecl", FDA_DEV);
	return(rc);
} /* end of fdiocrecal() */



int	fdiocreset ()
{
	int	rc;

	FD_TRACE1("in iocrset", FDA_DEV);
	fdadapter->state = FD_TYPE2_WAKEUP;
	rc = fdexecute_int_cmd(fdreset);
	fdadapter->state = FD_NO_STATE;
	FD_TRACE1("out iocrset", FDA_DEV);
	return(rc);
} /* end of fdiocreset */



/*
 * NAME: fdiocsettle
 *
 * FUNCTION: Tests a head settle time value.
 *  This routine checks to see if a particular head settle time
 *  works for a drive.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only from the process level.
 *
 * DATA STRUCTURES: none.
 *
 * INPUTS:
 *  fdp - a pointer to the floppy structure to use.
 *  arg - the head settle value to test.
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - the head settle time worked.
 *  ENOMEM    - the xmalloc failed.
 *  EBUSY     - the read/write queues are not empty.
 *  ENOTREADY - the drive took an extra revolution before writing.
 *  EIO       - the drive wrote bad data.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  xmalloc, pin, xmfree, delay,
 *  talloc, d_slave, tfree
 */

int	fdiocsettle (long arg)
{
	int	rc, trc, i;
	uchar old_load;
	register struct floppy *fdp;

	FD_TRACE1("in iocsett", FDA_DEV);
	fdp = fdadapter->drive_list[fdadapter->active_drive];
	rc = fddoor_check();
	if (rc != FDSUCCESS) {
		FD_TRACE1("out iocsett", FDA_DEV);
		return(rc);
	}
	fdadapter->state = FD_TYPE2_WAKEUP;
	for (i = 0; i < 3; i++) {
		rc = fdexecute_int_cmd(fdrecalibrate);
		if (rc != FDSUCCESS) {
			fdadapter->state = FD_NO_STATE;
			FD_TRACE1("out iocsett", FDA_DEV);
			return(rc);
		}
		if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL)
		    == 0)
			break;
	}
	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0) {
		fdadapter->state = FD_NO_STATE;
		FD_TRACE1("out iocsett", FDA_DEV);
		return(EIO);
	} /* end of RECALIBRATE processing */
	fdadapter->state = FD_NO_STATE;
	fdadapter->speed_buffer = (char * )xmalloc(PAGESIZE, PGSHIFT,
	    pinned_heap);
	if (fdadapter->speed_buffer == NULL) {
		FD_TRACE1("out iocsett", FDA_DEV);
		return(ENOMEM);
	}
	fdadapter->fdsettle_timer = talloc();
	old_load = fdp->head_load;
	fdp->head_load = 0;
	fdadapter->error_value = 0;
	fdspecify();
	if (fdadapter->error_value != 0) {
		fdadapter->state = FD_NO_STATE;
		fdadapter->reset_needed = TRUE;
		tfree(fdadapter->fdsettle_timer);
		trc = xmfree((caddr_t)fdadapter->speed_buffer,
		    pinned_heap);
		ASSERT(trc == 0);
		FD_TRACE1("out iocsett", FDA_DEV);
		return(fdadapter->error_value);
	}
	fdp->head_settle_time = arg;
	fdadapter->xdp.aspace_id = XMEM_GLOBAL;
#ifdef _IBMRT
	d_unmask(fdadapter->dma_id);
#endif
	d_slave(fdadapter->dma_id, DMA_READ,
	    (char *)fdadapter->speed_buffer, fdp->bytes_per_sector,
	    &fdadapter->xdp);
	fdadapter->state = FD_SETTLE;
	fdadapter->sub_state = FD_SETTLE_READ1;
	fdp->current.cylinder = 0;
	fdp->current.head = 0;
	fdp->current.sector = 1;
	rc = fdexecute_int_cmd(fdrw_ioctl);
	if (rc != FDSUCCESS) {
		fdp->head_load = old_load;
		fdadapter->error_value = 0;
		fdspecify();
		if (fdadapter->error_value != 0)
			fdadapter->reset_needed = TRUE;
		fdadapter->state = FD_NO_STATE;
		fdadapter->sub_state = NULL;
		if (fdadapter->d_comp_needed == TRUE) {
			trc = d_complete(fdadapter->dma_id, DMA_READ,
			    (char *)fdadapter->speed_buffer,
			    fdp->bytes_per_sector, &fdadapter->xdp, NULL);
			FD_TRACE1("call d_comp", trc);
			ASSERT(trc == 0);
		}
		tfree(fdadapter->fdsettle_timer);
		trc = xmfree((caddr_t)fdadapter->speed_buffer,
		    pinned_heap);
		ASSERT(trc == 0);
		FD_TRACE1("out iocsett", FDA_DEV);
		return(rc);
	}
	if (fdadapter->results.un1.names.byte0.status0 & FDNORMAL) {
		fdadapter->error_value = EIO;
		fdp->head_load = old_load;
		fdadapter->error_value = 0;
		fdspecify();
		if (fdadapter->error_value != 0)
			fdadapter->reset_needed = TRUE;
	} 
	fdadapter->state = FD_NO_STATE;
	fdadapter->sub_state = NULL;
	tfree(fdadapter->fdsettle_timer);
	trc = xmfree((caddr_t)fdadapter->speed_buffer, pinned_heap);
	ASSERT(trc == 0);
	FD_TRACE1("out iocsett", FDA_DEV);
	return(fdadapter->error_value);
} /* end of fdiocsettle() */



int	fdiocseek ()
{
	int	rc;

	FD_TRACE1("in iocseek", FDA_DEV);
	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value == 0) {
		fdadapter->state = FD_TYPE2_WAKEUP;
		rc = fdexecute_int_cmd(fdseek);
		fdadapter->state = FD_NO_STATE;
	} else
	 {
		rc = fdadapter->error_value;
	}
	FD_TRACE1("out iocseek", FDA_DEV);
	return(rc);
} /* end of fdiocseek */



int	fdiocseldrv()
{
	FD_TRACE1("in iocseld", FDA_DEV);
	fdadapter->error_value = 0;
	fdreset_check();
	if (fdadapter->error_value == 0)
	fdtop_select_drive();
	FD_TRACE1("out iocseld", FDA_DEV);
	return(fdadapter->error_value);
}



/*
 * NAME: fdiocspeed
 *
 * FUNCTION: Tests the speed of the diskette drive.
 *  This routine times the rotation of the drive.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only from the process level.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *
 * INPUTS: None
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - the speed was measured successfully
 *  ENOMEM    - the xmalloc failed.
 *  EBUSY     - the read/write queues are not empty.
 *  ENOTREADY - the drive took an extra revolution before writing.
 *  EIO       - the drive wrote bad data.
 *
 * ERROR CODES: none.
 *
 * EXTERNAL PROCEDURES CALLED:
 *  xmalloc, xmfree
 *  talloc, d_slave
 *  d_complete, d_unmask, tfree
 */

int	fdiocspeed ()
{
	int	rc, trc, i;
	ulong elapsed_time, time_sum = 0;
	register struct floppy *fdp;

	FD_TRACE1("in iocsped", FDA_DEV);
	fdp = fdadapter->drive_list[fdadapter->active_drive];
	rc = fddoor_check();
	if (rc != FDSUCCESS) {
		FD_TRACE1("out iocsped", FDA_DEV);
		return(rc);
	}
	fdadapter->state = FD_TYPE2_WAKEUP;
	for (i = 0; i < 3; i++) {
		rc = fdexecute_int_cmd(fdrecalibrate);
		if (rc != FDSUCCESS) {
			fdadapter->state = FD_NO_STATE;
			FD_TRACE1("out iocsped", FDA_DEV);
			return(rc);
		}
		if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL)
		    == 0)
			break;
	}
	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0) {
		fdadapter->state = FD_NO_STATE;
		FD_TRACE1("out iocsped", FDA_DEV);
		return(EIO);
	} /* end of RECALIBRATE processing */
	fdadapter->state = FD_NO_STATE;
	fdadapter->speed_buffer = (char *)xmalloc(PAGESIZE, PGSHIFT,
	    pinned_heap);
	if (fdadapter->speed_buffer == NULL) {
		FD_TRACE1("out iocsped", FDA_DEV);
		return(ENOMEM);
	}
	for (i = 0; i < 20; i++) {
		fdadapter->xdp.aspace_id = XMEM_GLOBAL;
#ifdef _IBMRT
		d_unmask(fdadapter->dma_id);
#endif
		d_slave(fdadapter->dma_id, DMA_READ,
		    (char *)fdadapter->speed_buffer, fdp->bytes_per_sector,
		    &fdadapter->xdp);
		fdadapter->state = FD_SPEED;
		fdadapter->sub_state = FD_SPEED_READ1;
		fdp->current.cylinder = 0;
		fdp->current.head = 0;
		fdp->current.sector = 1;
		rc = fdexecute_int_cmd(fdrw_ioctl);
		if (rc != 0) {
			fdadapter->state = FD_NO_STATE;
			fdadapter->sub_state = NULL;
			if (fdadapter->d_comp_needed == TRUE) {
				trc = d_complete(fdadapter->dma_id, DMA_READ,
				    (char *)fdadapter->speed_buffer,
				    fdp->bytes_per_sector, &fdadapter->xdp,
				    NULL);
				FD_TRACE1("call d_comp", trc);
				ASSERT(trc == 0);
			}
			trc = xmfree((caddr_t)fdadapter->speed_buffer,
			    pinned_heap);
			ASSERT(trc == 0);
			FD_TRACE1("out iocsped", FDA_DEV);
			return(rc);
		}
		if (fdadapter->start_time.tv_sec ==
		    fdadapter->end_time.tv_sec) {
			elapsed_time = fdadapter->end_time.tv_nsec -
			    fdadapter->start_time.tv_nsec;
		} else
		 {
			elapsed_time = NS_PER_SEC -
			    fdadapter->start_time.tv_nsec;
			elapsed_time += (fdadapter->end_time.tv_nsec +
			    (fdadapter->end_time.tv_sec -
			    fdadapter->start_time.tv_sec - 1) * NS_PER_SEC);
		}
		elapsed_time /= 1000; /* convert to microseconds */
		time_sum += elapsed_time;
	}
	fdadapter->state = FD_NO_STATE;
	elapsed_time = time_sum / 20;
	fdp->motor_speed = (uint)(60 * (uS_PER_SEC) / elapsed_time);
	xmfree((caddr_t)fdadapter->speed_buffer, pinned_heap);
	FD_TRACE1("out iocsped", FDA_DEV);
	return(rc);
} /* end of fdiocspeed() */


int	fdopen_exit (struct floppy *fdp)
{
	int	local_error_value, rc;
	uchar	drive_number;

	FD_TRACE1("in op_exit", FDA_DEV);
	drive_number = minor(fdp->device_number) >> 6;
	local_error_value = fdadapter->error_value;
	fddisable_controller();
	rc = unpin((caddr_t)fdp, sizeof(struct floppy));
	ASSERT(rc == 0);
	fdcleanup(drive_number);
	FD_TRACE1("new state", fdp->drive_state);
	fdp->drive_state = FDCLOSED;
	fdtop_unlock_adapter(drive_number);
	DDHKWD1(HKWD_DD_FDDD, DD_EXIT_OPEN, local_error_value,
	    fdp->device_number);
	FD_TRACE1("out op_exit", FDA_DEV);
	return(local_error_value);
}

void	fdmotor_start ()
{
	uchar drive_number;
	struct floppy *fdp;

	FD_TRACE1("in motstrt", FDA_DEV);
	fdadapter->error_value = 0;
	fdreset_check();
	if (fdadapter->error_value != 0) {
		FD_TRACE1("out motstrt", FDA_DEV);
		return;
	}
	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	if (fdadapter->motor_on != drive_number) {
		fdadapter->error_value = 0;
		fdtop_select_drive();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out motstrt", FDP_DEV);
			return;
		}
		fdadapter->error_value = 0;
		fdset_config();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out motstrt", FDP_DEV);
			return;
		}
		fdset_data_rate();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out motstrt", FDP_DEV);
			return;
		}
		fdadapter->error_value = 0;
		fdset_perp();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out motstrt", FDP_DEV);
			return;
		}
		fdadapter->error_value = 0;
		fdspecify();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out motstrt", FDP_DEV);
			return;
		}
		delay(fdp->motor_ticks);
		w_start(&(fdadapter->mottimer));
		fdadapter->motor_on = drive_number;
	}
	FD_TRACE1("out motstrt", FDP_DEV);
}

/*
 * NAME: fdtype
 *
 * FUNCTION: Determines the type of diskette in a drive.
 *  This routine attempts to determine the type of diskette that is
 *  in a diskette driver.  It does this by attempting to read some
 *  sectors using the characteristics of the default diskette type
 *  for the drive.  If this fails, the next action depends on the
 *  minor number passed to the routine.  If the minor number is less
 *  than four, reads will be attempted using the characteristics for
 *  every possible diskette that is valid for the drive until either
 *  one works or all have failed.  If the minor number is four or
 *  greater, only the characteristics indicated by the minor number
 *  are used.
 *
 * EXECUTION ENVIRONMENT:
 *  This routine is part of the top half of the device driver.  It
 *  can be called only at the process level.  Note that the adapter must
 *  be locked before this routine is called.
 *
 * DATA STRUCTURES:
 *  fdadapter struct - Common device driver information.
 *  floppy    struct - Per device information.
 *
 * INPUTS: None
 *
 * RETURN VALUE DESCRIPTION:
 *  FDSUCCESS - the type was successfully determined.
 *
 * ERROR CODES:
 *    EFORMAT   - the diskette was either not formatted, was formatted
 *                incorrectly, or is bad.
 *              - whatever error codes are returned from the hardware
 *                accesses. (either ENOTREADY or EIO)
 *
 * EXTERNAL PROCEDURES CALLED: none.
 */

int	fdtype ()
{
	int	rc, i, j;
	register struct floppy *fdp;
	uchar	drive_number, results;

	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	FD_TRACE1("in type", FDP_DEV);

	/*
	 * Try to read the diskette using the default diskette characteristics
	 * passed in the 'floppy' structure.  If the read fails and
	 * the minor number is not FDGENERIC, set rc to EFORMAT and return.
	 */

	fdadapter->error_value = 0;
	fdmotor_start();
	if (fdadapter->error_value != 0) {
		FD_TRACE1("out type", FDP_DEV);
		return(fdadapter->error_value);
	}
	fdp->head_id = 0;
	fdadapter->state = FD_TYPE1_WAKEUP;
	rc = fdexecute_int_cmd(fdreadid);
	if (rc != FDSUCCESS) {
		fdadapter->state = FD_NO_STATE;
		FD_TRACE1("out type", FDP_DEV);
		return(rc);
	}
	fdadapter->state = FD_NO_STATE;
	results = fdadapter->results.un1.names.byte0.status0;
	if ((results & FDNORMAL) != 0) {
		if (minor(fdp->device_number) & FDTYPEMASK) {
			if (fdp->retry_flag == FALSE) {
				FD_TRACE1("out type", FDP_DEV);
				return(EFORMAT);
			}
			for (i = 0; i < 3; i++) {
				fdadapter->motor_on = FDNO_DRIVE;
				fdadapter->reset_needed = TRUE;
				fdadapter->error_value = 0;
				fdmotor_start();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(fdadapter->error_value);
				}
				fdadapter->state = FD_TYPE2_WAKEUP;
				for (j = 0; j < 3; j++) {
					rc = fdexecute_int_cmd(fdrecalibrate);
					if (rc != FDSUCCESS) {
						fdadapter->state = FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					if ((fdadapter->
					    results.un1.names.byte0.status0 &
					    FDNORMAL) == 0)
						break;
				}
				if ((fdadapter->results.un1.names.byte0.status0
				    & FDNORMAL) != 0) {
					fdadapter->state = FD_NO_STATE;
					FD_TRACE1("out type", FDP_DEV);
					return(EIO);
				}
				fdadapter->state = FD_TYPE1_WAKEUP;
				rc = fdexecute_int_cmd(fdreadid);
				fdadapter->state = FD_NO_STATE;
				if (rc != FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				results = 
				    fdadapter->results.un1.names.byte0.status0;
			}
		}
	}
	if ((results & FDNORMAL) != 0) {
		if (minor(fdp->device_number) & FDTYPEMASK) {
			FD_TRACE1("out type", FDP_DEV);
			return(EFORMAT);
		}

	/*
 	 * Otherwise, start a loop that will attempt reads using all
 	 * the possible characteristics for the drive type.  This loop 
 	 * can be exited under three conditions.  The first is that one
 	 * of the read attempts is successful.  If this is so, leave
 	 * the 'floppy' structure loaded with the current 
 	 * characteristics (the ones that worked) and return FDSUCCESS.
 	 * The second exit condition is that all possible
 	 * characteristics have been tried and have failed.  In this
 	 * case, set rc to EFORMAT and return.  The third condition is 
	 * that one of the hardware accesses failed.  In this last case
	 * the return code is the error value returned from the hardware
	 * access.
 	 */

		switch (fdp->drive_type) {
		case D_96:
			if ((rc = fdload_floppy(FD360_5, fdp)) != FDSUCCESS) {
				FD_TRACE1("out type", FDP_DEV);
				return(rc);
			}
			for (i = 0; i < 3; i++) {
				fdadapter->motor_on = FDNO_DRIVE;
				fdadapter->reset_needed = TRUE;
				fdadapter->error_value = 0;
				fdmotor_start();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(fdadapter->error_value);
				}
				fdadapter->state = FD_TYPE2_WAKEUP;
				for (j = 0; j < 3; j++) {
					rc = fdexecute_int_cmd(fdrecalibrate);
					if (rc != FDSUCCESS) {
						fdadapter->state = FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) == 0)
						break;
				}
				if ((fdadapter->results.un1.names.byte0.status0
				    & FDNORMAL) != 0) {
					fdadapter->state = FD_NO_STATE;
					FD_TRACE1("out type", FDP_DEV);
					return(EIO);
				}
				fdadapter->state = FD_TYPE1_WAKEUP;
				rc = fdexecute_int_cmd(fdreadid);
				fdadapter->state = FD_NO_STATE;
				if (rc != FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				results = fdadapter->results.un1.names.
				    byte0.status0;
				if ((results & FDNORMAL) == 0)
					break;
				else {
					if (fdp->retry_flag == FALSE) {
						FD_TRACE1("out type", FDP_DEV);
						return(EFORMAT);
					}
				}
			}
			if ((results & FDNORMAL) != 0) {
				if ((rc = fdload_floppy(FD1200_5, fdp)) !=
				    FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				for (i = 0; i < 3; i++) {
					fdadapter->motor_on = FDNO_DRIVE;
					fdadapter->reset_needed = TRUE;
					fdadapter->error_value = 0;
					fdmotor_start();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out type", FDP_DEV);
						return(fdadapter->error_value);
					}
					fdadapter->state = FD_TYPE2_WAKEUP;
					for (j = 0; j < 3; j++) {
						rc = fdexecute_int_cmd(
						    fdrecalibrate);
						if (rc != FDSUCCESS) {
							fdadapter->state =
							    FD_NO_STATE;
							FD_TRACE1("out type",
							    FDP_DEV);
							return(rc);
						}
						if ((fdadapter->results.un1.
						    names.byte0.status0 &
						    FDNORMAL) == 0)
							break;
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) != 0) {
						fdadapter->state =
						    FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(EIO);
					}
					fdadapter->state = FD_TYPE1_WAKEUP;
					rc = fdexecute_int_cmd(fdreadid);
					fdadapter->state = FD_NO_STATE;
					if (rc != FDSUCCESS) {
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					results = fdadapter->results.un1.
					    names.byte0.status0;
					if ((results & FDNORMAL) == 0)
						break;
				}
				if ((results & FDNORMAL) != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(EFORMAT);
				} else
				 {
					fdp->diskette_type = FD80CYLS |
					    FDDOUBLE | FD15PRTRCK;
				}
			} else
			 {
				fdp->diskette_type = FD40CYLS | FDDOUBLE |
				    FD9PRTRCK;
			}
			break;
		case D_135H:
			if ((rc = fdload_floppy(FD720_3, fdp)) != FDSUCCESS) {
				FD_TRACE1("out type", FDP_DEV);
				return(rc);
			}
			for (i = 0; i < 3; i++) {
				fdadapter->motor_on = FDNO_DRIVE;
				fdadapter->reset_needed = TRUE;
				fdadapter->error_value = 0;
				fdmotor_start();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(fdadapter->error_value);
				}
				fdadapter->state = FD_TYPE2_WAKEUP;
				for (j = 0; j < 3; j++) {
					rc = fdexecute_int_cmd(fdrecalibrate);
					if (rc != FDSUCCESS) {
						fdadapter->state = FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) == 0)
						break;
				}
				if ((fdadapter->results.un1.names.byte0.status0
				    & FDNORMAL) != 0) {
					fdadapter->state = FD_NO_STATE;
					FD_TRACE1("out type", FDP_DEV);
					return(EIO);
				}
				fdadapter->state = FD_TYPE1_WAKEUP;
				rc = fdexecute_int_cmd(fdreadid);
				fdadapter->state = FD_NO_STATE;
				if (rc != FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				results = fdadapter->results.un1.names.
				    byte0.status0;
				if ((results & FDNORMAL) == 0)
					break;
				else {
					if (fdp->retry_flag == FALSE) {
						FD_TRACE1("out type", FDP_DEV);
						return(EFORMAT);
					}
				}
			}
			if ((results & FDNORMAL) != 0) {
				if ((rc = fdload_floppy(FD1440_3, fdp)) !=
				    FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				for (i = 0; i < 3; i++) {
					fdadapter->motor_on = FDNO_DRIVE;
					fdadapter->reset_needed = TRUE;
					fdadapter->error_value = 0;
					fdmotor_start();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out type", FDP_DEV);
						return(fdadapter->error_value);
					}
					fdadapter->state = FD_TYPE2_WAKEUP;
					for (j = 0; j < 3; j++) {
						rc = fdexecute_int_cmd(
						    fdrecalibrate);
						if (rc != FDSUCCESS) {
							fdadapter->state =
							    FD_NO_STATE;
							FD_TRACE1("out type",
							    FDP_DEV);
							return(rc);
						}
						if ((fdadapter->results.un1.
						    names.byte0.status0 &
						    FDNORMAL) == 0)
							break;
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) != 0) {
						fdadapter->state =
						    FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(EIO);
					}
					fdadapter->state = FD_TYPE1_WAKEUP;
					rc = fdexecute_int_cmd(fdreadid);
					fdadapter->state = FD_NO_STATE;
					if (rc != FDSUCCESS) {
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					results = fdadapter->results.un1.
					    names.byte0.status0;
					if ((results & FDNORMAL) == 0)
						break;
				}
				if ((results & FDNORMAL) != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(EFORMAT);
				} else
				 {
					fdp->diskette_type = FD80CYLS |
					    FDDOUBLE | FD18PRTRCK;
				}
			} else
			 {
				fdp->diskette_type = FD80CYLS | FDDOUBLE |
				    FD9PRTRCK;
			}
			break;
		case D_1354H:
			if ((rc = fdload_floppy(FD720_3, fdp)) != FDSUCCESS) {
				FD_TRACE1("out type", FDP_DEV);
				return(rc);
			}
			for (i = 0; i < 3; i++) {
				fdadapter->motor_on = FDNO_DRIVE;
				fdadapter->reset_needed = TRUE;
				fdadapter->error_value = 0;
				fdmotor_start();
				if (fdadapter->error_value != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(fdadapter->error_value);
				}
				fdadapter->state = FD_TYPE2_WAKEUP;
				for (j = 0; j < 3; j++) {
					rc = fdexecute_int_cmd(fdrecalibrate);
					if (rc != FDSUCCESS) {
						fdadapter->state = FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) == 0)
						break;
				}
				if ((fdadapter->results.un1.names.byte0.status0
				    & FDNORMAL) != 0) {
					fdadapter->state = FD_NO_STATE;
					FD_TRACE1("out type", FDP_DEV);
					return(EIO);
				}
				fdadapter->state = FD_TYPE1_WAKEUP;
				rc = fdexecute_int_cmd(fdreadid);
				fdadapter->state = FD_NO_STATE;
				if (rc != FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				results = fdadapter->results.un1.names.
				    byte0.status0;
				if ((results & FDNORMAL) == 0)
					break;
				else {
					if (fdp->retry_flag == FALSE) {
						FD_TRACE1("out type", FDP_DEV);
						return(EFORMAT);
					}
				}
			}
			if ((results & FDNORMAL) != 0) {
			    if ((rc = fdload_floppy(FD1440_3, fdp)) != FDSUCCESS) {
				FD_TRACE1("out type", FDP_DEV);
				return(rc);
			   }
			   for (i = 0; i < 3; i++) {
				   fdadapter->motor_on = FDNO_DRIVE;
				   fdadapter->reset_needed = TRUE;
				   fdadapter->error_value = 0;
				   fdmotor_start();
				   if (fdadapter->error_value != 0) {
					   FD_TRACE1("out type", FDP_DEV);
					   return(fdadapter->error_value);
				   }
				   fdadapter->state = FD_TYPE2_WAKEUP;
				   for (j = 0; j < 3; j++) {
					   rc = fdexecute_int_cmd(fdrecalibrate);
					   if (rc != FDSUCCESS) {
						fdadapter->state = FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					   }
					   if ((fdadapter->results.un1.names.
					      byte0.status0 & FDNORMAL) == 0)
						   break;
				   }
				   if ((fdadapter->results.un1.names.byte0.status0
				    & FDNORMAL) != 0) {
					   fdadapter->state = FD_NO_STATE;
					   FD_TRACE1("out type", FDP_DEV);
					   return(EIO);
				   }
				   fdadapter->state = FD_TYPE1_WAKEUP;
				   rc = fdexecute_int_cmd(fdreadid);
				   fdadapter->state = FD_NO_STATE;
				   if (rc != FDSUCCESS) {
					   FD_TRACE1("out type", FDP_DEV);
					   return(rc);
				   }
				   results = fdadapter->results.un1.names.
				     byte0.status0;
				   if ((results & FDNORMAL) == 0)
					   break;
				   else {
					   if (fdp->retry_flag == FALSE) {
						   FD_TRACE1("out type", FDP_DEV);
						   return(EFORMAT);
					   }
				   }
	   	           }
			   if ((results & FDNORMAL) != 0) {
                                if ((rc = fdload_floppy(FD2880_3, fdp)) != 
				    FDSUCCESS) {
					FD_TRACE1("out type", FDP_DEV);
					return(rc);
				}
				for (i = 0; i < 3; i++) {
					fdadapter->motor_on = FDNO_DRIVE;
					fdadapter->reset_needed = TRUE;
					fdadapter->error_value = 0;
					fdmotor_start();
					if (fdadapter->error_value != 0) {
						FD_TRACE1("out type", FDP_DEV);
						return(fdadapter->error_value);
					}
					fdadapter->state = FD_TYPE2_WAKEUP;
					for (j = 0; j < 3; j++) {
						rc = fdexecute_int_cmd(
						    fdrecalibrate);
						if (rc != FDSUCCESS) {
							fdadapter->state =
							    FD_NO_STATE;
							FD_TRACE1("out type",
							    FDP_DEV);
							return(rc);
						}
						if ((fdadapter->results.un1.
						    names.byte0.status0 &
						    FDNORMAL) == 0)
							break;
					}
					if ((fdadapter->results.un1.names.
					    byte0.status0 & FDNORMAL) != 0) {
						fdadapter->state =
						    FD_NO_STATE;
						FD_TRACE1("out type", FDP_DEV);
						return(EIO);
					}
					fdadapter->state = FD_TYPE1_WAKEUP;
					rc = fdexecute_int_cmd(fdreadid);
					fdadapter->state = FD_NO_STATE;
					if (rc != FDSUCCESS) {
						FD_TRACE1("out type", FDP_DEV);
						return(rc);
					}
					results = fdadapter->results.un1.
					    names.byte0.status0;
					if ((results & FDNORMAL) == 0)
						break;
				}
				if ((results & FDNORMAL) != 0) {
					FD_TRACE1("out type", FDP_DEV);
					return(EFORMAT);
				} else
				 {
					fdp->diskette_type = FD80CYLS |
					    FDDOUBLE | FD36PRTRCK;
				}
			    } else
			     {
				fdp->diskette_type = FD80CYLS | FDDOUBLE |
						     FD18PRTRCK;
			    }
			} else
			 {
			    fdp->diskette_type = FD80CYLS | FDDOUBLE |
						 FD9PRTRCK;
			}
			break;
		 default:
			rc = EINVAL;
			break;
		} /* end of switch on drive type */
	} else /* end of bad initial readid processing */
	 { /* start of good initial readid processing */
		switch (minor(fdp->device_number) & FDTYPEMASK) {
		case FDGENERIC:
			switch (fdp->drive_type) {
			case D_96:
				fdp->diskette_type = FD80CYLS | FDDOUBLE |
				    FD15PRTRCK;
				break;
			case D_135H:
				fdp->diskette_type = FD80CYLS | FDDOUBLE |
				    FD18PRTRCK;
				break;
			case D_1354H:
				fdp->diskette_type = FD80CYLS | FDDOUBLE |
				    FD36PRTRCK;
				break;
			default:
				rc = EINVAL;
				break;
			} /* end of switch on drive type */
			break;
		case FD2880_3:
			fdp->diskette_type = FD80CYLS | FDDOUBLE | FD36PRTRCK;
			break;
		case FD1440_3:
			fdp->diskette_type = FD80CYLS | FDDOUBLE | FD18PRTRCK;
			break;
		case FD720_3:
			fdp->diskette_type = FD80CYLS | FDDOUBLE | FD9PRTRCK;
			break;
		case FD1200_5:
			fdp->diskette_type = FD80CYLS | FDDOUBLE | FD15PRTRCK;
			break;
		case FD360_5:
			fdp->diskette_type = FD40CYLS | FDDOUBLE | FD9PRTRCK;
			break;
		default:
			rc = EINVAL;
			break;
		} /* end of switch on minor number */
	} /* end of good initial readid processing */
	FD_TRACE1("out type", FDP_DEV);
	return(rc);
} /* end of fdtype() */



void fdreset_check ()
{
	int	rc;

	FD_TRACE1("in rsetchk", FDA_DEV);
	if (fdadapter->reset_needed == TRUE) {
		fdadapter->state = FD_TYPE2_WAKEUP;
		rc = fdexecute_int_cmd(fdreset);
		fdadapter->state = FD_NO_STATE;
		fdadapter->error_value = rc;
	}
	FD_TRACE1("out rsetchk", FDA_DEV);
}



int	fdiocstatus (register struct fd_status *fdstatusp)
{
	uchar	drive_number, temp;
	struct floppy *fdp;

	drive_number = fdadapter->active_drive;
	fdp = fdadapter->drive_list[drive_number];
	FD_TRACE1("in iocstat", FDP_DEV);
	fdadapter->error_value = 0;
	fdstatusp->mainstat = fdread_main_status();
	if (fdadapter->error_value != 0) {
		FD_TRACE1("out iocstat", FDP_DEV);
		return(fdadapter->error_value);
	}
	if (fdadapter->motor_on == FDNO_DRIVE) {
		fdstatusp->status1 = FDNODRIVE;
		fdstatusp->status2 = 0;
		fdstatusp->status3 = 0;
		fdstatusp->command0 = 0;
		fdstatusp->command1 = 0;
		fdadapter->error_value = 0;
		fdstatusp->dsktchng = fdread_change_line();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out iocstat", FDP_DEV);
			return(fdadapter->error_value);
		}
		fdstatusp->result0 = 0;
		fdstatusp->result1 = 0;
		fdstatusp->result2 = 0;
		fdstatusp->result3 = 0;
		fdstatusp->cylinder_num = 0;
		fdstatusp->head_num = 0;
		fdstatusp->sector_num = 0;
		fdstatusp->bytes_num = 0;
		fdstatusp->head_settle_time = 0;
		fdstatusp->motor_speed = 0;
		fdstatusp->Mbytes_read = 0;
		fdstatusp->Mbytes_written = 0;
	} else
	 {
		fdadapter->error_value = 0;
		fdtop_select_drive();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out iocstat", FDP_DEV);
			return(fdadapter->error_value);
		}
		if (fdadapter->active_drive == 0) 
			temp = FDDRIVE0;
		else 
			temp = FDDRIVE1;
		switch (fdadapter->data_rate) {
		case 0x00:
			temp |= FD500KBPS; /* 5.25 inch */
			break;
		case 0x01:
			temp |= FD300KBPS; /* 5.25 inch */
			break;
		case 0x02:
			temp |= FD250KBPS; /* 3.5 inch */
			break;
		case 0x03:
			temp |= FD1MBPS; /* 3.5 4M inch */
			break;
		case 0x04:
			temp |= FD500KBPS; /* 3.5 inch */
			break;
		case 0x06:
			temp |= FD250KBPS; /* 3.5 inch */
			break;
		case 0xff: /* data rate has not been set yet */
			break;
		default:
			ASSERT((fdadapter->data_rate) != 
			    (fdadapter->data_rate));
			break;
		}
		fdstatusp->status1 = (uchar)temp;
		if (fdp->retry_flag) 
			temp = FDRETRY;
		else 
			temp = 0;
		if (fdadapter->error_value == ENOTREADY) 
			temp |= FDTIMEOUT;
		switch (fdp->drive_type) {
		case D_96:
			temp |= FD5INCHHIGH;
			break;
		case D_135H:
			temp |= FD3INCHHIGH;
			break;
		case D_1354H:
			temp |= FD3INCHHIGH4M;
			break;
		default:
			ASSERT(fdp->drive_type != fdp->drive_type);
			break;
		}
		fdstatusp->status2 = (uchar)temp;
		fdstatusp->status3 = fdp->diskette_type;
		fdstatusp->command0 = fdadapter->command.un1.cmds.command1;
		fdstatusp->command1 = fdadapter->command.un1.cmds.command2;
		fdadapter->error_value = 0;
		fdstatusp->dsktchng = fdread_change_line();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out iocstat", FDP_DEV);
			return(fdadapter->error_value);
		}
		fdstatusp->result0 = fdadapter->results.un1.result_array[0];
		fdstatusp->result1 = fdadapter->results.un1.result_array[1];
		fdstatusp->result2 = fdadapter->results.un1.result_array[2];
		fdstatusp->cylinder_num =
		    fdadapter->results.un1.result_array[3];
		fdstatusp->head_num = fdadapter->results.un1.result_array[4];
		fdstatusp->sector_num = fdadapter->results.un1.result_array[5];
		fdstatusp->bytes_num = fdadapter->results.un1.result_array[6];
		fdstatusp->head_settle_time = fdp->head_settle_time;
		fdstatusp->motor_speed = fdp->motor_speed;
		fdstatusp->Mbytes_read = fdp->rcount_megabytes;
		fdstatusp->Mbytes_written = fdp->wcount_megabytes;
		fdadapter->error_value = 0;
		fdsense_drive_status();
		if (fdadapter->error_value != 0) {
			FD_TRACE1("out iocstat", FDP_DEV);
			return(fdadapter->error_value);
		}
		fdstatusp->result3 =
		    fdadapter->results.un1.names.byte0.status3;
	}
	FD_TRACE1("out iocstat", FDP_DEV);
	return(FDSUCCESS);
} /* end of fdiocstatus() */



int	fdconfig_vpd (register struct floppy *fdp, 
		      register union fd_config *vpdptr, uchar drive_number)
{
	int	rc, trc, i, j;
	struct	pio_parms piop;
	uchar   mask;

	FD_TRACE1("in cfgvpd", FDP_DEV);
	piop.bus_val = BUSIO_ATT(fdadapter->bus_id, fdadapter->io_address);

	DEBUG_0 ("cfgvpd: fdconfig_reset\n");
	rc = fdconfig_reset(&piop);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}
	delay(HZ/10); /* delay for 1/10 of a second to let controller settle */

	DEBUG_0 ("cfgvpd: fdconfig_sense_interrupt\n");
	rc = fdconfig_sense_interrupt(fdp, &piop);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}

	DEBUG_1 ("cfgvpd: fdconfig_sense_interrupt  status = %x\n",
		fdadapter->results.un1.names.byte0.status0);
	if ((fdadapter->results.un1.names.byte0.status0 &
	    FDNORMAL) != FDNORMAL) {
		fdadapter->reset_needed = TRUE;
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}

	DEBUG_0 ("cfgvpd: fdconfig_specify\n");
	rc = fdconfig_specify(fdp, &piop);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}

	DEBUG_0 ("cfgvpd: fdconfig_motor_start\n");
	rc = fdconfig_motor_start(fdp, &piop, (uchar)drive_number);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}
	/* recal (may need as many as three)*/
	for (i = 0; i < 3; i++) {
		DEBUG_1 ("cfgvpd: fdconfig_recalibrate %d\n", i);
		rc = fdconfig_recalibrate(fdp, &piop, (uchar)drive_number);
		if (rc == FDSUCCESS) {
			if (i == 0)

				/* 
				 * delay 1.5 seconds the first time to give
				 * drive time to recalibrate all the way 
				 * from track 80.
				 */

				delay((3 * HZ) / 2);
			else
				delay(HZ / 10);

			DEBUG_1 ("cfgvpd: fdconfig_sense_interrupt lp %d\n", i);
			rc = fdconfig_sense_interrupt(fdp, &piop);
			if (rc != FDSUCCESS) {
				fdadapter->reset_needed = TRUE;
				fdconfig_motor_stop(&piop);
				BUSIO_DET(piop.bus_val);
				FD_TRACE1("out cfgvpd", FDP_DEV);
				return(rc);
			}

			DEBUG_1 ("cfgvpd: sense_interrupt lp status = %x\n",
				fdadapter->results.un1.names.byte0.status0);
			if ((fdadapter->results.un1.names.byte0.status0 &
			    FDNORMAL) == 0) 
				break;
			if (i == 2) {
				DEBUG_1 ("cfgvpd: fdconfig_motor_stop %d\n", i);
				fdconfig_motor_stop(&piop);
				BUSIO_DET(piop.bus_val);

				/*
				 * Do not return error and do not mark device
				 * as being present.
				 */
				
				FD_TRACE1("out cfgvpd", FDP_DEV);
				return(FDSUCCESS);
			}
		} else
		 {
			DEBUG_0 ("cfgvpd: fdconfig_motor_stop");
			DEBUG_0 (" unsuccessful calibrate\n");
			fdadapter->reset_needed = TRUE;
			fdconfig_motor_stop(&piop);
			BUSIO_DET(piop.bus_val);
			FD_TRACE1("out cfgvpd", FDP_DEV);
			return(rc);
		}
	}

	/* seek to 1 */
	DEBUG_0 ("cfgvpd: fdconfig_seek\n");
	rc = fdconfig_seek(fdp, &piop, drive_number);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}
	delay(HZ / 10);

	DEBUG_0 ("cfgvpd: fdconfig_sense_interrupt\n");
	rc = fdconfig_sense_interrupt(fdp, &piop);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}

	DEBUG_1 ("cfgvpd: fdconfig_sense_interrupt 3  status = %x\n",
		fdadapter->results.un1.names.byte0.status0);
	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0) {
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(FDSUCCESS);
	}
	if (fdadapter->results.un1.names.byte1.present_track != 1) {
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(FDSUCCESS);
	}

	DEBUG_0 ("cfgvpd: fdconfig_recalibrate\n");
	rc = fdconfig_recalibrate(fdp, &piop, drive_number);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}
	delay(HZ / 10);

	DEBUG_0 ("cfgvpd: fdconfig_sense_interrupt 4\n");
	rc = fdconfig_sense_interrupt(fdp, &piop);
	if (rc != FDSUCCESS) {
		fdadapter->reset_needed = TRUE;
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(rc);
	}

	DEBUG_1 ("cfgvpd: fdconfig_sense_interrupt 4  status = %x\n",
		fdadapter->results.un1.names.byte0.status0);
	if ((fdadapter->results.un1.names.byte0.status0 & FDNORMAL) != 0)  {
		fdconfig_motor_stop(&piop);
		BUSIO_DET(piop.bus_val);
		FD_TRACE1("out cfgvpd", FDP_DEV);
		return(FDSUCCESS);
	}

	/*
	 * Only mark as present if no errors are encountered.
	 */

	/* check if the drive supports 4 MB diskettes by checking */
	/* for SIO type fef6 (FD_ADAPTER1)			  */ 
	/* the "define_children" part of the SIO configuration    */
	/* method will determine the correct diskette adapter	  */
	if (fdadapter->adapter_type == FD_ADAPTER1) {
		rc = fdconfig_read_threshold(&piop);
		if ((rc == FDSUCCESS) && (piop.data & 0x40)) {
			vpdptr->drive_info[drive_number] |= D_1354H;
		}
	}

	/* mark that the drive exists on the system */
	vpdptr->drive_info[drive_number] |= FD_PRESENT;

	fdadapter->motor_on = FDNO_DRIVE;
	rc = fdconfig_motor_stop(&piop);
	BUSIO_DET(piop.bus_val);
	FD_TRACE1("out cfgvpd", FDP_DEV);
	return(rc);
}


int	fdconfig_reset ( register struct pio_parms *piop )
{
	caddr_t iocc_val;
	uchar	temp_val;
	struct	timestruc_t start_time, end_time;
	ulong	elapsed_time = 0;
	int	rc;

	FD_TRACE1("in cfgrset", FDA_DEV);
	if (fdadapter->adapter_type != FD_ADAPTER2) {
		iocc_val = IOCC_ATT(fdadapter->bus_id, 0);
		temp_val = BUSIO_GETC((caddr_t)((ulong)iocc_val + FDENABLEREG +
		    (fdadapter->slot_num << 16)));
		curtime(&start_time);
		FD_TRACE2("W:", temp_val | 0x08, (caddr_t)((ulong)iocc_val +
		    FDENABLEREG + (fdadapter->slot_num << 16)), 0);
		BUSIO_PUTC((caddr_t)((ulong)iocc_val + FDENABLEREG +
		    (fdadapter->slot_num << 16)), temp_val | 0x08);
		while (elapsed_time < 8000) { /* 8000 nanoseconds for R2 */
			curtime(&end_time);
			if (start_time.tv_sec == end_time.tv_sec) {
				elapsed_time = end_time.tv_nsec - start_time.tv_nsec;
			} else
			 {
				elapsed_time = NS_PER_SEC - start_time.tv_nsec;
				elapsed_time += (end_time.tv_nsec + (end_time.tv_sec -
				    start_time.tv_sec - 1) * NS_PER_SEC);
			}
		}
		FD_TRACE2("W:", temp_val & 0xf7, (caddr_t)((ulong)iocc_val +
		    FDENABLEREG + (fdadapter->slot_num << 16)), 0);
		BUSIO_PUTC((caddr_t)((ulong)iocc_val + FDENABLEREG +
		    (fdadapter->slot_num << 16)), temp_val & 0xf7);
		IOCC_DET(iocc_val);
        }
	rc = fdconfig_soft_reset(piop);
	FD_TRACE1("out cfgrset", FDA_DEV);
	return(rc);
}


int	fdconfig_soft_reset ( register struct pio_parms *piop )
{
	uchar	drive_number, mask;
	struct	timestruc_t start_time, end_time;
	ulong	elapsed_time = 0;
	int	rc;

	FD_TRACE1("in cfgsrst", FDA_DEV);
	drive_number = fdadapter->active_drive;
	ASSERT(drive_number < FDMAXDRIVES);
	mask = FDNOINTRESETMASK;
	piop->reg = FDDRIVE_CONTROL_REG;
	piop->read = FALSE;
	piop->data = mask;
	rc = pio_assist((caddr_t)(piop), fdcfg_cntl_pio,
	    fdcfg_cntl_pio_recovery);
	if (rc == 0) {
		curtime(&start_time);
		while (elapsed_time < 4000) { /* 4000 nanoseconds for R2 */
			curtime(&end_time);
			if (start_time.tv_sec == end_time.tv_sec) {
				elapsed_time = end_time.tv_nsec - 
				    start_time.tv_nsec;
			} else
		 	 {
				elapsed_time = NS_PER_SEC - start_time.tv_nsec;
				elapsed_time += (end_time.tv_nsec + 
				    (end_time.tv_sec - start_time.tv_sec - 1)
				    * NS_PER_SEC);
			}
		}
		mask |= 0x04;
		piop->data = mask;
		rc = pio_assist((caddr_t)(piop), fdcfg_cntl_pio,
		    fdcfg_cntl_pio_recovery);
		if (rc == 0) {
			fdadapter->reset_needed = FALSE;
		}
	}
	FD_TRACE1("out cfgsrst", FDA_DEV);
	return(rc);
}




int	fdconfig_specify ( register struct floppy *fdp, 
	    register struct pio_parms *piop )
{
	int	rc;

	fdadapter->command.total_bytes = 3;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDSPECIFY;
	fdadapter->command.un1.cmds.command2 = fdp->head_unload |
	    (fdp->step_rate << 4);
	fdadapter->command.un1.cmds.byte2.motor_time = fdp->head_load << 1;
	rc = pio_assist((caddr_t)(piop), fdcfg_data_pio,
	    fdcfg_data_pio_recovery);
	return(rc);
}




int	fdconfig_motor_start ( register struct floppy *fdp, 
	    register struct pio_parms *piop, uchar drive_number )
{
	int	rc;
	uchar	mask;

	mask = FDDRIVE_0_MASK;
	mask = mask << drive_number;
	mask |= drive_number;
	mask |= FDDISABLEMASK;
	piop->reg = FDDRIVE_CONTROL_REG;
	piop->read = FALSE;
	piop->data = mask;
	rc = pio_assist((caddr_t)(piop), fdcfg_cntl_pio,
	    fdcfg_cntl_pio_recovery);
	fdadapter->motor_on = drive_number;
	return(rc);
}




int	fdconfig_motor_stop ( register struct pio_parms *piop )
{
	int	rc;

	piop->reg = FDDRIVE_CONTROL_REG;
	piop->read = FALSE;
	if (fdadapter->adapter_type != FD_ADAPTER0) {
		piop->data = FDDISABLE2;
	} else
	 {
		piop->data = FDDISABLEMASK;
	}
	rc = pio_assist((caddr_t)(piop), fdcfg_cntl_pio,
	    fdcfg_cntl_pio_recovery);
	fdadapter->motor_on = FDNO_DRIVE;
	return(rc);
}

  /* THIS SECTION WILL READ THE FIFO THRESHOLD REG. 68 */
int     fdconfig_read_threshold ( register struct pio_parms *piop )
{
	int     rc;

	piop->reg = FDTHRES_FIFO_REG;
	piop->read = TRUE;
	piop->data = 0;
	rc = pio_assist((caddr_t)(piop), fdcfg_cntl_pio,
	    fdcfg_cntl_pio_recovery);
	return(rc);
}



int	fdconfig_recalibrate ( register struct floppy *fdp, 
	    register struct pio_parms *piop, uchar drive_number )
{
	int	rc;

	fdadapter->command.total_bytes = 2;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDRECALIBRATE;
	fdadapter->command.un1.cmds.command2 = drive_number;
	rc = pio_assist((caddr_t)(piop), fdcfg_data_pio,
	    fdcfg_data_pio_recovery);
	return(rc);
}




int	fdconfig_sense_interrupt ( register struct floppy *fdp, 
	    register struct pio_parms *piop )
{
	int	rc;

	fdadapter->command.total_bytes = 1;
	fdadapter->results.total_bytes = 2;
	fdadapter->command.un1.cmds.command1 = FDSENSE_INTERRUPT;
	rc = pio_assist((caddr_t)(piop), fdcfg_data_pio,
	    fdcfg_data_pio_recovery);
	return(rc);
}




int	fdconfig_seek ( register struct floppy *fdp, 
	    register struct pio_parms *piop, uchar drive_number )
{
	int	rc;

	fdadapter->command.total_bytes = 3;
	fdadapter->results.total_bytes = 0;
	fdadapter->command.un1.cmds.command1 = FDSEEK;
	fdadapter->command.un1.cmds.command2 = drive_number;
	fdadapter->command.un1.cmds.byte2.track = 1;
	rc = pio_assist((caddr_t)(piop), fdcfg_data_pio,
	    fdcfg_data_pio_recovery);
	return(rc);
}



void	fd_qvpd_exit ( register union fd_config *vpdptr,
	   register struct iovec *localuio_iovec)
{
	int	i, rc;
	register struct floppy *fdp;
	
	for (i = 0; i < FDMAXDRIVES; i++) {
		fdp = fdadapter->drive_list[i];
		if (fdp != NULL) {
			if (!(vpdptr->drive_info[i] & FD_CONFIGURED)) {
#ifdef _POWER_MP
				lock_free(&fdp->intr_lock);
#endif /* _POWER_MP */
				rc = xmfree((caddr_t)fdp, pinned_heap);
				ASSERT(rc == 0);
				fdadapter->drive_list[i] = NULL;
			}
		}
	}
	if (fdadapter->initialized == FALSE) {
    		rc = xmfree((caddr_t)fdadapter->fderrptr, pinned_heap);
		ASSERT(rc == 0);
		rc = unpin((caddr_t)fdadapter,
		    sizeof(struct adapter_structure ));
		ASSERT(rc == 0);
		rc = unpincode(fd_intr);
		ASSERT(rc == 0);
	}
	rc = xmfree((caddr_t)localuio_iovec, pinned_heap);
	ASSERT(rc == 0);
	return;
}
