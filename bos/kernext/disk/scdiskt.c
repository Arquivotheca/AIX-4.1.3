static char sccsid[] = "@(#)44  1.10.4.8  src/bos/kernext/disk/scdiskt.c, sysxdisk, bos41J, 9521A_all 5/23/95 17:43:19";
/*
 * COMPONENT_NAME: (SYSXDISK) SCSI Disk Device Driver Top Half Routines
 *
 * FUNCTIONS:  scdisk_config
 *             scdisk_open
 *             scdisk_close
 *             scdisk_ioctl
 *             scdisk_rdwrse
 *	       scdisk_dkiocmd
 *	       scdisk_dk_amr_pmr
 *	       scdisk_dkeject
 *	       scdisk_format
 *	       scdisk_dkaudio
 *	       scdisk_dk_cd_mode
 *             scdisk_read
 *             scdisk_write
 *	       scdisk_rdwr
 *             scdisk_mincnt
 *	       scdisk_raw
 *	       scdisk_raw_buffer
 *	       scdisk_io_buffer
 *	       scdisk_raw_io
 *	       scdisk_build_raw_cmd
 *	       scdisk_alloc
 *	       scdisk_init_cmds
 *	       scdisk_release_allow
 *	       scdisk_ioctl_mode_sense
 *	       scdisk_audio_msf
 *	       scdisk_audio_trk_indx
 *	       scdisk_pause_resume
 *	       scdisk_read_subchnl
 *	       scdisk_read_toc
 *	       scdisk_pin_buffer
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988,1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Included System Files: */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syspest.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/pin.h>
#include <sys/sleep.h>
#include <sys/ioctl.h>
#include <sys/intr.h>
#include <sys/i_machine.h>
#include <sys/systm.h>
#include <sys/m_param.h>
#include <sys/malloc.h>
#include <sys/conf.h>
#include <sys/devinfo.h>
#include <sys/lockl.h>
#include <sys/uio.h>
#include <sys/device.h>
#include <sys/watchdog.h>
#include <sys/lvdd.h>
#include <sys/errids.h>
#include <sys/dump.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h>
#include <sys/priv.h>
#include <sys/iostat.h>
#include <sys/pm.h>

#ifdef _POWER_MP
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#endif

#include <sys/scdisk.h>
#include <sys/scdiskdd.h>



/* End of Included System Files */

/************************************************************************/
/*                                                                      */
/*                      Static Structures                               */
/*                                                                      */
/*      The scdisk_list data structure is an array of pointers to       */
/*      the first diskinfo structure of a NULL terminated linked        */
/*      list of diskinfo structures describing each of the disk         */
/*      device's currently configured in the system.                    */
/*                                                                      */
/*      The scdisk_open_list data structure is similar to the disk_list */
/*      the only exception being that it contains only those disk       */
/*      devices currently open on the system.                           */
/*                                                                      */
/*      The scdisk_info data structure is a structure used for storing  */
/*      general disk driver information, including global states,       */
/*      configuration counts, global locks, etc.                        */
/*                                                                      */
/*      The diskinfo data structures are allcoated on a one per device  */
/*      basis and are used for storing device specific information.     */
/*      These structures are dynamically allocated when a device is     */
/*      configured into the system, and deallocated when the device     */
/*      is deleted. These structures are linked into the disk_list      */
/*      upon configuration and linked into the disk_open_list during    */
/*      any outstanding opens.                                          */
/*                                                                      */
/************************************************************************/

extern struct scdisk_info      scdisk_info;
extern struct scdisk_diskinfo  *scdisk_list[DK_HASHSIZE];
extern struct scdisk_diskinfo  *scdisk_open_list[DK_HASHSIZE];
extern int			scdisk_threshold;
extern int			scdisk_mode_data_offset;

#ifdef DEBUG
extern int                     scdisk_debug;
extern	uint		       *scdisk_trace;

/*
 ******  strings for Internal Debug Trace Table *******
 * To use compile with the flags SC_GOOD_PATH and SC_ERROR_PATH
 * the variable scdisk_trace will be the beginning of the trace table in
 * memory.  The variable scdisk_trctop will point to location where the
 * next entry in the trace table will go.
 */


char     *opentrc	= "OPEN	   ";
char	 *closetrc	= "CLOSE   ";
char	 *read		= "READ    ";
char	 *write		= "WRITE   ";
char	 *rdwr		= "RDWR    ";
char	 *mincnt	= "MINCNT  ";
char 	 *raw		= "RAW	   ";
char	 *io_buf	= "IO_BUFF ";
char 	 *raw_buff	= "RAW_BUFF";
char 	 *raw_io	= "RAW_IO  ";
char 	 *raw_bld	= "RAW_BLD ";
char	 *ioctls	= "IOCTL   ";
char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */

#endif


/************************************************************************/
/*									*/
/*	NAME:	scdisk_config						*/
/*									*/
/*	FUNCTION: SCSI disk device driver configuration routine.	*/
/*		During device INIT, this routine allocates and		*/
/*		initializes data structures required for processing	*/
/*		user requests. This routine will also delete an already	*/
/*		defined device and free these structures when called	*/
/*		for the TERM operation.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		scdiskinfo	General disk driver information		*/
/*		scdisk_list	List of configured devices		*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		op	- what function to perform (INIT or TERM)	*/
/*		uiop	- uio pointer to configuration data		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO     - Minor number out of range			*/
/*			- Attempt to INIT an existing device		*/
/*			- Attempt to TERM a nonexistent device		*/
/*			- Attempt to TERM an open device		*/
/*		EINVAL  - Unknown operation				*/
/*		EBUSY   - Attempt to TERM an open device		*/
/*		EEXIST  - Devswadd failed due to the fact the device	*/
/*			  is already in the devsw table			*/
/*		ENOMEM	- Xmalloc failed				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		pincode		unpincode				*/
/*		devswadd	devswdel				*/
/*		xmalloc		xmfree					*/
/*		uiomove		pm_register_handle			*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_config(
dev_t		devno,
int		op,
struct uio	*uiop)
{
	int			errnoval, dev, rc, hash_key, idlun;
	uint			cdt_size;
	struct scdisk_diskinfo	*last_diskinfo, *diskinfo;
	struct disk_ddi		config_info;
	struct devsw		devsw_struct;
	struct cdt		*old_cdt;
	int     		min;
	extern			nodev();

#ifdef DEBUG
	DKprintf(("Entering scdisk_config devno[0x%x] op[0x%x]\n", devno, op));
#endif

	DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_CONFIG, 0, devno, op, uiop, 0, 0);
	errnoval = 0;
	dev = minor(devno);


	(void) lockl(&(scdisk_info.lock), LOCK_SHORT);

	switch (op) {
	case CFG_INIT:
		scdisk_info.config_cnt++;
		if (scdisk_info.config_cnt == 0x01) {

			/* Initialize devsw struct for scdisk */
			devsw_struct.d_open = (int(*)())scdisk_open;
			devsw_struct.d_close = (int(*)())scdisk_close;
			devsw_struct.d_read = (int(*)())scdisk_read;
			devsw_struct.d_write = (int(*)())scdisk_write;
			devsw_struct.d_ioctl = (int(*)())scdisk_ioctl;
			devsw_struct.d_strategy=(int(*)())scdisk_strategy;
			devsw_struct.d_ttys = (struct tty *)NULL;
			devsw_struct.d_select = (int(*)())nodev;
			devsw_struct.d_config = (int(*)())scdisk_config;
			devsw_struct.d_print = (int(*)())nodev;
			devsw_struct.d_dump = (int(*)())scdisk_dump;
			devsw_struct.d_mpx = (int(*)())nodev;
			devsw_struct.d_revoke = (int(*)())nodev;
			devsw_struct.d_dsdptr = NULL;
			devsw_struct.d_selptr = NULL;
#ifdef _POWER_MP
			devsw_struct.d_opts = DEV_MPSAFE;
#else
			devsw_struct.d_opts = 0;
#endif

			

			/* Add devsw struct to system devsw table */
			errnoval = devswadd(devno, &devsw_struct);
			if (errnoval) {
				scdisk_info.config_cnt--;
				unlockl(&(scdisk_info.lock));
				DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG,
				    errnoval, devno);
				return(errnoval);
			}
		}

		/* Search for diskinfo structure in configured list */
		hash_key = dev & DK_HASH;
		diskinfo = scdisk_list[hash_key];
		while (diskinfo != NULL) {
			if (diskinfo->devno == devno)
				break;
			diskinfo = diskinfo->next;
		}
		/* Verify device not already configured */
		if (diskinfo != NULL) {
			scdisk_info.config_cnt--;
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(EINVAL);
		}
		/* Allocate memory for diskinfo structure and zero it */
		diskinfo = (struct scdisk_diskinfo *)
		    xmalloc((uint) sizeof(struct scdisk_diskinfo), 2, 
		    (heapaddr_t) pinned_heap);
		if (diskinfo == NULL) {
			scdisk_info.config_cnt--;
			if (scdisk_info.config_cnt == 0) {
				(void) devswdel(devno);
			}
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(ENOMEM);
		}
		bzero((caddr_t) diskinfo, sizeof(struct scdisk_diskinfo));

		/*
		 *  Allocate memory for component dump table.  Three entries
		 *  are needed for the static structures, and then one
		 *  additional entry for each disk structure.
		 */
		 
		cdt_size = (uint) sizeof(struct cdt_head) + (3 + 
		    scdisk_info.config_cnt) * (uint) sizeof(struct cdt_entry);
		old_cdt = scdisk_info.cdt;
		scdisk_info.cdt = (struct cdt *) xmalloc(cdt_size, 2, 
		    (heapaddr_t) pinned_heap);
		if (scdisk_info.cdt == NULL) {
			scdisk_info.cdt = old_cdt;
			(void) xmfree((caddr_t)diskinfo, (heapaddr_t)
			    pinned_heap);
			scdisk_info.config_cnt--;
			if (scdisk_info.config_cnt == 0) {
				(void) devswdel(devno);
			}
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(ENOMEM);
		}
		/*  Free the old component dump table */
		(void) xmfree((caddr_t)old_cdt, (heapaddr_t) pinned_heap);

		bzero((caddr_t) scdisk_info.cdt, cdt_size);
		scdisk_info.cdt->_cdt_head._cdt_magic = DMP_MAGIC;
		bcopy("scdisk", scdisk_info.cdt->_cdt_head._cdt_name, 7);
		scdisk_info.cdt->_cdt_head._cdt_len = cdt_size;

		/* Move config data into diskinfo structure */
		uiomove((caddr_t)&config_info, (int) sizeof(struct disk_ddi),
		    (enum uio_rw) UIO_WRITE, uiop);

		diskinfo->devno = devno;
		diskinfo->adapter_devno = config_info.adapter_devno;
		diskinfo->scsi_id = config_info.scsi_id;
		diskinfo->lun_id = config_info.lun_id;
		diskinfo->safe_relocate = config_info.safe_relocate;
		diskinfo->async_flag = config_info.async_flag;
		diskinfo->extended_rw = config_info.extended_rw;
		diskinfo->reset_delay = config_info.reset_delay;
		diskinfo->stats.recovery_limit = config_info.recovery_limit;
		diskinfo->max_coalesce = config_info.max_coalesce;
		diskinfo->queue_depth = config_info.queue_depth;
		if (diskinfo->queue_depth < 1)
			diskinfo->queue_depth = 1;
		diskinfo->cmds_out = 0;
		diskinfo->max_request = config_info.max_request;
		diskinfo->q_type = config_info.q_type;
		diskinfo->q_err_value = config_info.q_err_value;
		diskinfo->clr_q_on_error = config_info.clr_q_on_error;
		diskinfo->cmd_tag_q = config_info.cmd_tag_q;
		diskinfo->buffer_ratio = config_info.buffer_ratio;
		diskinfo->prevent_eject = config_info.prevent_eject;
		diskinfo->reserve_lock = config_info.reserve_lock;
		diskinfo->cfg_prevent_ej = config_info.prevent_eject;
		diskinfo->cfg_reserve_lck = config_info.reserve_lock;
		diskinfo->load_eject_alt = config_info.load_eject_alt;
		diskinfo->pm_susp_bdr = config_info.pm_susp_bdr;
		diskinfo->dev_type = config_info.dev_type;
		diskinfo->rw_timeout = config_info.rw_timeout;
		diskinfo->fmt_timeout = config_info.fmt_timeout;
		diskinfo->start_timeout = config_info.start_timeout;
		diskinfo->reassign_timeout = config_info.reassign_timeout;
		diskinfo->q_clr = FALSE;
		diskinfo->q_status = 0;	
		diskinfo->timer_status = 0;
                diskinfo->play_audio = config_info.play_audio;
                diskinfo->cd_mode2_form1_code = config_info.cd_mode2_form1_code;
                diskinfo->cd_mode2_form2_code = config_info.cd_mode2_form2_code;
                diskinfo->cd_da_code = config_info.cd_da_code;
                diskinfo->multi_session = config_info.multi_session;
                diskinfo->valid_cd_modes = config_info.valid_cd_modes;
                diskinfo->cfg_block_size = config_info.block_size;


		/*
		 * Start out with with the block_size specified 
		 * by the configuration method.
		 */
		diskinfo->block_size = diskinfo->cfg_block_size;
	
		if (diskinfo->block_size % DK_BLOCKSIZE) {
			/*
			 * If the device's block size is not a multiple
			 * of 512 then set mult_of_blksize to 0.
			 */
			diskinfo->mult_of_blksize = 0;
		
		}
		else {
			diskinfo->mult_of_blksize = 
				diskinfo->block_size/DK_BLOCKSIZE;
		}

		diskinfo->play_audio_started = FALSE;

		/* Copy mode select data from dds data into diskinfo struct*/
		bcopy(config_info.mode_data,
		    diskinfo->mode_buf,
		    config_info.mode_data_length);
		diskinfo->mode_data_length = config_info.mode_data_length;

		/* Copy mode default data from dds data into diskinfo struct*/
		bcopy(config_info.mode_default_data, 
		    diskinfo->df_data, config_info.mode_default_length);

		/*
		 * Format the data base mode data
		 */
		scdisk_format_mode_data((char *)diskinfo->mode_buf, 
				    (struct scdisk_mode_format *)&diskinfo->dd,
				    (int)diskinfo->mode_data_length,
				    (char) TRUE,
				    (struct scdisk_diskinfo *)diskinfo);

		/*
		 * Format the data base mode default data
		 */
		scdisk_format_mode_data((char *)diskinfo->df_data, 
				    (struct scdisk_mode_format *)&diskinfo->df,
				    (int)config_info.mode_default_length,
				    (char)FALSE,
				    (struct scdisk_diskinfo *)NULL);

		/* Initialize remaining sections of disk info structure */
		diskinfo->dk_cmd_q_head = NULL;
		diskinfo->dk_cmd_q_tail = NULL;

		diskinfo->overide_pg_e = FALSE;
		diskinfo->currbuf = NULL;
		diskinfo->raw_io_cmd = NULL;
		diskinfo->low = NULL;
		diskinfo->dk_bp_queue.in_progress.head = NULL;
		diskinfo->dk_bp_queue.in_progress.tail = NULL;
		diskinfo->reset_count = 0;
		diskinfo->mode = DK_NORMAL;
		diskinfo->state = DK_OFFLINE;
		diskinfo->opened = FALSE;
		diskinfo->retain_reservation = FALSE;

		diskinfo->cmd_pending = FALSE;
		diskinfo->starting_close = FALSE;
		diskinfo->lock = LOCK_AVAIL;
		diskinfo->fp = NULL;
		diskinfo->dump_inited = FALSE;
		diskinfo->m_sense_status = 0;
		diskinfo->open_event = EVENT_NULL;

 


		diskinfo->current_cd_mode = CD_MODE1;
		/*
    		 * The CD-ROM Mode 1 Density code is determined from
		 * the mode data string in ODM.  It is extracted in
		 * the scdisk_format_mode_data routine not in the
		 * the configuration method.
		 */
                diskinfo->current_cd_code = diskinfo->cd_mode1_code;


		/* initialize fields for watchdog timer */
		diskinfo->watchdog_timer.watch.next = NULL;
		diskinfo->watchdog_timer.watch.prev = NULL;
		diskinfo->watchdog_timer.watch.func = (void (*)())scdisk_watchdog;
		diskinfo->watchdog_timer.watch.count = 0;
		diskinfo->watchdog_timer.watch.restart = 2;
		diskinfo->watchdog_timer.pointer = (void *)diskinfo;

		/* initialize fields for watchdog timer */
		diskinfo->pm_timer.watch.next = NULL;
		diskinfo->pm_timer.watch.prev = NULL;
		diskinfo->pm_timer.watch.func = (void (*)())scdisk_pm_watchdog;
		diskinfo->pm_timer.watch.count = 0;
		diskinfo->pm_timer.watch.restart = 2;
		diskinfo->pm_timer.pointer = (void *)diskinfo;

#ifdef _POWER_MP

		/*
		 * Loop until w_init succeeds
		 */
		while (w_init((struct watchdog *)&diskinfo->watchdog_timer));

		
		/*
		 * Loop until w_init succeeds
		 */
		while (w_init((struct watchdog *)&diskinfo->pm_timer));
#else
		w_init(&diskinfo->watchdog_timer);

		w_init(&diskinfo->pm_timer);
#endif

		/* Initialize cmd structs for use */
		scdisk_init_cmds(diskinfo);

		/* Initialize drive statistics */
		diskinfo->stats.recovery_limit = config_info.recovery_limit;
		diskinfo->stats.segment_size = config_info.segment_size;
		diskinfo->stats.segment_count = config_info.segment_cnt;
		diskinfo->stats.byte_count = config_info.byte_count;

		/* Initialize drive error record */
		bcopy(config_info.resource_name, 
		    diskinfo->error_rec.resource_name, 8);

		/* fill out fields in io statistics structure */
		bzero (&(diskinfo->dkstat), sizeof(struct dkstat));
		diskinfo->dkstat.dk_bsize = diskinfo->block_size;
		bcopy(config_info.resource_name, diskinfo->dkstat.diskname, 8);

		/* Register io statistics tracking structure */
		rc = iostadd(DD_DISK, &(diskinfo->dkstat));
		ASSERT(rc == 0);


		/* Link diskinfo structure in configured device list */
		diskinfo->next = scdisk_list[hash_key];
		scdisk_list[hash_key] = diskinfo;
#ifdef _POWER_MP
		/*
		 * Set up Interrupt processor synchronization
		 * lock, when device is configured.
		 */
		
		/*
		 * get minor number 
		 */
		min = minor(devno);
		
		lock_alloc(&(diskinfo->spin_lock),LOCK_ALLOC_PIN,
			   SCDISK_LOCK_CLASS,min);
		
		simple_lock_init(&(diskinfo->spin_lock));

#endif
		if (scdisk_info.config_cnt == 0x01) {
			/*
			 * Set device driver's thresholding value.
			 * This variable is exported so that
			 * hardware can then override mode data.
			 */
			scdisk_threshold = 4;

			/*
			 * Determine offset of mode data in 
			 * diskinfo structure.  This variable
			 * is exported so that hardware can then
			 * override mode data.
			 */
			scdisk_mode_data_offset = 
				(int)((int)(&(diskinfo->mode_buf))
				      - (int)diskinfo);

		}

#ifdef DEBUG

		/*
		 * Malloc array of Trace Buffers one time only, this is
		 * so we can have the buffer more readable 
		 * 32-byte aligned
		 */
		if (scdisk_info.config_cnt == 0x01) {
			scdisk_trace=(uint *)xmalloc(
					  (uint) (TRCLNGTH_PTR * sizeof(uint)), 
					      5, (heapaddr_t)pinned_heap);
		}
		/*
		 * Mark the pointer to the trace table
		 */

		strcpy(diskinfo->trace_label,TRACE_TABLE_PTR);

		/*
		 * Malloc Trace Buffers for this disk, this is
		 * so we can have the buffer more readable 
		 * 32-byte aligned
		 */

		diskinfo->dk_trace = (struct scdisk_trace *)xmalloc(
				 (uint)(sizeof(struct scdisk_trace) * TRCLNGTH),
					      5, (heapaddr_t)pinned_heap);
	
		if (scdisk_info.config_cnt <= TRCLNGTH_PTR) {
			/*
			 * If we have room in our list
			 * of disk trace tables, then
			 * add this one to it.
			 */
			scdisk_trace[(scdisk_info.config_cnt -1)] = 
				(uint) &(diskinfo->dk_trace[0]);
		}	
#endif
		diskinfo->pm_event = EVENT_NULL;


		/* Set pm_device_id */
		diskinfo->pm_device_id = config_info.pm_device_id;
		/* Initialize Power Management handle */
		diskinfo->pmh.handle.activity = PM_NO_ACTIVITY;
		diskinfo->pmh.handle.mode = PM_DEVICE_FULL_ON;
		diskinfo->pmh.handle.device_idle_time = config_info.pm_dev_itime;
		diskinfo->pmh.handle.device_standby_time = config_info.pm_dev_stime;
		diskinfo->pmh.handle.handler = scdisk_pm_handler;
		diskinfo->pmh.handle.private = (uchar *)&diskinfo->pmh;
		diskinfo->pmh.handle.devno = devno;
		if (diskinfo->dev_type == DK_CDROM) {
			/*
			 * If this is a CD-ROM device, then
			 * indicate it is an audio input device.
			 */
			diskinfo->pmh.handle.attribute = PM_AUDIO_INPUT;
		} else {
			diskinfo->pmh.handle.attribute = config_info.pm_dev_att;
		}
		diskinfo->pmh.handle.device_idle_time1 = 0;
		diskinfo->pmh.handle.device_idle_time2 = 0;
		diskinfo->pmh.handle.device_logical_name = 
			diskinfo->dkstat.diskname;



		diskinfo->pmh.handle.pm_version = PM_PHASE2_SUPPORT; 

		diskinfo->pmh.diskinfo = diskinfo;
		/* Register Power Management handle */
		rc = pm_register_handle((struct pm_handle *)&(diskinfo->pmh), 
					PM_REGISTER);
		ASSERT(rc == PM_SUCCESS);


		break;
	case CFG_TERM:
		/* Search for diskinfo structure in configured device list */
		hash_key = dev & DK_HASH;
		diskinfo = scdisk_list[hash_key];
		last_diskinfo = diskinfo;
		while (diskinfo != NULL) {
			if (diskinfo->devno == devno)
				break;
			last_diskinfo = diskinfo;
			diskinfo = diskinfo->next;
		}
		if (diskinfo == NULL) {
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, 0, devno);
			return(0);
		}
		if (diskinfo->opened) {
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, EINVAL,
			    devno);
			return(EBUSY);
		}

		/* Open adapter for device and store filepointer */
		errnoval = fp_opendev(diskinfo->adapter_devno, (int) FWRITE,
		    (caddr_t) NULL, (int) NULL, &(diskinfo->fp));
		if (errnoval == 0) {
			/* Issue a start IOCTL to the adapter driver */
			idlun = IDLUN(diskinfo->scsi_id, diskinfo->lun_id);
			errnoval = fp_ioctl(diskinfo->fp, (uint) SCIOSTART,
			    (caddr_t) idlun, (int) NULL);
			if (errnoval == 0) {
				/* pin bottom half routines */
				errnoval = pincode((int(*)())scdisk_iodone);
				if (errnoval == 0) {
					diskinfo->errno = 0x00;
					diskinfo->disk_intrpt = 1;
					scdisk_start_unit_disable(diskinfo,
					    (uchar) DK_STOP,(uchar) FALSE);
					scdisk_sleep(diskinfo,
						     &(diskinfo->disk_intrpt),
						     &(diskinfo->open_event));


					errnoval = diskinfo->errno;
					(void *) unpincode((int(*)())
					    scdisk_iodone);
				}
				fp_ioctl(diskinfo->fp, (uint) SCIOSTOP,
				    (caddr_t) idlun, (int) NULL);
			}
			fp_close(diskinfo->fp);
		}

		if (diskinfo == last_diskinfo) {
			scdisk_list[hash_key] = diskinfo->next;
		} else {
			last_diskinfo->next = diskinfo->next;
		}

		scdisk_info.config_cnt--;
#ifdef _POWER_MP
		/*
		 * Free Interrupt processor synchronization
		 * lock, when last device is unconfigured.
		 */
		lock_free(&(diskinfo->spin_lock));

#endif
		/* Check for last device being deleted */
		if (scdisk_info.config_cnt == 0) {
			(void) devswdel(devno);
			(void) xmfree((caddr_t)scdisk_info.cdt,
				      (heapaddr_t) pinned_heap);
			scdisk_info.cdt = NULL;
#ifdef DEBUG
			/*
			 * If this is the last disk to unconfigure
			 * then free our buffer of disk trace tables.
			 */
			xmfree((caddr_t)scdisk_trace,
			       (heapaddr_t)pinned_heap);
#endif

		} else {
			/*
			 *  Allocate memory for component dump table.  Three
			 *  entries are needed for the static structures, and
			 *  then one additional entry for each disk structure.
			 */
		 
			cdt_size = (uint) sizeof(struct cdt_head) + (3 + 
			    scdisk_info.config_cnt) * (uint) sizeof(struct 
			    cdt_entry);
			old_cdt = scdisk_info.cdt;
			scdisk_info.cdt = (struct cdt *) xmalloc(cdt_size, 2,
			    (heapaddr_t) pinned_heap);
			if (scdisk_info.cdt == NULL) {
				scdisk_info.cdt = old_cdt;
			} else {
				(void) xmfree((caddr_t)old_cdt,
				    (heapaddr_t) pinned_heap);
			}
			bzero((caddr_t) scdisk_info.cdt, cdt_size);
			scdisk_info.cdt->_cdt_head._cdt_magic = DMP_MAGIC;
			bcopy("scdisk",scdisk_info.cdt->_cdt_head._cdt_name,7);
			scdisk_info.cdt->_cdt_head._cdt_len = cdt_size;
		}
#ifdef DEBUG
		/*
		 * Free our disk trace table.
		 */
		xmfree((caddr_t)diskinfo->dk_trace,
		       (heapaddr_t)pinned_heap);
#endif

#ifdef _POWER_MP
		/*
		 * Loop until w_clear succeeds
		 */
		while (w_clear((struct watchdog *)&diskinfo->watchdog_timer));

		/*
		 * Loop until w_clear succeeds
		 */
		while (w_clear((struct watchdog *)&diskinfo->pm_timer));


#else
		w_clear(&diskinfo->watchdog_timer);
		w_clear(&diskinfo->pm_timer);
#endif
		

		/* Unregister Power Management handle */
		pm_register_handle((struct pm_handle *)&(diskinfo->pmh), 
				   PM_UNREGISTER);

		


		/* de-register the dkstat structure */
		iostdel(&(diskinfo->dkstat));
		(void) xmfree((caddr_t)diskinfo, (heapaddr_t)pinned_heap);
		/*
		 *  The unconfigure method checks for EBUSY to see if the
		 *  device unconfigured the device or not.  If we got this
		 *  far, we must have unconfigured it, so don't return
		 *  EBUSY (EIO will basically be ignored).
		 */
		if (errnoval == EBUSY) {
			errnoval = EIO;
		}
		break;

	default:
		/* Unknown op */
		errnoval = EINVAL;
	}
#ifdef DEBUG
	DKprintf(("Exiting scdisk_config errnoval[0x%x]\n", errnoval));
#endif
	unlockl(&(scdisk_info.lock));
	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CONFIG, errnoval, devno);
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_open						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Open Routine.			*/
/*		This routine prepares a device for utilization by the	*/
/*		system. The disk's information structure is located	*/
/*		in the scdisk_list.					*/
/*		The open is checked for validity, and if the first	*/
/*		valid open, the diskinfo structure is pinned and placed	*/
/*		on the scdisk_open_list. The adapter driver is called	*/
/*		to open and start the device.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo		Disk device specific information*/
/*		scdiskinfo		General disk driver information	*/
/*		scdisk_list		List of configured devices	*/
/*		scdisk_open_list	List of open devices		*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		rwflag	- FREAD for read only, FWRITE for read/write	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter, could be SC_DIAGNOSTIC	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- Minor number out of range			*/
/*			- Device not configured				*/
/*			- Devopen to adapter failed			*/
/*			- Devioctl to start device failed		*/
/*			- Device open cycle failed			*/
/*		EACCES  - Device is in DIAGNOSTIC mode			*/
/*			- Attempt to enter DIAGNOSTIC mode while open	*/
/*			- Device is in SINGLE open mode			*/
/*		EPERM   - Insufficient authority to open		*/
/*		EWRPROTECT- Write protected device			*/
/*		EBUSY	- Single mode open attempted to device that is	*/
/*			  is already open.				*/
/*		ENODEV  - Invalid devno					*/
/*			- Device not configured				*/
/*		ENOTREADY- Media Missing.				*/
/*									*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		fp_opendev	fp_close				*/
/*		fp_ioctl						*/
/*		pincode		unpincode				*/
/*		dmp_add		dmp_del					*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_open(
dev_t	devno,
int	rwflag, 
int	chan, 
int	ext)
{
	int			errnoval, rc, dev, hash_key, idlun;
	struct scdisk_diskinfo	*diskinfo;
	struct devinfo		iocinfo_buf;




	DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_OPEN, 0, devno, rwflag, chan, ext,
	    0);
	errnoval = 0;
	dev = minor(devno);

	(void) lockl(&(scdisk_info.lock), LOCK_SHORT);


	/* Locate diskinfo structure in device list */
	hash_key = dev & DK_HASH;
	diskinfo = scdisk_list[hash_key];
	while (diskinfo != NULL) {
		if (diskinfo->devno == devno)
			break;
		diskinfo = diskinfo->next;
	}
	if (diskinfo == NULL) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, ENODEV, devno);
		return(ENODEV);
	}
	if ((ext && (rc = privcheck(RAS_CONFIG))) &&
	    (!(ext & SC_SINGLE))) {
		/*
		 * If extended open other then SC_SINGLE, but with no
		 * authority then fail.
		 */
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, ENODEV, devno);
		return(rc);
	}
	if ((diskinfo->opened == TRUE) && ((ext & SC_DIAGNOSTIC) ||
	    (diskinfo->mode & DK_DIAG) || (diskinfo->mode & DK_SINGLE_MD))) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, EACCES, devno);
		return(EACCES);
	}
	if ((diskinfo->opened == TRUE) && (ext & SC_SINGLE)) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, EBUSY, devno);
		return(EBUSY);
	}
        if (diskinfo->opened == TRUE) {
                if ((diskinfo->wprotected) &&  
                    ((rwflag & DWRITE) || (rwflag & DAPPEND))) {
                        /*
                         * If device is write protected, then fail any opens 
                         * that try to write to the device.
                         */
                        unlockl(&(scdisk_info.lock));
                        DDHKWD1(HKWD_DD_SCDISKDD,DD_EXIT_OPEN,EWRPROTECT,devno);
                        return(EWRPROTECT);
                }
        }

	if (diskinfo->opened == TRUE) {
		if (ext & SC_RETAIN_RESERVATION) {
			diskinfo->retain_reservation = TRUE;
		}
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, 0, devno);
		return(0);
	} else {
		diskinfo->errno = 0;
	}
	if (ext & SC_NO_RESERVE) {
		/*
		 * If this is a no reserve openx then
		 * guarantee that we will not issue a reserve
		 * as part of open nor part of the reset cycle
		 * error recovery.
		 */
		diskinfo->reserve_lock = FALSE;

	}

        diskinfo->wprotected = FALSE;

	/*
	 * Start out with with the block_size specified 
	 * by the configuration method.
	 */
	diskinfo->block_size = diskinfo->cfg_block_size;
	
	if (diskinfo->block_size % DK_BLOCKSIZE) {
		/*
		 * If the device's block size is not a multiple
		 * of 512 then set mult_of_blksize to 0.
		 */
		diskinfo->mult_of_blksize = 0;
		
	}
	else {
		diskinfo->mult_of_blksize = 
			diskinfo->block_size/DK_BLOCKSIZE;
	}

	diskinfo->ioctl_chg_mode_flg = FALSE;	
	
	/*
	 * Allocate command block pool
	 */

	if ((rc = scdisk_alloc(diskinfo))) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, EINVAL, devno);
		return(rc);
	}


	/* Open adapter for device and store filepointer */
	errnoval = fp_opendev(diskinfo->adapter_devno, (int) FWRITE,
	    (caddr_t) NULL, (int) NULL, &(diskinfo->fp));
	if (errnoval != 0) {
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}
	/*
	 * Issue IOCINFO to the adapter driver to find out the maximum request
	 * size.
	 */
	idlun = IDLUN(diskinfo->scsi_id, diskinfo->lun_id);
	errnoval = fp_ioctl(diskinfo->fp, (uint) IOCINFO,
	    (caddr_t) &iocinfo_buf, (int) NULL);
	if (errnoval != 0) {
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	} else {

		/*
                 *  Max_request can not exceed adapter's max transfer size
                 */

		if (diskinfo->max_request > iocinfo_buf.un.scsi.max_transfer) 
			diskinfo->max_request = 
				iocinfo_buf.un.scsi.max_transfer;
		

		/*
                 *  If max_request is larger than 128K and extended read
                 *  and write commands are not supported, set the max
                 *  to 128K so we will not overrun the normal r/w cmds.
                 */
		if ((diskinfo->max_request > 0x20000) && 
		    !(diskinfo->extended_rw)) {
			/*
			 *  If max_request is larger than 128K and extended read
			 *  and write commands are not supported, set the max
			 *  to 128K so we will not overrun the normal r/w cmds.
			 */
			diskinfo->max_request = 0x20000;
		}
		else if ((diskinfo->max_request > 
			  (diskinfo->block_size * 65535))
			&& (diskinfo->extended_rw)) {
			/*
			 * If max_request is larger then 2exp 16 - 1
			 * times the device block size and extended
			 * read and write commands are supported then set it
			 * to this value so we will not overrun the extended 
			 * r/w commands.
			 */
			diskinfo->max_request =  diskinfo->block_size * 65535;
		}
		if (diskinfo->max_coalesce > diskinfo->max_request) {
			diskinfo->max_coalesce = diskinfo->max_request;
		}
	}
	/* Issue a start IOCTL to the adapter driver */
	errnoval = fp_ioctl(diskinfo->fp, (uint) SCIOSTART, (caddr_t) idlun,
	    (int) NULL);
	if (errnoval != 0) {
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}

	/* pin bottom half routines */
	errnoval = pincode((int(*)())scdisk_iodone);
	if (errnoval) {
		fp_ioctl(diskinfo->fp, (uint) SCIOSTOP, (caddr_t) idlun,
		    (int) NULL);
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,opentrc, trc,(char)0, (uint)devno, (uint)rwflag,
		(uint)chan, (uint)ext,(uint)0);
#endif	
#endif
#endif
	/*  Register component dump table function on first open */
	if (scdisk_info.open_cnt == 0) {
		(void) dmp_add(scdisk_cdt_func);
	}

	/* Issue the BDR if requested */
	if (ext & SC_FORCED_OPEN) {
		idlun = IDLUN(diskinfo->scsi_id, diskinfo->lun_id);
		errnoval = fp_ioctl(diskinfo->fp, (uint) SCIORESET,
		    (caddr_t) idlun, (int) NULL);
		if (errnoval != 0) {
			if (scdisk_info.open_cnt == 0) {
				(void) dmp_del(scdisk_cdt_func);
			}
			(void *) unpincode((int(*)())scdisk_iodone);
			fp_ioctl(diskinfo->fp, (uint) SCIOSTOP,
			    (caddr_t) idlun, (int) NULL);
			fp_close(diskinfo->fp);
			(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
			unlockl(&(scdisk_info.lock));
			DDHKWD1(HKWD_DD_SCDISKDD,DD_EXIT_OPEN,errnoval,devno);
			return(errnoval);
		}
	}
	if (ext & SC_DIAGNOSTIC) {
		if (ext & SC_RETAIN_RESERVATION) {
			diskinfo->retain_reservation = TRUE;
		}
		/* Place diskinfo struct in scdisk_open_list */
		diskinfo->next_open = scdisk_open_list[hash_key];
		scdisk_open_list[hash_key] = diskinfo;
		diskinfo->state = DK_ONLINE;
		diskinfo->mode = DK_DIAG;
		diskinfo->opened = TRUE;
		scdisk_info.open_cnt++;
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, 0, devno);
		return(0);
	}
	diskinfo->disk_intrpt = 1;
	scdisk_test_unit_ready_disable(diskinfo);
	scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),&(diskinfo->open_event));

	if (diskinfo->dev_type == DK_CDROM) {
		/*
		 * ANSI SCSI-2 does not require SCSI CDROMS to support the
		 * WP bit in mode sense.  So we must guarantee here that
		 * we treat them as write protected devices.
		 */
		diskinfo->wprotected = TRUE;
	}
        if ((diskinfo->wprotected) && ( 
            (rwflag & DWRITE) || (rwflag & DAPPEND))) {
                /*
                 * If device is write protected, then fail any opens that
                 * try to write to the device.
                 */
                diskinfo->errno = EWRPROTECT;
        }
        /* 
         * Check for reset cycle failure or if device is being opened for
         * writes when it is write protected.
         */

	if (diskinfo->errno != 0) {
		errnoval = diskinfo->errno;
		scdisk_release_allow(diskinfo);
		if (scdisk_info.open_cnt == 0) {
			(void) dmp_del(scdisk_cdt_func);
		}
		(void *) unpincode((int(*)())scdisk_iodone);
		fp_ioctl(diskinfo->fp, (uint) SCIOSTOP, (caddr_t) idlun,
		    (int) NULL);
		fp_close(diskinfo->fp);
		(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);
		return(errnoval);
	}
	if (ext & SC_RETAIN_RESERVATION) {
		diskinfo->retain_reservation = TRUE;
	}

	/*
	 * initialize timer starts  on first open.  
	 */
	diskinfo->reset_failures = 0;

	/* Place diskinfo struct in scdisk_open_list */
	diskinfo->next_open = scdisk_open_list[hash_key];
	scdisk_open_list[hash_key] = diskinfo;
	diskinfo->state = DK_ONLINE;
	if (ext & SC_SINGLE) {
		/*
		 * If this is an single open mode.
		 * This will prevent others
		 * from trying to open the device.
		 */
		diskinfo->mode = DK_SINGLE_MD;
	}
	else
		diskinfo->mode = DK_NORMAL;
	diskinfo->opened = TRUE;
	scdisk_info.open_cnt++;

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,opentrc, trc,(char)1, (uint)errnoval, 
		      (uint)diskinfo->mode,(uint)0, (uint)0,(uint)0);
#endif	
#endif
#endif
	unlockl(&(scdisk_info.lock));
	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_OPEN, errnoval, devno);

	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_close						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Close Routine.		*/
/*		This routine searches the scdisk_open_list for		*/
/*		the specified device, and unlinks it when found.	*/
/*		The device is then released for other initiators.	*/
/*		The device's diskinfo structure is unpinned from	*/
/*		memory, device open flag is cleared, and the		*/
/*		adapter driver is called via fp_ioctl to stop the	*/
/*		device and via fp_close to close if necessary.		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo        Disk device specific information	*/
/*		scdiskinfo      General disk driver information		*/
/*		scdisk_list     List of configured devices		*/
/*		scdisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter, could be SC_DIAGNOSTIC	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO     - Minor number out of range			*/
/*			- Device not configured				*/
/*			- Device not open				*/
/*			- Close cycle failed				*/
/*		ENXIO 	- Invalid devno					*/
/*			- Device not configured				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*				unpincode				*/
/*		fp_ioctl	fp_close				*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_close(
dev_t	devno,
int	chan, 
int	ext)
{
	int			errnoval, dev, rc, hash_key, idlun;
	struct scdisk_diskinfo	*diskinfo, *last_diskinfo;
	short			i,index;
	short			p_length;
	struct dk_cmd		*cmd_ptr;


	DDHKWD1(HKWD_DD_SCDISKDD, DD_ENTRY_CLOSE, 0, devno);
	errnoval = 0;
	dev = minor(devno);


	(void) lockl(&(scdisk_info.lock), LOCK_SHORT);

	/* Locate diskinfo structure in scdisk_open_list */
	hash_key = dev & DK_HASH;
	diskinfo = scdisk_open_list[hash_key];
	last_diskinfo = diskinfo;
	while (diskinfo != NULL) {
		if (diskinfo->devno == devno)
			break;
		else {
			last_diskinfo = diskinfo;
			diskinfo = diskinfo->next_open;
		}
	}

	if (diskinfo == NULL) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CLOSE, ENXIO, devno);
		return(ENXIO);
	}
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,closetrc, trc,(char)0, (uint)devno, (uint)0,
		      (uint)chan, (uint)ext,(uint)0);
#endif	
#endif
#endif
	/* Unlink diskinfo struct from scdisk_open_list */
	if (last_diskinfo == diskinfo) {
		scdisk_open_list[hash_key] = diskinfo->next_open;
	}
	else {
		last_diskinfo->next_open = diskinfo->next_open;
	}

	/*
	 * Lock the disk lock for this device to wait for an ioctl to complete.
	 */
	(void) lockl(&(diskinfo->lock), LOCK_SHORT);

	/*
	 * Set the following flags to prevent any new commands from
	 * being issued to the device driver
	 */

	diskinfo->opened = FALSE;
	diskinfo->starting_close = TRUE;
	diskinfo->state = DK_OFFLINE;



	/* Check for requests awaiting completion */

	while (diskinfo->cmds_out || 
	       (diskinfo->dk_cmd_q_head != NULL) ||
	       (diskinfo->ioctl_cmd.flags & DK_READY) ||
	       (diskinfo->dk_bp_queue.in_progress.head != NULL) ||
	       (diskinfo->currbuf != NULL) ||
	       diskinfo->cmd_pending || 
	       diskinfo->dump_inited) {

		if (diskinfo->dk_cmd_q_head != NULL) {
			cmd_ptr = diskinfo->dk_cmd_q_head;
			if ((cmd_ptr->type == DK_Q_RECOV) &&
			    (cmd_ptr->next == NULL)) {
				
				/* 
				 * If this is a Q Recovery command
				 * on our error queue and if there
				 * are no other commands on our error
				 * queue, lets remove it.  This can
				 * be done, since the close will
				 * issue an SCIOSTOP to the adapter
				 * thus releasing its halted queue state.
				 */
				scdisk_d_q_cmd_disable(cmd_ptr,
						       (uchar) DK_CMD_Q);
				scdisk_free_cmd_disable(cmd_ptr);

			}

		}

		/*
		 * We will wait until all outstanding commands complete, 
		 * all pending commands (i.e. bufs, ioctls, and error recovery
		 * commands) get issued and complete, and for a dump to 
		 * to complete.
		 */
		delay(HZ);  /* wait one second for operation to complete */
	}

	if (diskinfo->play_audio_started) {
		/*
		 * If a Play audio operation was issued but not
		 * a subsequent stop unit, then the device may
		 * still be playing the audio.  Therefore we
		 * we need to issue a stop unit to guarantee
		 * that it will stop playing on close.
		 */



		diskinfo->play_audio_started = FALSE;
		diskinfo->errno = 0x00;
		diskinfo->disk_intrpt = 1;
		(void) scdisk_start_unit_disable(diskinfo,(uchar) DK_STOP,
						 (uchar) FALSE);		
		scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
			     &(diskinfo->open_event));


		errnoval = diskinfo->errno;


	}

	/* Check for diagnostic mode or retain reservation, release if OK */
	if (!(diskinfo->mode & DK_DIAG) && !(diskinfo->retain_reservation)) {
		/* Release device for other initiators */
		if (diskinfo->state == DK_RST_FAILED) {
			/*
			 * Since the reset sequence failed assume that it is
			 * released (i.e. do not issue release).  The 
			 * alternative would be to do the
			 * reset cycle over and potentially hang the system.
			 * Also if this host is having difficulty opening the
			 * disk then most likely the other host will too, 
			 * regardless if it is  released  or not.
			 * We will however flag this as an error.
			 */
			diskinfo->errno = EIO;
		}
		else {
			/*
			 * If we did a reserve on open
			 * then we must do a release on close.
			 */
			diskinfo->errno = 0x00;
			scdisk_release_allow(diskinfo);
		}

		errnoval = diskinfo->errno;
	}
	else if ((diskinfo->retain_reservation) && 
		 (diskinfo->prevent_eject) && 
		 (diskinfo->state != DK_RST_FAILED)) {

		/*
		 * If we did a prevent media removal on open
		 * then we must do an allow media removal on close
		 * even for the retain_reservation open case. The 
		 * only exception is the case of reset sequence failure,
		 * because this may cause a hang.
		 */
		diskinfo->errno = 0x00;
		
		diskinfo->disk_intrpt = 1;
		scdisk_prevent_allow_disable(diskinfo,(uchar) DK_ALLOW);
		scdisk_start_disable(diskinfo);
		scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
			     &(diskinfo->open_event));
		
	}
	diskinfo->state = DK_OFFLINE;
	diskinfo->mode = DK_NORMAL;
	diskinfo->starting_close = FALSE;
	diskinfo->retain_reservation = FALSE;

	if (diskinfo->overide_pg_e) {
		/*
		 * The desired Mode data from configuration has
		 * been modified.  So we must revert it back to
		 * it's original values on close.
		 */
	  	diskinfo->overide_pg_e = FALSE;
		index = diskinfo->dd.page_index[0xe];
		p_length = diskinfo->mode_buf[++index];

		if (p_length > DK_MAX_SIZE_PG_E) {
		  	p_length = DK_MAX_SIZE_PG_E;
		}
		for (i=DK_VOL_START_PG_E;i<=p_length;i++) {
		  	diskinfo->mode_buf[index + i] =
				  diskinfo->mode_page_e[i-DK_VOL_START_PG_E];
					  
		}


	}
	/*
	 * Revert back to original settings
	 */

	diskinfo->prevent_eject = diskinfo->cfg_prevent_ej;
	diskinfo->reserve_lock = diskinfo->cfg_reserve_lck;

	diskinfo->current_cd_code = diskinfo->cd_mode1_code;
	diskinfo->current_cd_mode = CD_MODE1;


	/*
	 * initialize timer starts on close so it is ready for next open.  
	 */
	diskinfo->reset_failures = 0;

	/* Terminate adapter connection for this device */
	idlun = IDLUN(diskinfo->scsi_id, diskinfo->lun_id);
	rc = fp_ioctl(diskinfo->fp, (uint)SCIOSTOP, (caddr_t)idlun, (int)NULL);
	errnoval = (errnoval == 0) ? rc : errnoval;
	rc = fp_close(diskinfo->fp);
	errnoval = (errnoval == 0) ? rc : errnoval;
	scdisk_info.open_cnt--;
	if (scdisk_info.open_cnt == 0) {
		(void) dmp_del(scdisk_cdt_func);
	}
	(void)xmfree((caddr_t)(diskinfo->cmd_pool),pinned_heap);

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,closetrc, trc,(char)1, (uint)devno, 
		      (uint)errnoval,(uint)rc, (uint)diskinfo->errno,(uint)0);
#endif
#endif	
#endif
	rc = unpincode((int(*)())scdisk_iodone);
	errnoval = (errnoval == 0) ? rc : errnoval;



	unlockl(&(diskinfo->lock));
	unlockl(&(scdisk_info.lock));
	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_CLOSE, errnoval, devno);
	return(errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_read						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Raw Read Routine.		*/
/*		This routine processes raw IO requests from the system.	*/
/*		A buf struct is allocated, and the b_options field	*/
/*		is initialized with the ext argument. Uphysio is called	*/
/*		to begin processing of the raw IO request.		*/
/*		Upon completion of the operation, the buf struct is	*/
/*		freed.							*/
/*                                                                      */
/*		Additional functionality will process raw requests      */
/*		for non-512 block sizes.  For non-512 block sizes 	*/
/*		which have accesses which do not fall on their block 	*/
/*		boundary, pre-reads will be made of the affected blocks	*/
/*		and 512 aligned data merged in.                         */
/*                                                                      */
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio	*/
/*              or a value determined by individual scsi cmds (non-     */
/*              512 block sizes)                                        */ 
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			       - Device is open in diagnostic mode.	*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			       - The device is offline.			*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*		               - Pinning of user's buffer failed.	*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_read(
dev_t		devno,
struct uio	*uiop,
int		chan, 
int		ext)
{
	int	errnoval;
        dev_t   dev;


	DDHKWD1(HKWD_DD_SCDISKDD, DD_ENTRY_READ, 0, devno);

	errnoval = scdisk_rdwr(devno,uiop,chan,ext,B_READ);



	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_READ, errnoval, devno);
	return(errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_write						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Raw Write Routine.		*/
/*		This routine processes raw IO requests from the system.	*/
/*		A buf struct is allocated, and the b_options field	*/
/*		is initialized with the ext argument. Uphysio is called	*/
/*		to begin processing of the raw IO request.		*/
/*		Upon completion of the operation, the buf struct is	*/
/*		freed.							*/
/*                                                                      */
/*		Additional functionality will process raw requests      */
/*		for non-512 block sizes.  For non-512 block sizes 	*/
/*		which have accesses which do not fall on their block 	*/
/*		boundary, pre-reads will be made of the affected blocks	*/
/*		and 512 aligned data merged in.                         */
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			       - Device is open in diagnostic mode.	*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			       - The device is offline.			*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*		               - Pinning of user's buffer failed.	*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_write(
dev_t		devno,
struct uio	*uiop,
int		chan, 
int		ext)
{
	int	errnoval;


	DDHKWD1(HKWD_DD_SCDISKDD, DD_ENTRY_WRITE, 0, devno);


	errnoval = scdisk_rdwr(devno,uiop,chan,ext,B_WRITE);



	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_WRITE, errnoval, devno);
	return(errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:	scdisk_rdwr						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver Raw Read/Write Routine.	*/
/*		This routine processes raw IO requests from the system.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		uiop	- Address of uio struct describing request	*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*		rw_flag - Equals B_WRITE for writes and B_READ for reads*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		This routine returns the value received from uphysio	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			       - Device is open in diagnostic mode.	*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			       - The device is offline.			*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*		               - Pinning of user's buffer failed.	*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		uphysio							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_rdwr(
dev_t		devno,
struct uio	*uiop,
int		chan, 
int		ext,
int             rw_flag)
{
	int	errnoval, dev, hash_key;
        struct scdisk_diskinfo  *diskinfo;


        /* 
	 * Lock to access structure 
	 */
        (void) lockl(&(scdisk_info.lock),LOCK_SHORT);

        /* 
	 * Need to determine diskinfo structure for devno in order 
         * to tell if block size is 512 or larger 
	 */
        dev = minor(devno);
        hash_key = dev & DK_HASH;
        diskinfo = scdisk_open_list[hash_key];
        while (diskinfo != NULL) {
                if (diskinfo->devno == devno)
                        break;
                diskinfo = diskinfo->next_open;
        }

	/*
         * Now, hopefully have diskinfo structure for devno. Check      
         * to be sure we found a structure.                       
	 */
        if (diskinfo == NULL) {
                /* 
		 * No open disk at this devno 
		 */ 
                unlockl(&(scdisk_info.lock));
                return(ENXIO);
        }               

        /* 
	 * Lock  the device's lock 
	 */
        (void) lockl(&(diskinfo->lock), LOCK_SHORT);

        /* 
	 * Finished with scdisk_open_list 
	 */
        unlockl(&(scdisk_info.lock));

	if (diskinfo->block_size == DK_BLOCKSIZE) { 

	 	/* 
	 	 * Check to see if the current block size is 512 bytes 
	 	 */

                /* 
		 * Give up our lock 
		 */
                unlockl(&(diskinfo->lock));
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,rdwr, trc,(char)0, (uint)devno, (uint)uiop,
		      (uint)chan,(uint)ext,(uint)diskinfo->block_size);
#endif
#endif	
#endif
                /* 
		 * Process the 512 blocksize request using uphysio 
		 */
                errnoval = uphysio(uiop, rw_flag, 9, devno, scdisk_strategy,
                           scdisk_mincnt, ext);



        } else { 

                /* 
		 * Device is using a block size other then 512 bytes
		 * per block.
		 */
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,rdwr, trc,(char)1, (uint)devno, 
		      (uint)uiop,(uint)chan, 
		      (uint)ext,(uint)diskinfo->block_size);
#endif
#endif	
#endif
                errnoval = scdisk_raw(uiop,ext,rw_flag,diskinfo);

   		if (errnoval == ESOFT) {
			/* 
			 * We need to filer ESOFT's here, since
			 * uphysdone may not be called.  Uphysdone
			 * filters ESOFTs.
			 */
			errnoval = 0;
		}

                /* 
		 * Give up our lock 
		 */
                unlockl(&(diskinfo->lock));


        }

	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_mincnt						*/
/*									*/
/*	FUNCTION:							*/
/*		This routine is used to breakup raw requests for	*/
/*		processing by uphysio.					*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*									*/
/*	INPUTS:								*/
/*		bp		- Pointer to buf structure for request	*/
/*		minparms	- Parameters for breakup		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_mincnt(
struct buf	*bp,
void		*minparms)
{

	struct scdisk_diskinfo *diskinfo;        /* disk information structure*/




	diskinfo = scdisk_open_list[minor(bp->b_dev) & DK_HASH];
	while (diskinfo != NULL) {
		if (diskinfo->devno == bp->b_dev)
			break;
		diskinfo = diskinfo->next_open;
	}

	if (diskinfo == NULL) {
		return(ENXIO);
	}

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,mincnt, entry,(char)0, (uint)bp, 
		      (uint)bp->b_bcount,
		      (uint)minparms, (uint)0,(uint)0);
#endif	
#endif
#endif
	if (bp->b_bcount > diskinfo->max_request)
		/*
		 * If this buf is bigger then our max transfer
		 * size then change count to our max transfer
		 */
		bp->b_bcount = diskinfo->max_request;
	
	
	bp->b_options = (int) minparms;
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,mincnt, exit,(char)0, (uint)bp, 
		      (uint)bp->b_bcount,
		      (uint)bp->b_options, (uint)0,(uint)0);
#endif	
#endif
#endif
	return(0);
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_raw                                              */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine breaks up a uio structure into separate    */
/*              requests and handles them using uphysio (for requests   */
/*              which fall on both a blocksize boundary and have a      */
/*              transfer length which is a multiple of the blocksize)   */
/*              or a buffering scheme which executes one SCSI command   */
/*              at a time.                                              */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              uiop    - Address of uio struct describing request      */
/*              ext     - extended parameter                            */
/*              rw_flag - either B_READ or B_WRITE                      */
/*              diskinfo - Address of scdisk_diskinfo struct            */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns the value received from uphysio    */
/*              or a value determined by individual scsi cmds (non-     */
/*              512 block sizes)                                        */ 
/*                                                                      */
/*      ERROR DESCRIPTION:                                              */
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			       - Device is open in diagnostic mode.	*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			       - The device is offline.			*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*		               - Pinning of user's buffer failed.	*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*              uphysio                                                 */
/*                                                                      */
/*                                                                      */
/************************************************************************/
int
scdisk_raw(
struct uio      *uiop,
int             ext,
int             rw_flag,
struct scdisk_diskinfo  *diskinfo)
{
        int             errnoval;	/* Return code of routine.	*/
        struct iovec    *iovp;		/* Individual Iovec struct.	*/
        struct xmem     *xmemp;		/* Cross memory descriptor.	*/
#ifdef _LONG_LONG
        offset_t        offset;		/*64bit byte offset for transfer*/
#else
        off_t           offset;		/* Byte offset for transfer.	*/
#endif
        int             i;		/* General counter.		*/
        struct uio      uiotmp;



	if (diskinfo->mode == DK_DIAG) {
		/* 
		 * Do not allow raw I/O if we are in
		 * diagnostic mode.
		 */
	 	return (EACCES);
	}

        /* 
	 * Break up uio into separate requests and handle       
         * using uphysio or our own buffering mechanism.        
	 */

        /* 
	 * Setup ptr to first iovec, set offset on device 
	 */
        iovp = uiop->uio_iov; 
        xmemp = uiop->uio_xmem;
        offset = uiop->uio_offset;

        /* 
	 * For every iovec in this request 
	 */
        for (i=0; i<uiop->uio_iovcnt; i++) {


#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw, trc,(char)0, (uint)diskinfo->block_size, 
		      (uint)offset,(uint)iovp->iov_len, (uint)i,
		      (uint)uiop->uio_iovcnt);
#endif	
#endif
#endif

   		if (diskinfo->block_size % DK_BLOCKSIZE) {
		  	/*
			 * Devices that use block sizes that
			 * are not multiples of 512 need to
			 * be handled differently
			 */
		  	errnoval = scdisk_raw_io(iovp,uiop,
						 offset, rw_flag, 
						 diskinfo, ext);

                        /* 
			 * If problem, return to caller 
			 */
                        if (errnoval != 0) 
			  	break;

		} else if (((offset % diskinfo->block_size) == 0) &&
                    ((iovp->iov_len % diskinfo->block_size) == 0)) {
			/* 
			 * Check if this request is on both a blocksize 
			 * boundary and has a transfer length which is 
			 * a multiple of the blocksize.
			 */
 
                        /* 
			 * If so, create a uio structure with   
                         * only this iovec and pass to uphysio 
			 */


                        uiotmp.uio_iov = iovp;
                        uiotmp.uio_xmem = xmemp;
                        uiotmp.uio_iovcnt = 1;
                        uiotmp.uio_iovdcnt = 0;
                        uiotmp.uio_offset = offset;
                        uiotmp.uio_resid = iovp->iov_len;
                        uiotmp.uio_segflg = uiop->uio_segflg;
                        uiotmp.uio_fmode = uiop->uio_fmode;
                        errnoval = uphysio(&uiotmp,rw_flag,1,
					   diskinfo->devno,
					   scdisk_strategy,
					   scdisk_mincnt,ext);

                        /* 
			 * If problem, return to caller 
			 */
                        if (errnoval != 0) 
			  	break;

                } else {

                        /* 
			 * This request requires special buffering 
			 */
                        errnoval = scdisk_raw_buffer(iovp,xmemp,uiop,
						     offset, rw_flag, 
						     diskinfo, ext);
                        /* 
			 * If problem, return to caller 
			 */
                        if (errnoval != 0) 
				break;

                }

                /* 
		 * Update offset on device, residual count 
		 */
                offset += iovp->iov_len;
   		uiop->uio_offset += iovp->iov_len;
                uiop->uio_resid -= iovp->iov_len;
                iovp->iov_len = 0;
                        
		/* 
		 * Move to the next iovec 
		 */                    
                iovp = (struct iovec *) (((int) iovp) + 
                       sizeof(struct iovec));
                xmemp = (struct xmem *) (((int) xmemp) +
                        sizeof(struct xmem));

        } /* endfor */  

        return(errnoval);

}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_raw_buffer                                       */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine will be called when the device is using	*/
/* 		a block size other then 512, but which is still a 	*/ 
/*		multiple of 512.  It's purpose with the transfer length */
/*              is to deal with requests that are as follows:           */ 
/* 									*/
/*			1. Requests that start on 512 byte boundaries,	*/
/*			   but not on the device's block boundary. 	*/
/*			   For this situation this routine will need 	*/
/*			   to provide buffering to get the request to	*/
/*			   be aligned with the device's block 		*/
/*			   boundaries.					*/
/*									*/
/*			2. Requests that end on a 512 byte boudary, but */
/*			   not on a device block boundary. For this	*/
/*			   situation this routine will need to provide	*/
/*			   buffering to get requests to end on the	*/
/*			   on the device's block boundary.		*/
/*									*/
/*		The middle of the transfer that begins and ends on the  */
/* 		device's block boundary will be processed via uphysio	*/
/*		using our strategy routine.				*/
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              iovp    - Address of a single iovec                     */
/*              xmemp   - Address of xmem descriptor for single iovec   */
/*              uiop    - Address of uio struct describing request      */
/*              offset - Offset in device to read/write location        */
/*              rw_flag - either B_READ or B_WRITE                      */
/*              diskinfo - Address of scdisk_diskinfo struct            */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns the value received from uphysio    */
/*              or a value determined by individual scsi cmds (non-     */
/*              512 block sizes)                                        */ 
/*                                                                      */
/*                                                                      */
/*      ERROR DESCRIPTION:                                              */
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                             - Invalid access authority		*/
/*      		EAGAIN - Pin count exceeded on buffer		*/
/*                                                            		*/
/*                                                                      */
/*                                                                      */
/************************************************************************/
#ifdef _LONG_LONG
int
scdisk_raw_buffer(
struct iovec    *iovp,
struct xmem     *xmemp,
struct uio      *uiop,
offset_t         offset,
int             rw_flag,
struct scdisk_diskinfo  *diskinfo,
int             ext)
#else
int
scdisk_raw_buffer(
struct iovec    *iovp,
struct xmem     *xmemp,
struct uio      *uiop,
off_t           offset,
int             rw_flag,
struct scdisk_diskinfo  *diskinfo,
int             ext)
#endif
{
        int             errnoval;	/* Return code of this routine	*/
#ifdef _LONG_LONG
        offset_t        first_blkno;	/* 1st block number of request	*/
	offset_t	last_blkno;	/* Last block number of request	*/
#else
        off_t           first_blkno;	/* 1st block number of request	*/
	off_t		last_blkno;	/* Last block number of request	*/
#endif
        struct iovec    iovectmp;
        struct uio      uiotmp;         
        int             first_partial_len;/* Partial length at front of	*/
					/* transfer.			*/
	int		last_partial_len;/* Partial length at back of	*/
					/* transfer.			*/


        /*
	 * Determine the first, last blkno to be processed 
	 */
        first_blkno = offset / diskinfo->block_size;
        last_blkno = (offset + iovp->iov_len) / diskinfo->block_size;


        /*
         *
         *
         *  Example of a Transfer that does not start on 
	 *  a block boundary and that does not
	 *  end on a block boundary:
         *
         *  B              B               B              B
         *  |--------------|---------------|--------------|  = Block Boundaries
         *        ^------------------------------^           = data buffer
	 *	  ^--------^				     = first_partial_len
	 *				   ^-----^           = last_partial_len
         *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|XXXXXXXXXXXXXX|  = Blocks needed
	 *
	 *
         *  Example of a Transfer that starts on a block boundary,
	 *  but does not end on a block boundary:
         *
         *  B              B               B              B
         *  |--------------|---------------|--------------|  = Block Boundaries
         *                 ^--------------------------^      = data buffer
	 *				   ^----------^      = last_partial_len
         *  |              |XXXXXXXXXXXXXXX|XXXXXXXXXXXXXX|  = Blocks needed
	 *
	 *
         *  Example of a Transfer that does not start on a block boundary,
	 *  but does end on a block boundary:
         *
         *  B              B               B              B
         *  |--------------|---------------|--------------|  = Block Boundaries
         *      ^--------------------------^      	     = data buffer
	 *      ^----------^                                 = first_partial_len
         *  |XXXXXXXXXXXXXXX|XXXXXXXXXXXXXX|              |  = Blocks needed
	 *
	 *
	 */


        /* 
	 * Determine partials at front and back of the transfer that do not
	 * end up on block boundaries.
	 */
        if (offset % diskinfo->block_size) {
	  
		/*
		 * The start of this request does not begin
		 * on a block boundary.  So compute the size of
		 * transfer up to the first block boundary.
		 */
                first_partial_len = diskinfo->block_size - 
                        (offset % diskinfo->block_size);

                if (first_partial_len > iovp->iov_len) {
		        /*
			 * If this request does not reach to end
			 * of the current device block, then
			 * truncate it down.
			 */
                        first_partial_len = iovp->iov_len;
		}
        } else {

	  	/*
		 * The start of this request does begin
		 * on a block boundary.
		 */
                first_partial_len = 0;
        }


        if ((first_blkno == last_blkno) && (first_partial_len != 0)) {
	        /*
		 * The transfer does not span across multiple blocks
		 */
                last_partial_len = 0;
        } else {
	        /*
		 * Since scdisk_raw called us, we know that the
		 * transfer can't begin and end on block boundaries
		 * So we can only get here if the transfer does
		 * not end on a block boundary.
		 */

                last_partial_len = (offset + iovp->iov_len) % 
                                    diskinfo->block_size;
        }

        /* 
	 * If the first blkno is not a complete block, do buffering 
	 */

        if (first_partial_len > 0) {
                /* 
		 * Request requires buffering at beginning to be on     
                 * a block boundary.                                    
		 */
                errnoval = scdisk_io_buffer(iovp,xmemp,uiop,offset,rw_flag,
					 diskinfo,first_partial_len,TRUE);

                if (errnoval != 0) 
		  	return(errnoval);
                
                /* 
		 * Set new first blkno value 
		 */
                first_blkno++;
        }
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_buff, trc,(char)0, (uint)last_blkno, 
		      (uint)first_blkno,(uint)offset, 
		      (uint)first_partial_len,(uint)last_partial_len);
#endif	
#endif
#endif


        /* 
	 * Now work on intermediate complete blocks 
	 */  
        if (last_blkno > first_blkno) {

                /* 
		 * Process full blocksize requests in middle. 
		 * These begin and end on block boundaries
		 * so we can use uphysio to call our strategy
		 * routine.
		 */
                iovectmp.iov_base = iovp->iov_base + first_partial_len;
                iovectmp.iov_len = iovp->iov_len - first_partial_len -
                                   last_partial_len; 
                uiotmp.uio_iov = &iovectmp;
                uiotmp.uio_xmem = xmemp;
                uiotmp.uio_iovcnt = 1;
                uiotmp.uio_iovdcnt = 0;
                uiotmp.uio_offset = offset + first_partial_len;
                uiotmp.uio_resid = iovectmp.iov_len;
                uiotmp.uio_segflg = uiop->uio_segflg;
                uiotmp.uio_fmode = uiop->uio_fmode;
                errnoval = uphysio(&uiotmp,rw_flag,1,
				   diskinfo->devno,scdisk_strategy,
				   scdisk_mincnt,ext);

                if (errnoval != 0) 
		  	return(errnoval);
        }

        /* 
	 * Now, take care of the last partial block, if any 
	 */
        if (last_partial_len > 0) {

                /* 
		 * Request requires buffering at end to be on   
                 * a block boundary.                            
		 */
                errnoval = scdisk_io_buffer(iovp,xmemp,uiop,offset,rw_flag,
					 diskinfo,last_partial_len,FALSE);

        }

        return(errnoval);
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_io_buffer                                        */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine will fill a partial iovec buffer area.     */
/*              The complete block is first read from the device. Then  */
/*              for writes the user's area will be merged in and the    */
/*              entire block written out. For reads, the partial block  */
/*              area will be written to the user's area.                */
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              iovp    - Address of a single iovec                     */
/*              xmemp   - Address of xmem descriptor for single iovec   */
/*              uiop    - Address of uio struct describing request      */
/*              offset - Offset in device to read/write location        */
/*              rw_flag - either B_READ or B_WRITE                      */
/*              diskinfo - Address of scdisk_diskinfo struct            */
/*              size - Size of partial block area                       */
/*              start_flag - If TRUE, user's partial block area is at   */
/*                      the end of the block, else at the front         */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns the value received from uphysio    */
/*              or a value determined by individual scsi cmds (non-     */
/*              512 block sizes)                                        */ 
/*                                                                      */
/*      ERROR DESCRIPTION:                                              */
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - I/O Failed.				*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Uiomove failed with bad address.	*/
/*                                                                      */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*                                                                      */
/*                                                                      */
/************************************************************************/
#ifdef _LONG_LONG
int
scdisk_io_buffer(
struct iovec    *iovp,			/* Iovec structure to process	*/
struct xmem     *xmemp,			/* Address of xmem for iovp.	*/
struct uio      *uiop,			/* Uio structure containing iovp*/
offset_t        offset,			/* starting byte offset of 	*/
					/* transfer.			*/
int             rw_flag,		/* Read/Write flag.		*/
struct scdisk_diskinfo  *diskinfo,	/* Disk information structure.	*/
int             size,			/* Size in bytes of transfer.	*/
int             start_flag)		/* If true, user's partial block*/
					/* area is at the end of the 	*/
					/* block, else at the front.	*/
#else
int
scdisk_io_buffer(
struct iovec    *iovp,			/* Iovec structure to process	*/
struct xmem     *xmemp,			/* Address of xmem for iovp.	*/
struct uio      *uiop,			/* Uio structure containing iovp*/
off_t           offset,			/* starting byte offset of 	*/
					/* transfer.			*/
int             rw_flag,		/* Read/Write flag.		*/
struct scdisk_diskinfo  *diskinfo,	/* Disk information structure.	*/
int             size,			/* Size in bytes of transfer.	*/
int             start_flag)		/* If true, user's partial block*/
					/* area is at the end of the 	*/
					/* block, else at the front.	*/
#endif
{
        int             errnoval;	/* Return code of this routine	*/
        uchar           *buffer;	/* Padded buffer we use.	*/

        int             full_block_number;/* Starting block number.	*/


        struct dk_cmd   *cmd_ptr;	/* Command pointer for request	*/
        struct iovec    iovectmp;
        struct uio      uiotmp;
        caddr_t         cp;		/* Character pointer.		*/
        struct dk_raw_buf *raw_bp;	/* Buf for raw I/O.		*/
	int 		buffer_size;


#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,io_buf, entry,(char)0, (uint)offset, 
		      (uint)rw_flag,(uint)diskinfo, 
		      (uint)size,(uint)start_flag);
#endif	
#endif
#endif
        /*
	 * Malloc a buffer of size block_size 
	 * NOTE: This buffer must be a multiple of 4k based
	 * on the way DMA works on AIX.
	 */
	buffer_size = diskinfo->block_size/DK_PAGESIZE + DK_PAGESIZE;
        buffer = (uchar *)xmalloc((uint) buffer_size,2,
				   (heapaddr_t) pinned_heap);

        /* 
	 * If unable to malloc, give it up 
	 */
        if (buffer == NULL) {
		return(ENOMEM);

        }

        /* 
	 * Build a raw buf.  A buf is created so that the
	 * existing error recovery path of the device driver for 
	 * read/write commands can be used for these raw I/O requests.
	 */
        raw_bp = (struct dk_raw_buf *) 
			xmalloc((uint) (sizeof(struct dk_raw_buf)),2,
				(heapaddr_t) pinned_heap);


        /* 
	 * If unable to malloc, give it up 
	 */
        if (raw_bp == NULL) {
		
		/* 
		 * Free malloced buffer 
		 */
		xmfree((caddr_t) buffer, (heapaddr_t) pinned_heap); 
		return(ENOMEM);

        }



        /* 
	 * Determine offset of block to read 
	 */
        if (start_flag == TRUE) {
	  	/*
		 * This is the first part of the transfer
		 */
                full_block_number = offset / diskinfo->block_size;
        } else {
	  	/*
		 * This is the last part of the transfer
		 */

                full_block_number = (offset + iovp->iov_len) /
                                    diskinfo->block_size;
        }

        /* 
	 * Build raw command for read operation, add to queue 
	 */
        cmd_ptr = scdisk_build_raw_cmd(diskinfo,B_READ,full_block_number,
				       size,buffer,raw_bp,XMEM_GLOBAL);

        /* 
	 * If unable to alloc a command, give it up 
	 */
        if (cmd_ptr == NULL) {

		/* 
		 * Free raw buf 
		 */
		xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);
		
		/* 
		 * Free malloced buffer 
		 */
		xmfree((caddr_t) buffer, (heapaddr_t) pinned_heap); 

		return(ENOMEM);
        }

	diskinfo->raw_io_intrpt = 1;
        diskinfo->raw_io_cmd = cmd_ptr;
        /* 
	 * Issue the command 
	 */
        scdisk_start_disable(diskinfo); 

        /* 
	 * Sleep until read completes 
	 */
        scdisk_sleep(diskinfo,(uchar *) &(diskinfo->raw_io_intrpt),
		     (uint *) & (raw_bp->bp.b_event));

        /* 
	 * Check to see if request worked 
	 */
        diskinfo->reset_failures = 0;
        errnoval = raw_bp->bp.b_error;
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,io_buf, trc,(char)1, (uint)raw_bp, 
		      (uint)errnoval,(uint)start_flag, 
		      (uint)0,(uint)0);
#endif	
#endif
#endif
        if (errnoval) {
		/* 
		 * Free raw buf 
		 */
		xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);
		
		/* 
		 * Free malloced buffer 
		 */
		xmfree((caddr_t) buffer, (heapaddr_t) pinned_heap); 

		return(errnoval);
	}
        /* 
	 * Process user's area with data in full blocksize 
	 */
        if (rw_flag == B_WRITE) {

                /* 
		 * Merge user's partial buffer with full block 
                 * Build uio structure and use uiomove 
                 * First set determine user's and kernel addresses 
		 */
                if (start_flag) {
                        /* 
			 * Partial block at start of entire request 
			 */
                        iovectmp.iov_base = iovp->iov_base;
                        cp = (caddr_t) ((int)buffer + 
                                (offset % diskinfo->block_size));

                } else {
                        /* 
			 * Partial block at end of entire request 
			 */
                        iovectmp.iov_base = iovp->iov_base + iovp->iov_len -
                                size;
                        cp = (caddr_t) ((int)buffer); 
                }

                uiotmp.uio_iov = &iovectmp;
                uiotmp.uio_xmem = xmemp;
                uiotmp.uio_iovcnt = 1;
                uiotmp.uio_offset = 0;
                uiotmp.uio_resid = size;
                uiotmp.uio_segflg = uiop->uio_segflg;
                uiotmp.uio_fmode = uiop->uio_fmode;
                iovectmp.iov_len = size;

                /* 
		 * Now, issue iomove to move data from user's area. 
		 */
                errnoval = uiomove(cp,size,UIO_WRITE,&uiotmp); 
 
                if (errnoval != 0){
		  	/* 
			 * Free raw buf 
			 */

		  	xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);
		
		  	/* 
		   	 * Free malloced buffer 
		   	 */
			xmfree((caddr_t) buffer, 
			       (heapaddr_t) pinned_heap); 

		  	return(errnoval); 
		} 

                /* 
		 * Build a write command with the full block 
		 */
                cmd_ptr = scdisk_build_raw_cmd(diskinfo,B_WRITE,
                                full_block_number, size, buffer,
                                raw_bp,XMEM_GLOBAL);

                /* 
		 * If unable to alloc command, give it up 
		 */
                if (cmd_ptr == NULL) {

		  	/* 
			 * Free raw buf 
			 */

		  	xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);
		
		  	/* 
		   	 * Free malloced buffer 
		   	 */
			xmfree((caddr_t) buffer, 
			       (heapaddr_t) pinned_heap); 

		  	return(ENOMEM); 
                }

		diskinfo->raw_io_intrpt = 1;
                diskinfo->raw_io_cmd = cmd_ptr;

                /* 
		 * Issue the command 
		 */
                scdisk_start_disable(diskinfo); 

                /* 
		 * Sleep until write completes 
		 */
                scdisk_sleep(diskinfo,(uchar *) &(diskinfo->raw_io_intrpt),
			     (uint *) & (raw_bp->bp.b_event));

                /* 
		 * Check to see if request worked 
		 */
                diskinfo->reset_failures = 0;
                errnoval = raw_bp->bp.b_error;
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,io_buf, trc,(char)2, (uint)raw_bp, 
		      (uint)errnoval,(uint)0, 
		      (uint)0,(uint)0);
#endif	
#endif
#endif
        } else {

                /* 
		 * Copy partial full block to user's area 
		 */

                /* 
		 * Build uio structure and use uiomove 
                 * First set determine user's and kernel addresses 
		 */
                if (start_flag) {
                        /* 
			 * Partial block at start of entire request 
			 */
                        iovectmp.iov_base = iovp->iov_base;
                        cp = (caddr_t) ((int)buffer + 
                                (offset % diskinfo->block_size));
                } else {
                        /* 
			 * Partial block at end of entire request 
			 */
                        iovectmp.iov_base = iovp->iov_base + iovp->iov_len -
                                size;
                        cp = (caddr_t) ((int)buffer); 
                }

                uiotmp.uio_iov = &iovectmp;
                uiotmp.uio_xmem = xmemp;
                uiotmp.uio_iovcnt = 1;
                uiotmp.uio_offset = 0;
                uiotmp.uio_resid = size;
                uiotmp.uio_segflg = uiop->uio_segflg;
                uiotmp.uio_fmode = uiop->uio_fmode;
                iovectmp.iov_len = size;

                /* 
		 * Now, issue iomove to move data to user's area. 
		 */
                errnoval = uiomove(cp,size,UIO_READ,&uiotmp);  
        }



        /* 
	 * Free raw buf 
	 */
        xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);


        /* 
	 * Free malloced buffer 
	 */
        xmfree((caddr_t) buffer, (heapaddr_t) pinned_heap); 

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,io_buf, exit,(char)errnoval, (uint)0, 
		      (uint)0,(uint)0, 
		      (uint)0,(uint)0);
#endif	
#endif
#endif
        return(errnoval);
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_raw_io                                           */
/*                                                                      */
/*      FUNCTION:                                                       */
/*		This routine is used when the device's block size is	*/
/*		is not a multiple of 512.  It will fragment a request	*/
/* 		which is greater then our maximum transfer size into 	*/
/*		requests that are no bigger then our maximum transfer 	*/
/*		size.  It will setup the correct cross memory 		*/
/*		descriptors and pin the user's data area.  It will then */
/*		build and issue the appropriate read or write SCSI      */
/*		command for each of these smaller requests, with out 	*/
/*		using uphysio. 						*/
/*									*/
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/*      NOTES:                                                          */
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              iovp    - Address of a single iovec                     */
/*              uiop    - Address of uio struct describing request      */
/*              offset - Offset in device to read/write location        */
/*              rw_flag - either B_READ or B_WRITE                      */
/*              diskinfo - Address of scdisk_diskinfo struct            */
/*              ext     - extended parameter                            */
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns the value determined by individual */
/*		scsi cmds.                                              */ 
/*                                                                      */
/*      ERROR DESCRIPTION:                                              */
/*              The following values may be returned:                   */
/*			EINVAL - length of request is not a multiple of */
/*			         the device's block size.		*/
/*			ENXIO  - Block number is beyond end of media.	*/
/*			EIO    - The device is offline.			*/
/*			       - I/O Failed.				*/
/*			ENOMEM - Insufficient Memory.			*/
/*			EFAULT - Pinning of user's buffer failed.	*/
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*                                                                      */
/*                                                                      */
/************************************************************************/

#ifdef _LONG_LONG
int
scdisk_raw_io(
struct iovec    *iovp,			/* Iovec structure to process	*/
struct uio      *uiop,			/* Uio structure containing iovp*/
offset_t        offset,			/* starting byte offset of 	*/
					/* transfer.			*/
int             rw_flag,		/* Read/Write flag.		*/
struct scdisk_diskinfo  *diskinfo,	/* Disk information structure.	*/
int             ext)			/* Extended parameter	        */
#else
int
scdisk_raw_io(
struct iovec    *iovp,			/* Iovec structure to process	*/
struct uio      *uiop,			/* Uio structure containing iovp*/
off_t           offset,			/* starting byte offset of 	*/
					/* transfer.			*/
int             rw_flag,		/* Read/Write flag.		*/
struct scdisk_diskinfo  *diskinfo,	/* Disk information structure.	*/
int             ext)			/* Extended parameter	        */
#endif
{
        int             errnoval;	/* return code of this routine.	*/
	int		transfer_length;/* Fragmented transfer length.	*/
        int             blkno;		/* Starting block number of a	*/
					/* fragmented request.		*/
	int		last_blk;	/* Last block number on device.	*/
        struct dk_cmd   *cmd_ptr;	/* command pointer for each	*/
					/* fragmented transfer.		*/
        struct dk_raw_buf *raw_bp;	/* Buf for raw I/O transfer.	*/
	caddr_t         buffer_addr;	/* Starting addres of buffer.	*/
	int		xfer_size;	/* Transfer size left to process*/
	int		xfer_resid;	/* Resid still remaining.	*/
	struct scdisk_stats     *stats; /* Statistics info pointer	*/


	if (offset % diskinfo->block_size) {
		/* 
		 * If offset is not on block boundary
		 * then fail request.
		 */
		return (EINVAL);
	}

	/*
	 * Compute starting block number of request.
	 */

	blkno = offset / diskinfo->block_size;
	last_blk = diskinfo->capacity.lba;

	/*
         * Perform the same checks that our strategy
	 * does for the block interface except to do not
	 * check if transfer size exceeds max_request since
	 * this routine will do the necessary fragmentation.  Also
	 * do not check if in diagnostic mode since our parent routine
	 * does that.
	 * NOTE: iovp already has the resid set to the to iovp_len
	 * so it does not need to be set here to iov_len when
	 * we fail.
	 */

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_io,entry,(char)0, (uint) blkno, (uint)ext, 
		      (uint)last_blk,(uint)rw_flag, (uint)offset);
   scdisk_trc_disable(diskinfo,raw_io,entry,(char)0, 
		      (uint) diskinfo->block_size, 
		      (uint)diskinfo->state, (uint)iovp->iov_len,
		      (uint)diskinfo->starting_close, (uint)0);

#endif	
#endif
#endif
	if (blkno > (last_blk+1)) {
	  	/*
		 * Verify block address on device
		 */
	  	return(ENXIO);
	}
	else if (blkno == (last_blk+1)) {

	  	if (rw_flag == B_READ) {
	  		return(ENXIO);
	  	}
		return(0);
	}
        else if (iovp->iov_len == 0) {
	  	/*
		 * Check for zero length read or write
		 */
	  	return (0);
        }
	else if ((diskinfo->state == DK_OFFLINE) ||
		 (diskinfo->starting_close)) {
	  	/*
                 * Verify state of device.
                 */
		return (EIO);
        }
	else if ((int)(iovp->iov_len % diskinfo->block_size) != 0) {
	  	/*
		 * Fail any request that whose length is not
		 * a multiple of our block size.
		 */

	  	return (EINVAL);
	}


	stats = &(diskinfo->stats);

	/*
	 * If we made it here then it should be a valid request
	 */

        /* 
	 * Get user buffer address and length of this transfer.           
	 */

        /* 
	 * Build a raw buf.  A buf is created so that the
	 * existing error recovery path of the device driver for 
	 * read/write commands can be used for these raw I/O requests.
	 */
        raw_bp = (struct dk_raw_buf *) 
			xmalloc((uint) (sizeof(struct dk_raw_buf)),2,
				(heapaddr_t) pinned_heap);

        /* 
	 * If unable to malloc, give it up 
	 */
        if (raw_bp == NULL) {
		return(ENOMEM);

        }

	/*
	 * Point to user's address space.
	 */

	buffer_addr = iovp->iov_base;

	raw_bp->bp.b_xmemd.aspace_id = XMEM_INVAL;

        /* 
         * Call xmattach  to obtain addressability to the user buffer.       
	 */
        errnoval = xmattach(buffer_addr, iovp->iov_len,&(raw_bp->bp.b_xmemd),
                     uiop->uio_segflg);
        if (errnoval != XMEM_SUCC) {
	  	xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);
		return (EFAULT);
        }
 

	xfer_size = iovp->iov_len;
	xfer_resid = uiop->uio_resid;

#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_io,trc,(char)1, (uint) xfer_size, 
		      (uint)xfer_resid, 
		      (uint)diskinfo->max_request,(uint)raw_bp, 
		      (uint)buffer_addr);
#endif	
#endif
#endif
        while (xfer_size) {          
          
		/* 
		 * Loop until entire tranfer is complete. 
		 */

	  	if (xfer_size > diskinfo->max_request) {
		  	transfer_length = diskinfo->max_request -
			  	(diskinfo->max_request % diskinfo->block_size);
		}
		else {
		  	transfer_length = xfer_size;
		}
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_io,trc,(char)2, (uint) xfer_size, 
		      (uint)xfer_resid, 
		      (uint)transfer_length,(uint)blkno, 
		      (uint)0);
#endif	
#endif
#endif

		/* 
		 * Pin user buffer.  
		 * NOTE: scdisk_pin_buffer may reduce the transfer_length,
		 * it may not be able to pin the original transfer.
		 */
		errnoval = scdisk_pin_buffer(buffer_addr,
				       &transfer_length,uiop->uio_segflg, 
				       diskinfo->block_size);

		if (errnoval != 0) {
		  	uiop->uio_resid = xfer_resid;
			iovp->iov_len = xfer_size;
			break;
		}


		/* 
		 * Build a write command with the full block 
		 */
		cmd_ptr = scdisk_build_raw_cmd(diskinfo,rw_flag,
					       blkno, transfer_length, 
					       buffer_addr,
					       raw_bp,
					       raw_bp->bp.b_xmemd.aspace_id);
		/* 
		 * If unable to alloc a command, give it up 
		 */
		if (cmd_ptr == NULL) {
		  	errnoval = ENOMEM;
		  	uiop->uio_resid = xfer_resid;
			iovp->iov_len = xfer_size;
			/* 
			 *  Unpin user buffer.
			 */

			unpinu(buffer_addr,transfer_length,uiop->uio_segflg);

			break;
		}

		diskinfo->raw_io_intrpt = 1;
		diskinfo->raw_io_cmd = cmd_ptr;

		/* 
		 * Issue  the command 
		 */
		scdisk_start_disable(diskinfo); 

		/* 
		 * Sleep until command completes 
		 */
		scdisk_sleep(diskinfo,(uchar *) &(diskinfo->raw_io_intrpt),
			     (uint *) & (raw_bp->bp.b_event));

		/* 
		 * Check to see if request worked 
		 */
		diskinfo->reset_failures = 0;

		/* 
		 *  Unpin user buffer.
		 */

		unpinu(buffer_addr,transfer_length,uiop->uio_segflg);


		errnoval = raw_bp->bp.b_error;

		if (errnoval) {
		  	uiop->uio_resid = xfer_resid;
			iovp->iov_len = xfer_size;
		  	break;
		}


		/* 
		 * Update transfer statistics for device 
		 */
		if ((rw_flag & B_READ) &&
		    ((stats->byte_count += transfer_length) >
                                     stats->segment_size)) {

		  	stats->segment_count++;
			stats->byte_count %=   stats->segment_size;
		}


		/*
     		 * Update tranfer information
		 */
		xfer_size -= transfer_length;
		xfer_resid -= transfer_length;
		/*
		 * Get block number for next transfer
		 */
		blkno += transfer_length/diskinfo->block_size;
		buffer_addr = (caddr_t) (((int) buffer_addr) + transfer_length);

        }

	(void) xmdetach(&raw_bp->bp.b_xmemd);
	xmfree((caddr_t) raw_bp, (heapaddr_t) pinned_heap);


#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_io,exit,(char)0, (uint) xfer_size, 
		      (uint)xfer_resid, 
		      (uint)errnoval,(uint)blkno, 
		      (uint)0);
#endif	
#endif
#endif
 
        return(errnoval);
}

/************************************************************************/
/*                                                                      */
/*      NAME:   scdisk_build_raw_cmd                                    */
/*                                                                      */
/*      FUNCTION:                                                       */
/*              This routine will build a command which will either     */
/*              do a raw extended read or extended write operation      */
/*              The built command will be pointed to by 		*/
/*		diskinfo->raw_io_cmd.     				*/
/*                                                                      */
/*      EXECUTION ENVIRONMENT:                                          */
/*              This routine can be called by a process.                */
/*              It can page fault only if called under a process and    */
/*              the stack is not pinned.                                */
/*                                                                      */
/*      NOTES:                                                          */
/*		This routine builds the following SCSI commands:	*/
/*									*/
/*                        READ(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (28h)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*									*/
/*                        WRITE(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (2ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*                                                                      */
/*      DATA STRUCTURES:                                                */
/*                                                                      */
/*      INPUTS:                                                         */
/*              diskinfo - Address of scdisk_diskinfo struct            */
/*              rw_flag - either B_READ or B_WRITE                      */
/*              full_block_number - read/write location                 */
/*              size - Size of partial block area                       */
/*              bptr - Address of blocksize buffer area                 */    
/*                                                                      */
/*      RETURN VALUE DESCRIPTION:                                       */
/*              This routine returns a pointer to the command built.    */
/*              Value will be NULL if the command could not be          */
/*              allocated.                                              */
/*                                                                      */
/*      ERROR DESCRIPTION:                                              */
/*              See above.                                              */
/*                                                                      */
/*      EXTERNAL PROCEDURES CALLED:                                     */
/*                                                                      */
/*                                                                      */
/************************************************************************/


struct dk_cmd *
scdisk_build_raw_cmd(
struct scdisk_diskinfo  *diskinfo,	/* Disk information structure	*/
int                     rw_flag,	/* Read/Write flage 		*/
int                     block_number,	/* Starting block number.	*/
int                     size,		/* size of transfer in bytes.	*/
char                    *buffer,	/* Buffer for data transfer.	*/
struct dk_raw_buf       *raw_bp,	/* Buff associated with this 	*/
int			aspace)	        /* Address xmem space info.	*/

{

        struct dk_cmd           *cmd_ptr;	/* command pointer 	*/
        struct buf              *cmd_bp;	/* Pointer to buf	*/
						/* of cmd_ptr.		*/
        int                     blk_cnt;	/* Size of transfer in	*/
						/* blocks.		*/
        struct scsi             *scsi;		/* SCSI command block	*/
	int			actual_size;	/* Actual size of 	*/
						/* request including any*/
						/* buffering		*/


	if (size % diskinfo->block_size) {
	  	/*
		 * If the size of this transfer is not a multiple
		 * of our block size then pad out to the end
		 * of the last block.
		 */
		actual_size = size/diskinfo->block_size + diskinfo->block_size;
	}
	else {
		/*
		 * If the size of this transfer is a multiple
		 * of our block size, then use it as is.
		 */
	  	actual_size = size;
	}
	if (actual_size > diskinfo->max_request) {
	  	/*
		 * Transfer size exceeds our maximum
		 * transfer size.
		 */
	  	return (NULL);
	}


	if ((block_number == DK_CD_PVD) &&			
	    (diskinfo->last_ses_pvd_lba) &&		
	    (diskinfo->block_size == DK_M2F1_BLKSIZE)) {  
		/*					
		 * If this is mult-session photo CD	
		 * and the request is to lba 16		
		 * (pvd) and we are running in M2F1	
		 * (the only valid mode for photo CD),	
		 * then remap it to the			
		 * last session's pvd.			
		 */					
		block_number = diskinfo->last_ses_pvd_lba;     
	}

        /* 
	 * Build a bp which is for this single block operation 
	 */
        raw_bp->bp.b_dev = diskinfo->devno;
#ifdef _POWER_MP
        raw_bp->bp.b_flags = rw_flag | B_BUSY|B_MPSAFE;
#else
        raw_bp->bp.b_flags = rw_flag | B_BUSY;
#endif
        raw_bp->bp.b_options = 0;
        raw_bp->bp.b_work = 0x00;
        raw_bp->bp.b_error = 0;
        raw_bp->bp.b_blkno = block_number;
        raw_bp->bp.b_bcount = actual_size;
        raw_bp->bp.b_un.b_addr = (caddr_t) buffer;
        raw_bp->bp.b_xmemd.aspace_id = aspace;
        raw_bp->bp.b_iodone = ((void(*)())scdisk_raw_iodone);
        raw_bp->bp.b_event = EVENT_NULL;
        raw_bp->bp.b_forw = NULL;
        raw_bp->bp.b_back = NULL;
        raw_bp->bp.av_forw = NULL;
        raw_bp->bp.av_back = NULL;
	raw_bp->diskinfo = diskinfo;

        /* 
	 * Allocate a command structure for this command 
	 */
        cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_BUF);

        if (cmd_ptr == NULL) 
	  	return(cmd_ptr);

	/* 
	 * increment transfer count 
	 */
	diskinfo->dkstat.dk_xfers++;


        /* 
	 * Initialize command block for requested operation 
	 */
        cmd_bp = &(cmd_ptr->scbuf.bufstruct);
        bcopy( raw_bp, cmd_bp, sizeof(struct buf));
        cmd_bp->b_dev = diskinfo->adapter_devno;
        cmd_bp->b_iodone = ((void(*)())scdisk_iodone);

        /* 
	 * Fill out scbuf structure 
	 */
        cmd_ptr->scbuf.timeout_value = diskinfo->rw_timeout;
        cmd_ptr->scbuf.bp = NULL;
        cmd_ptr->scbuf.status_validity = 0x00;
        cmd_ptr->scbuf.flags = SC_RESUME | diskinfo->reset_delay;
	cmd_ptr->scbuf.q_tag_msg = diskinfo->q_type;


        /* 
	 * Setup rest of cmd_ptr structure 
	 */
        cmd_ptr->retry_count = 0;
        cmd_ptr->segment_count = 0x00;
        cmd_ptr->soft_resid = 0x00;
        cmd_ptr->group_type = DK_SINGLE;
        cmd_ptr->bp = (struct buf *)raw_bp;
        cmd_ptr->intrpt = 1;
        cmd_ptr->flags |= DK_READY;

        /* 
	 * Initialize SCSI cmd for operation 
	 */
        blk_cnt = actual_size/diskinfo->block_size;

        scsi = &(cmd_ptr->scbuf.scsi_command);
                       
        /* 
	 * Use a 10 byte command. 
	 */
        scsi->scsi_length = 10;
        scsi->scsi_id = diskinfo->scsi_id;
        scsi->flags = (ushort) diskinfo->async_flag;
        if (rw_flag == B_READ)
              scsi->scsi_cmd.scsi_op_code = SCSI_READ_EXTENDED;
        else
              scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_EXTENDED;


        scsi->scsi_cmd.scsi_bytes[0] = ((block_number >> 24) & 0xff);
        scsi->scsi_cmd.scsi_bytes[1] = ((block_number >> 16) & 0xff);
        scsi->scsi_cmd.scsi_bytes[2] = ((block_number >> 8) & 0xff);
        scsi->scsi_cmd.scsi_bytes[3] = (block_number & 0xff);
        scsi->scsi_cmd.scsi_bytes[4] = 0x00;
        scsi->scsi_cmd.scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
        scsi->scsi_cmd.scsi_bytes[6] = (blk_cnt & 0xff);
        scsi->scsi_cmd.scsi_bytes[7] = 0x00;
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,raw_bld,exit,(char)0, (uint)blk_cnt, 
		      (uint)block_number, 
		      (uint)scsi->scsi_cmd.scsi_op_code,
		      (uint)actual_size, (uint)cmd_ptr);
#endif	
#endif
#endif
        return(cmd_ptr);
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_ioctl						*/
/*									*/
/*	FUNCTION: SCSI Disk Device Driver IOCTL Routine.		*/
/*		This routine supports the following ioctl commands:	*/
/*		IOCINFO	  - Returns info about the device such as block */
/*			    size and number of blocks.			*/
/*		DKIORDSE  - Issues a read cmd and returns sense data	*/
/*			    if an error occurs				*/
/*		DKIOWRSE  - Issues a read cmd and returns sense data	*/
/*			    if an error occurs				*/
/*		DKIOCMD   - Issues the scsi cmd described in the arg	*/
/*			    and returns the cmd status.			*/
/*		DKPMR	  - Issues the SCSIprevent media removal command*/
/*		DKAMR	  - Issues the SCSI allow media removal command */
/*		DKEJECT   - Issues the eject media command		*/
/*		DKFORMAT  - Issues format unit command			*/
/*		DKAUDIO   - Issues Play Audio Commands.			*/
/*		DK_CD_MODE- Used for determining and/or changing the 	*/
/*			    CD-ROM Data Mode.				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		scdisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		op	- operation to be performed			*/
/*		arg	- address of user argument structure		*/
/*		flag	- File parameter word				*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		Otherwise a value of zero will be returned to indicate	*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- operation failed				*/
/*	    		- device not open				*/
/*			- copyin failed					*/
/*		EINVAL	- Invalid parameter in request			*/
/*			- device is a SCSI Disk when it shouldn't be    */
/*			- Transfer size is too large		        */
/*		ENXIO   - Device not open				*/
/*		EACCES	- Device not open in SC_SINGLE mode		*/
/*		        - Device not open in diagnostic mode		*/
/*		EFAULT  - Xmattach failed				*/
/*		EWRPROTECT - Device is write protected			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		copyout							*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_ioctl(
dev_t	devno,
int	op, 
int	arg, 
int	devflag, 
int	chan, 
int	ext)
{
	int			errnoval, dev, hash_key;
	struct scdisk_diskinfo	*diskinfo;



	DDHKWD5(HKWD_DD_SCDISKDD, DD_ENTRY_IOCTL, 0, devno, op, arg, 0, 0);

	errnoval = 0;

	/* 
	 * Lock the scsi disk common lock 
	 */
	(void) lockl(&(scdisk_info.lock), LOCK_SHORT);

	/* 
	 * Search for diskinfo structure in open list 
	 */
	dev = minor(devno);
	hash_key = dev & DK_HASH;
	diskinfo = scdisk_open_list[hash_key];
	while (diskinfo != NULL) {
		if (diskinfo->devno == devno)
			break;
		diskinfo = diskinfo->next_open;
	}

	if (diskinfo == NULL) {
		unlockl(&(scdisk_info.lock));
		DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_IOCTL, ENXIO, devno);
		return(ENXIO);
	}
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,ioctls, trc,(char)0, (uint)devno, (uint)op,
		      (uint)arg, 
		      (uint)devflag,(uint)chan);
#endif	
#endif
#endif
	/* 
	 * Lock the device's lock 
	 */
	(void) lockl(&(diskinfo->lock), LOCK_SHORT);

	/* 
	 * Unlock the scsi disk common lock 
	 */
	unlockl(&(scdisk_info.lock));

	/* 
	 * Determine operation to perform 
	 */
	switch (op) {
	case IOCINFO:
		{

			struct devinfo  devinfo;
			
			if (diskinfo->dev_type == DK_CDROM) {
				devinfo.devtype = DD_SCCD;
                                devinfo.flags = (uchar) DF_RAND;
                                devinfo.devsubtype = 0x00;
                                devinfo.un.sccd.blksize = diskinfo->block_size;
                                devinfo.un.sccd.numblks = 
                                        diskinfo->capacity.lba + 1;
			}
			else {
				if (diskinfo->dev_type == DK_RWOPTICAL) {
					devinfo.devtype = DD_SCRWOPT;
					devinfo.flags = (uchar)DF_RAND;
				}
				else {
					devinfo.devtype = DD_SCDISK;
					devinfo.flags = (uchar) 
						(DF_RAND | DF_FAST | DF_FIXED);
				}
				
				devinfo.devsubtype = (uchar) DS_PV;
				devinfo.un.scdk.blksize = diskinfo->block_size;
				devinfo.un.scdk.numblks = 
					             diskinfo->capacity.lba + 1;
				devinfo.un.scdk.max_request = 
					diskinfo->max_request;
				devinfo.un.scdk.segment_size = 
					diskinfo->stats.segment_size;
				devinfo.un.scdk.segment_count = 
					diskinfo->stats.segment_count;
				devinfo.un.scdk.byte_count = 
					diskinfo->stats.byte_count;
			}
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &devinfo, (caddr_t) arg,
				    sizeof(struct devinfo));
			} else {
				copyout(&devinfo, arg, sizeof(struct devinfo));
			}
			break;
		}
	case DKIORDSE:
		        errnoval = scdisk_rdwrse(op, arg, devflag, diskinfo);
			break;
	case DKIOWRSE:
		        if (diskinfo->dev_type != DK_CDROM) {
				errnoval = scdisk_rdwrse(op, arg, 
							 devflag, diskinfo);
				break;
			}
			/*
			 * If CDROM then this must be CDIOCMD
			 * so drop down to DKIOCMD.  The CDICMD define could
			 * not be changed to DKIOCMD, because binary 
			 * compatibility with AIX 3.2 must be maintained
			 */

	case DKIOCMD:
		errnoval = scdisk_dkiocmd(arg, devflag, diskinfo);
		break;

	case DKPMR:
	case DKAMR:
		errnoval = scdisk_dk_amr_pmr(op, diskinfo);
		break;
	case DKEJECT:
		errnoval = scdisk_dkeject(diskinfo);
		break;
	case DKFORMAT:
		errnoval = scdisk_dkformat(arg, devflag, diskinfo);
		break;
	case DKAUDIO:
		errnoval = scdisk_dkaudio(arg,devflag,diskinfo);
		break;
	case DK_CD_MODE:
		errnoval = scdisk_dk_cd_mode(arg,devflag,diskinfo);
		break;

	default:
		errnoval = EINVAL;
	}
#ifdef DEBUG
#ifdef SC_GOOD_LOCK_PATH
#ifdef SC_GOOD_PATH
   scdisk_trc_disable(diskinfo,ioctls, trc,(char)1, (uint)devno, 
		      (uint)op,(uint)arg, 
		      (uint)devflag,(uint)errnoval);
#endif	
#endif
#endif


	unlockl(&(diskinfo->lock));
	DDHKWD1(HKWD_DD_SCDISKDD, DD_EXIT_IOCTL, errnoval, devno);
	return(errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_rdwrse						*/
/*									*/
/*	FUNCTION: Read and Write with Sense Subroutine.			*/
/*		This routine supports the following ioctl commands:	*/
/*		DKIORDSE - Issues a read cmd and returns sense data	*/
/*			   if an error occurs				*/
/*		DKIOWRSE - Issues a read cmd and returns sense data	*/
/*			   if an error occurs				*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*		This routine will build the following SCSI commands:    */
/*									*/
/*                        READ(6) Command        			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (08h)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | (MSB)                            |	*/
/*  |-----+-----------------------                             ---|	*/
/*  | 2   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 4   |                    Transfer Length                    |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 5   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        READ(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (28h)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        WRITE(6) Command        			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (0Ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | (MSB)                            |	*/
/*  |-----+-----------------------                             ---|	*/
/*  | 2   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 4   |                    Transfer Length                    |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 5   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*                        WRITE(10) Command         			*/
/*  +=====-======-======-======-======-======-======-======-======+	*/
/*  |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/*  |Byte |      |      |      |      |      |      |      |      |	*/
/*  |=====+=======================================================|	*/
/*  | 0   |                Operation Code (2ah)                   |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 1   |Logical Unit Number | DPO  | FUA  | Reserved    |RelAdr|	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 2   | (MSB)                                                 |	*/
/*  |-----+---						       ---|	*/
/*  | 3   |             Logical Block Address                     |	*/
/*  |-----+---						       ---|	*/
/*  | 4   |                                                       |	*/
/*  |-----+---                                                 ---|	*/
/*  | 5   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 6   |                    Reserved                           |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 7   | (MSB)              Transfer Length                    |	*/
/*  |-----+---                                                 ---|	*/
/*  | 8   |                                                 (LSB) |	*/
/*  |-----+-------------------------------------------------------|	*/
/*  | 9   |                    Control                            |	*/
/*  +=============================================================+	*/
/*									*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		scdisk_open_list List of open devices			*/
/*									*/
/*	INPUTS:								*/
/*		devno	- device major/minor number			*/
/*		op	- operation to be performed			*/
/*		arg	- address of user argument structure		*/
/*		flag	- File parameter word				*/
/*		chan	- unused (will be zero)				*/
/*		ext	- extended parameter				*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*			- operation failed				*/
/*		EINVAL	- Invalid parameter in request			*/
/*			- transfer length too large			*/
/*		EWRPROTECT - Device is write protected			*/
/*		EFAULT	   - Xmattach failed				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		lockl		unlockl					*/
/*		copyin		copyout					*/
/*		pinu		unpinu					*/
/*		xmattach	xmdetach				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_rdwrse(
int			op, 
int			arg, 
int			devflag, 
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval, rc, lba, blk_cnt, segflag;
	struct dk_cmd	*cmd_ptr;
	struct sc_rdwrt	dk_rdwrt;
	struct sc_buf	*scbuf;
	struct buf	*bp;
	struct scsi	*scsi;


	if ((op == DKIOWRSE) && (diskinfo->wprotected)) {
		/*
		 * Fail if this is a write with sense to write
		 * protected media.
		 */
		return (EWRPROTECT);
	}


	errnoval = 0;

	/*
	 *  Set up segflag for calls to pinu and xmattach and copy in the
	 *  user's structure.
	 */
	
	if (devflag & DKERNEL) {
		segflag = SYS_ADSPACE;
		bcopy((caddr_t) arg, (caddr_t) &dk_rdwrt,
		    sizeof(struct sc_rdwrt));
	} else {
		segflag = USER_ADSPACE;
		copyin(arg, &dk_rdwrt, sizeof(struct sc_rdwrt));
	}

	/* Validate request */
	if ((dk_rdwrt.data_length & 0x01ff) ||
	    (dk_rdwrt.data_length > diskinfo->max_request)) {
		dk_rdwrt.status_validity = 0x00;
		if (devflag & DKERNEL) {
			bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg,
			    sizeof(struct sc_rdwrt));
		} else {
			copyout(&dk_rdwrt, arg, sizeof(struct sc_rdwrt));
		}
		return(EINVAL);
	}

	/* Allocate an IOCTL cmd block for operation */

	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
	ASSERT(cmd_ptr != NULL);
	scbuf = &(cmd_ptr->scbuf);
	scsi = &(scbuf->scsi_command);
	bp = &(scbuf->bufstruct);

	/* Initialize the scbuf for the operation */
	scbuf->bp = NULL;
	scbuf->status_validity = 0x00;
	scbuf->timeout_value = dk_rdwrt.timeout_value;
	scbuf->flags = SC_RESUME | diskinfo->reset_delay| dk_rdwrt.q_flags;
	scbuf->q_tag_msg = dk_rdwrt.q_tag_msg;
	/* Initialize scbuf.bufstruct for the operation */
	bp->b_dev = diskinfo->adapter_devno;
	bp->b_flags = B_BUSY;
	if (op == DKIORDSE) {
		bp->b_flags |= B_READ;
	}
	bp->b_bcount = (uint) dk_rdwrt.data_length;
	bp->b_work = 0x00;
	bp->b_error = 0x00;
	bp->b_un.b_addr = dk_rdwrt.buffer;
	bp->b_blkno = dk_rdwrt.logical_blk_addr;
	bp->b_iodone = scdisk_iodone;
	bp->b_xmemd.aspace_id = XMEM_INVAL;
	bp->b_event = EVENT_NULL;

	/* Initialize SCSI cmd for operation */
	/*
	 * The blocksize must be hard-coded here because if
	 * the device was opened in diag mode, the capacity
	 * data will be invalid.
	 */
	blk_cnt = bp->b_bcount / diskinfo->block_size;
	lba = bp->b_blkno;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	if ((blk_cnt & 0xffffff00) || (lba & 0xffe00000)) {
		scsi->scsi_length = 10;
		if (op == DKIORDSE) {
			scsi->scsi_cmd.scsi_op_code = SCSI_READ_EXTENDED;
		} else {
			scsi->scsi_cmd.scsi_op_code = SCSI_WRITE_EXTENDED;
		}

		scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 24) & 0xff);
		scsi->scsi_cmd.scsi_bytes[1] = ((lba >> 16) & 0xff);
		scsi->scsi_cmd.scsi_bytes[2] = ((lba >> 8) & 0xff);
		scsi->scsi_cmd.scsi_bytes[3] = (lba & 0xff);
		scsi->scsi_cmd.scsi_bytes[4] = 0x00;
		scsi->scsi_cmd.scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
		scsi->scsi_cmd.scsi_bytes[6] = (blk_cnt & 0xff);
		scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	} else {
		scsi->scsi_length = 0x06;
		if (op == DKIORDSE) {
			scsi->scsi_cmd.scsi_op_code=SCSI_READ;
		} else {
			scsi->scsi_cmd.scsi_op_code=SCSI_WRITE;
		}
		scsi->scsi_cmd.lun |= ((lba >> 16) & 0x1f);
		scsi->scsi_cmd.scsi_bytes[0] = ((lba >> 8) & 0xff);
		scsi->scsi_cmd.scsi_bytes[1] = (lba & 0xff);
		scsi->scsi_cmd.scsi_bytes[2] = blk_cnt;
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	}
	/*
	 * Attach and pin user memory (buffer and req. sense 
	 * buffer) for transfer from bottom half routines.
	 */
	if (bp->b_bcount != 0) {

		rc = xmattach((uint)bp->b_un.b_addr, (int) bp->b_bcount,
		    &(bp->b_xmemd), (int) segflag);
		if (rc != XMEM_SUCC) {
			scdisk_free_cmd_disable(cmd_ptr);
			dk_rdwrt.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg, 
				    sizeof (struct sc_rdwrt));
			} else {
				copyout((caddr_t) &dk_rdwrt, (caddr_t) arg,
				    sizeof (struct sc_rdwrt));
			}
			return(EFAULT);
		}
		rc = pinu((caddr_t) bp->b_un.b_addr, (int) bp->b_bcount,
		    (short) segflag);
		if (rc != PIN_SUCC) {
			xmdetach(&(bp->b_xmemd));
			scdisk_free_cmd_disable(cmd_ptr);
			dk_rdwrt.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg, 
				    sizeof (struct sc_rdwrt));
			} else {
				copyout((caddr_t) &dk_rdwrt, (caddr_t) arg,
				    sizeof (struct sc_rdwrt));
			}
			return(rc);
		}
	}
	diskinfo->ioctl_req_sense.xmemd.aspace_id = XMEM_INVAL;
	if (dk_rdwrt.req_sense_length != 0) {
		rc = xmattach((uint) dk_rdwrt.request_sense_ptr,
		    (int)dk_rdwrt.req_sense_length,
		    &(diskinfo->ioctl_req_sense.xmemd), (int) segflag);
		if (rc != XMEM_SUCC) {
			if (bp->b_bcount != 0) {
				(void) unpinu((caddr_t) bp->b_un.b_addr,
				    (int) bp->b_bcount, (short) segflag);
				xmdetach(&(bp->b_xmemd));
			}
			scdisk_free_cmd_disable(cmd_ptr);
			dk_rdwrt.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg, 
				    sizeof (struct sc_rdwrt));
			} else {
				copyout(&dk_rdwrt, arg, sizeof
				    (struct sc_rdwrt));
			}
			return((int) EFAULT);
		}
		rc = pinu((caddr_t) dk_rdwrt.request_sense_ptr,
		    (int) dk_rdwrt.req_sense_length, (short) segflag);
		if (rc != PIN_SUCC) {
			xmdetach(&(diskinfo-> ioctl_req_sense.xmemd));
			if (bp->b_bcount != 0) {
				(void) unpinu((caddr_t) bp->b_un.b_addr,
				    (int) bp->b_bcount, (short) segflag);
				xmdetach(&(bp->b_xmemd));
			}
			scdisk_free_cmd_disable(cmd_ptr);
			dk_rdwrt.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg, 
				    sizeof (struct sc_rdwrt));
			} else {
				copyout(&dk_rdwrt, arg, sizeof
				    (struct sc_rdwrt));
			}
			return(rc);
		}
	}
	/*
	 * Set up for xmemout of request sense data in 
	 * bottom half.
	 */
	diskinfo->ioctl_req_sense.count = dk_rdwrt.req_sense_length;
	diskinfo->ioctl_req_sense.buffer = dk_rdwrt.request_sense_ptr;

	/* Start execution of operation */

	cmd_ptr->subtype = DK_RDWR_SENSE;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;

	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);

	/* Wait for completion */

	scdisk_sleep(diskinfo,
		     (uchar *)&(cmd_ptr->intrpt),(uint *)&(bp->b_event));	

	/* detach and unpin user memory */
	if (bp->b_bcount != 0) {
		(void) unpinu((caddr_t) bp->b_un.b_addr, (int) bp->b_bcount,
		    (short) segflag);
		xmdetach(&(bp->b_xmemd));
	}
	if (dk_rdwrt.req_sense_length != 0) {
		(void) unpinu((caddr_t) dk_rdwrt.request_sense_ptr,
		    (int) dk_rdwrt.req_sense_length, (short) segflag);
		xmdetach(&(diskinfo->ioctl_req_sense.xmemd));
	}

	/* Check for IO error to return sense info */
	errnoval = bp->b_error;
	dk_rdwrt.status_validity = scbuf->status_validity;
	dk_rdwrt.scsi_bus_status = scbuf->scsi_status;
	dk_rdwrt.adapter_status = scbuf->general_card_status;
	dk_rdwrt.adap_q_status = scbuf->adap_q_status;
	/*
	 * For the special case where a resid is returned
	 * from the SCSI adapter driver, but no error is
	 * set, set an adapter hardware failure.
	 */
	if ((errnoval == EIO) && (dk_rdwrt.status_validity == 0x00)) {
		dk_rdwrt.status_validity = SC_ADAPTER_ERROR;
		dk_rdwrt.adapter_status = SC_ADAPTER_HDW_FAILURE;
	}
	/* Deallocate cmd block from operation */
	scdisk_free_cmd_disable(cmd_ptr);

	/* Return arg stucture to user */
	if (devflag & DKERNEL) {
		bcopy((caddr_t) &dk_rdwrt, (caddr_t) arg,
		    sizeof(struct sc_rdwrt));
	} else {
		copyout(&dk_rdwrt,arg,sizeof(struct sc_rdwrt));
	}
	return(errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_dkiocmd						*/
/*									*/
/*	FUNCTION: Issues a pass through SCSI command to the device      */
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*		Requires Diagnostic open				*/ 
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg	 - address of user argument structure		*/
/*		devflag	 - File parameter word				*/
/*		diskinfo -Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*			- operation failed				*/
/*		EINVAL	- Invalid parameter in request			*/
/*			- Transfer size is too large			*/
/*		EACCES  - Device not open in diagnostic mode		*/
/*		EFAULT  - Xmattach failed				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		copyin		copyout					*/
/*		pinu		unpinu					*/
/*		xmattach	xmdetach				*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dkiocmd(
int			arg, 
int			devflag, 
struct scdisk_diskinfo	*diskinfo)
{
	struct sc_buf           *scbuf;
	struct dk_cmd		*cmd_ptr;
	struct buf              *bp;
	struct scsi             *scsi;
	struct sc_iocmd         iocmd;
	int			segflag;
	int 			errnoval;
	int			rc;
	uchar			lun;

	/*
	 *  Set up segflag for calls to pinu and xmattach and
	 *  copy in the user's structure.
	 */
	if (devflag & DKERNEL) {
		segflag = SYS_ADSPACE;
		bcopy((caddr_t) arg, (caddr_t) &iocmd,
		      sizeof(struct sc_iocmd));
	} else {
		segflag = USER_ADSPACE;
		copyin(arg, &iocmd, sizeof(struct sc_iocmd));
	}
	
	/* This pass thru ioctl is only available during */
	/* DIAGNOSTIC mode.                              */
	if (diskinfo->mode != DK_DIAG) {
		iocmd.status_validity = 0x00;
		if (devflag & DKERNEL) {
			bcopy((caddr_t) &iocmd, (caddr_t) arg,
			      sizeof(struct sc_iocmd));
		} else {
			copyout(&iocmd, arg,
				sizeof(struct sc_iocmd));
		}


		return(EACCES);
	}
	
	/* Validate request */
	if ((int) iocmd.buffer & 0x03) {
		iocmd.status_validity = 0x00;
		if (devflag & DKERNEL) {
			bcopy((caddr_t) &iocmd, (caddr_t) arg,
			      sizeof(struct sc_iocmd));
		} else {
			copyout(&iocmd, arg,
				sizeof(struct sc_iocmd));
		}


		return(EINVAL);
	}
	
	/* Allocate an IOCTL cmd block for operation */
	
	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
	ASSERT(cmd_ptr != NULL);
	scbuf = &(cmd_ptr->scbuf);
	scsi = &(scbuf->scsi_command);
	bp = &(scbuf->bufstruct);
	
	/* Initialize the scbuf for the operation */
	scbuf->bp = NULL;
	scbuf->status_validity = 0x00;
	scbuf->timeout_value = iocmd.timeout_value;
	scbuf->flags = SC_RESUME | diskinfo->reset_delay | 
		iocmd.q_flags;
	scbuf->q_tag_msg = iocmd.q_tag_msg;
	
	/* Initialize scbuf.bufstruct for the operation */
	bp->b_dev = diskinfo->adapter_devno;
	if (iocmd.flags & B_READ)
		bp->b_flags = (B_BUSY | B_READ);
	else
		bp->b_flags = B_BUSY;
	bp->b_bcount = iocmd.data_length;
	bp->b_work = 0x00;
	bp->b_error = 0x00;
	bp->b_un.b_addr = iocmd.buffer;
	bp->b_blkno = 0x00;
	bp->b_iodone = scdisk_iodone;
	bp->b_xmemd.aspace_id = XMEM_INVAL;
	bp->b_event = EVENT_NULL;
	
	/* Initialize SCSI cmd for operation */
	scsi->scsi_length = iocmd.command_length;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (iocmd.flags & ~(B_READ | B_WRITE));
	scsi->scsi_cmd.scsi_op_code = iocmd.scsi_cdb[0];

	scsi->scsi_cmd.lun |= (iocmd.scsi_cdb[1] & 0x1f);
	scsi->scsi_cmd.scsi_bytes[0] = iocmd.scsi_cdb[2];
	scsi->scsi_cmd.scsi_bytes[1] = iocmd.scsi_cdb[3];
	scsi->scsi_cmd.scsi_bytes[2] = iocmd.scsi_cdb[4];
	scsi->scsi_cmd.scsi_bytes[3] = iocmd.scsi_cdb[5];
	scsi->scsi_cmd.scsi_bytes[4] = iocmd.scsi_cdb[6];
	scsi->scsi_cmd.scsi_bytes[5] = iocmd.scsi_cdb[7];
	scsi->scsi_cmd.scsi_bytes[6] = iocmd.scsi_cdb[8];
	scsi->scsi_cmd.scsi_bytes[7] = iocmd.scsi_cdb[9];
	scsi->scsi_cmd.scsi_bytes[8] = iocmd.scsi_cdb[10];
	scsi->scsi_cmd.scsi_bytes[9] = iocmd.scsi_cdb[11];
	
	/* Check for data transfer on operation */
	if (bp->b_bcount != 0) {
		if (bp->b_bcount > diskinfo->max_request) {
			scdisk_free_cmd_disable(cmd_ptr);
			iocmd.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &iocmd,
				      (caddr_t) arg, sizeof
				      (struct sc_iocmd));
			} else {
				copyout(&iocmd, arg, sizeof
					(struct sc_iocmd));
			}


			return(EINVAL);
		}
		/* Attach and pin user memory for transfer */
		rc = xmattach((uint)bp->b_un.b_addr,
			      (int) bp->b_bcount,
			      &(bp->b_xmemd), (int) segflag);
		if (rc != XMEM_SUCC) {
			scdisk_free_cmd_disable(cmd_ptr);
			iocmd.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &iocmd,
				      (caddr_t) arg, sizeof
				      (struct sc_iocmd));
			} else {
				copyout((caddr_t) &iocmd,
					(caddr_t) arg, sizeof
					(struct sc_iocmd));
			}


			return(EFAULT);
		}
		rc = pinu((caddr_t) bp->b_un.b_addr,
			  (int) bp->b_bcount, (short) segflag);
		if (rc != PIN_SUCC) {
			xmdetach(&(bp->b_xmemd));
			scdisk_free_cmd_disable(cmd_ptr);
			iocmd.status_validity = 0x00;
			if (devflag & DKERNEL) {
				bcopy((caddr_t) &iocmd,
				      (caddr_t) arg, sizeof
				      (struct sc_iocmd));
			} else {
				copyout(&iocmd, arg, sizeof
					(struct sc_iocmd));
			}


			return(rc);
		}
	}
	
	/* Start execution of operation */
	
	cmd_ptr->subtype = DK_IOCMD;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0;
	cmd_ptr->soft_resid = 0;
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(bp->b_event));
	
	
	if (bp->b_bcount != 0) {
		/* detach and unpin user memory */
		(void) unpinu((caddr_t) bp->b_un.b_addr,
		       (int) bp->b_bcount, (short) segflag);
		xmdetach(&(bp->b_xmemd));
	}
	
	errnoval = bp->b_error;
	iocmd.status_validity = scbuf->status_validity;
	iocmd.scsi_bus_status = scbuf->scsi_status;
	iocmd.adapter_status = scbuf->general_card_status;
	iocmd.adap_q_status = scbuf->adap_q_status;
	/* Deallocate cmd block from operation */
	scdisk_free_cmd_disable(cmd_ptr);
	
	/* Return arg stucture to user */
	if (devflag & DKERNEL) {
		bcopy((caddr_t) &iocmd, (caddr_t) arg,
		      sizeof(struct sc_iocmd));
	} else {
		copyout(&iocmd, arg, sizeof(struct sc_iocmd));
	}

	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_dk_amr_pmr					*/
/*									*/
/*	FUNCTION: 							*/
/*		DKPMR:							*/
/*									*/
/*		  Issues a prevent media removal command to the drive.  */
/*		  It also sets diskinfo->prevent_eject to TRUE so future */
/*		  reset cycles will re-issue the prevent media removal  */
/*		  command as part of error recovery.			*/
/*									*/
/*		DKAMR:							*/
/*									*/
/*		  Issues an allow media removal command to the drive.   */
/*		  It also sets diskinfo->prevent_eject to FALSE so 	*/
/*		  future reset cycles will not re-issue the prevent 	*/
/*		  media removal command as part of error recovery.	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		op	  - operation to be performed			*/
/*		devflag	 - File parameter word				*/
/*		diskinfo  - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*			- operation failed				*/
/*		EINVAL	- Invalid parameter in request			*/
/*			- Device is a SCSI Disk.			*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dk_amr_pmr(
int			op, 
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;


	if (diskinfo->dev_type == DK_DISK) {
		/*
		 * Prevent media command can not be issued to
		 * SCSI disk.
		 */
		return (EINVAL);
	}
	

	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);


        cmd_ptr->subtype = DK_ERP_IOCTL;  



	/* Initialize cmd block for prevent media removal cmd */

	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;

	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);



     /*
      *                 PREVENT ALLOW MEDIUM REMOVAL Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (1Eh)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |               Reserved                |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Reserved                    |Prevent|
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_PREVENT_ALLOW_REMOVAL;

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	if (op == DKPMR) {
		/*
		 * If DKPMR then set prevent media
		 * removal bit
		 */
		scsi->scsi_cmd.scsi_bytes[2] = 0x01;
		diskinfo->prevent_eject = TRUE;
	}
	else {
		/*
		 * If DKAMR then do not set prevent media
		 * removal bit
		 */
		scsi->scsi_cmd.scsi_bytes[2] = 0x00;
		diskinfo->prevent_eject = FALSE;
	}
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;

	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	if( cmd_bp->b_error) {
		errnoval = EIO;
	}

	scdisk_free_cmd_disable(cmd_ptr);
	return (errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_dkeject						*/
/*									*/
/*	FUNCTION: Issues a start_unit command with the load/eject bit   */
/*		  turned on (i.e. an eject media command) to a device.  */
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:	The first shipped levels of IBM supported CDROMS 	*/
/*		(i.e. Toshiba Drives) do not support the SCSI-2 	*/
/*		load eject command.  They instead require use of 	*/
/*		Toshiba unique SCSI command called "Eject Disc Caddy	*/
/*		Command". 						*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	- device not open				*/
/*			- operation failed				*/
/*		EINVAL	- Invalid parameter in request			*/
/*			- Device is a SCSI Disk				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dkeject(
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;

	if (diskinfo->dev_type == DK_DISK) {
		/*
		 * Eject media command can not be issued to
		 * SCSI disk so fail the eject command.
		 */
		return (EINVAL);
	}
	amr_flg = FALSE;
	if (diskinfo->prevent_eject) {
		/*
		 * If a prevent media removal command has been issued
		 * to the device issue allow media removal command first.
		 */
		amr_flg = TRUE;
		diskinfo->prevent_eject = FALSE;
		errnoval = scdisk_dk_amr_pmr(DKAMR,diskinfo);
		if (errnoval)
			return (errnoval);
	}

	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for start unit cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);


	if (!diskinfo->load_eject_alt) {
		/*
		 * If this device supports the SCSI-2 load eject
		 * mechanism then we will not use Toshiba's unique 
		 * eject command.
		 */

      /*                   START STOP UNIT Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (1Bh)                |
       *|-----+---------------------------------------------------------------|
       *| 1   | Logical Unit Number   |                  Reserved     | Immed |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved            |  LoEj |  Start|
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Control                             |
       *+=====================================================================+
       */

		/* Initialize SCSI cmd for operation */
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_length = 6;
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		scsi->scsi_cmd.scsi_op_code = SCSI_START_STOP_UNIT;

		scsi->scsi_cmd.scsi_bytes[0] = 0x00;
		scsi->scsi_cmd.scsi_bytes[1] = 0x00;
		scsi->scsi_cmd.scsi_bytes[2] = 0x02;  /* LoEj bit set */
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	}
	else {
		/*
		 * This device does not support the SCSI-2 load eject 
		 * command but only supports the Toshiba's unique eject disc
		 * caddy command.  0xc4
		 */

      /*                Toshiba's Eject Disc Caddy Command
       *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
       *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
       *|Byte |       |       |       |       |       |       |       |       |
       *|=====+===============================================================|
       *| 0   |                           Operation Code (C4h)                |
       *|-----+---------------------------------------------------------------|
       *| 1   | Reserved              |                  Reserved     | Immed |
       *|-----+---------------------------------------------------------------|
       *| 2   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 3   |                           Reserved                            |
       *|-----+---------------------------------------------------------------|
       *| 4   |                           Reserved            		      |
       *|-----+---------------------------------------------------------------|
       *| 5   |                           Reserved            		      |
       *|-----+---------------------------------------------------------------|
       *| 6   |                           Reserved            		      |
       *|-----+---------------------------------------------------------------|
       *| 7   |                           Reserved            		      |
       *|-----+---------------------------------------------------------------|
       *| 8   |                           Reserved            		      |
       *|-----+---------------------------------------------------------------|
       *| 9   |                           Control                             |
       *+=====================================================================+
       */
		/* Initialize SCSI cmd for operation */
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_length = 0xa;
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;
		scsi->scsi_cmd.scsi_op_code = 0xc4; 

		scsi->scsi_cmd.scsi_bytes[0] = 0x00;
		scsi->scsi_cmd.scsi_bytes[1] = 0x00;
		scsi->scsi_cmd.scsi_bytes[2] = 0x00; 
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;
		scsi->scsi_cmd.scsi_bytes[4] = 0x00;
		scsi->scsi_cmd.scsi_bytes[5] = 0x00;
		scsi->scsi_cmd.scsi_bytes[6] = 0x00;
		scsi->scsi_cmd.scsi_bytes[7] = 0x00;
		scsi->scsi_cmd.scsi_bytes[8] = 0x00;
		scsi->scsi_cmd.scsi_bytes[9] = 0x00;

	}

	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	if( cmd_bp->b_error) {
		errnoval = EIO;
	}
	
	scdisk_free_cmd_disable(cmd_ptr);

	if (amr_flg) {
		/*
		 * Let next open or reset cycle re-issue
		 * the prevent media removal command.
		 */
		diskinfo->prevent_eject = TRUE;
	}
	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_dkformat						*/
/*									*/
/*	FUNCTION: Issues a format unit command to the open device	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg	  - address of user argument structure		*/
/*		devflag	   - File parameter word			*/
/*		diskinfo  - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	    - device not open				*/
/*			    - operation failed				*/
/*			    - copyin failed				*/
/*		EINVAL	    - Invalid parameter in request	        */
/*			    - device is a SCSI Disk 			*/
/*		EWRPROTECT  - media is write protected			*/
/*		EACCES	    - Device not open in SC_SINGLE mode		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		copyin		bcopy					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dkformat(
int			arg, 
int			devflag, 
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	int		tries = 0;


	if (diskinfo->dev_type == DK_DISK) {
		/*
		 * Format unit command can not be issued to
		 * SCSI disks.  
		 */
		return (EINVAL);
	}
	if (diskinfo->wprotected) {
		/*
		 * Fail if media is write protected
		 */
		return(EWRPROTECT);
	}	
	if (diskinfo->mode != DK_SINGLE_MD) {
		/*
		 * Format unit command requires SC_SINGLE open.
		 */
		return (EACCES);
	}

	while (tries < 4) {



    /*                    Defect List Header
     *
     * +=====-=======-=======-=======-=======-=======-=======-=======-========+
     * |  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0    |
     * |Byte |       |       |       |       |       |       |       |        |
     * |=====+================================================================|
     * | 0   |                           Reserved                             |
     * |-----+----------------------------------------------------------------|
     * | 1   |  FOV  |  DPRY |  DCRT |  STPF |   IP  |   DSP |  Immed|   VS   |
     * |-----+----------------------------------------------------------------|
     * | 2   | (MSB)                                                          |
     * |-----+---                        Defect List Length                ---|
     * | 3   |                                                          (LSB) |
     * +======================================================================+
     */

		/*
		 * Fill in defect list header: FOV = 0 (i.e. use 
		 * default settings) if no arg parameter is given
		 */
		ASSERT(DK_FMT_DEF_SIZE >= 4);
		diskinfo->def_list_header[0] = 0x00; 
		diskinfo->def_list_header[2] = 0x00;
		diskinfo->def_list_header[3] = 0x00;

		if (arg == (int) NULL) {
			/*
			 * Fill in defect list header: FOV = 0 (i.e. use 
			 * default settings) if no arg parameter is given
			 */

			diskinfo->def_list_header[1] = 0x00;

		}
		else {
			/*
			 * Fill in defect list header with user specified
			 * values.
			 */

			if (devflag & DKERNEL) {
				bcopy((caddr_t) arg, 
				      (caddr_t) &(diskinfo->def_list_header[1]),
				      sizeof(uchar));
			} else {
				if (copyin(arg, 
				       (caddr_t) &(diskinfo->def_list_header[1]), 
				       sizeof(uchar))) {
					return (EIO);
				}
			}
	
		}
		errnoval = 0;
		cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
		cmd_ptr->subtype = DK_ERP_IOCTL;  

		/* Initialize cmd block for format unit cmd */
		cmd_bp = &(cmd_ptr->scbuf.bufstruct);
		cmd_bp->b_dev = diskinfo->adapter_devno;
		cmd_bp->b_flags = (B_WRITE | B_BUSY);
		cmd_bp->b_bcount = DK_FMT_DEF_SIZE;
		cmd_bp->b_un.b_addr = (caddr_t)&(diskinfo->def_list_header[0]);
		cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
		cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
		cmd_ptr->scbuf.timeout_value = diskinfo->fmt_timeout;
		cmd_ptr->scbuf.bp = NULL;
		cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
		cmd_ptr->bp = NULL;
		
		scbuf = &(cmd_ptr->scbuf);

		
		/* Initialize SCSI cmd for operation */
		
		scsi = &(cmd_ptr->scbuf.scsi_command);
		scsi->scsi_length = 6;
		scsi->scsi_id = diskinfo->scsi_id;
		scsi->flags = (ushort) diskinfo->async_flag;

     /*                         FORMAT UNIT Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                           Operation Code (04h)                |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |FmtData| CmpLst|   Defect List Format  |
      *|-----+---------------------------------------------------------------|
      *| 2   |                           Vendor Specific                     |
      *|-----+---------------------------------------------------------------|
      *| 3   | (MSB)                                                         |
      *|-----+---                        Interleave                     -----|
      *| 4   |                                                       (LSB)   |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */


		
		scsi->scsi_cmd.scsi_op_code = SCSI_FORMAT_UNIT;

		
		/*
		 * To support the widest range of drives we will try all 
		 * permutations of the FmtData and CmpLst. This is
		 * because SCSI-2 does not say which settings
		 * a device must supp
		 */
		 switch (tries) {
		     case 0:	/* FmtData = 0 CmpLst = 0 */

			/*
			 * No data transferred for this case
			 */
			cmd_bp->b_flags = 0x0;
			cmd_bp->b_bcount = 0;
			cmd_bp->b_un.b_addr = NULL;
			break;	
		    case 1:	/* FmtData = 1 CmpLst = 0 */
			scsi->scsi_cmd.lun |= 0x10;
			break;
		    case 2:	/* FmtData = 0 CmpLst = 1 */
			scsi->scsi_cmd.lun |= 0x8;
			break;
		    case 3:	/* FmtData = 1 CmpLst = 1 */
			scsi->scsi_cmd.lun |= 0x18;
			break;
		    
		}

		/*
		 * Interleave of 0 means use the device's default interleave
		 * settings.
		 */
		scsi->scsi_cmd.scsi_bytes[0] = 0x00;
		scsi->scsi_cmd.scsi_bytes[1] = 0x00;
		scsi->scsi_cmd.scsi_bytes[2] = 0x00;
		scsi->scsi_cmd.scsi_bytes[3] = 0x00;
		
		/* Start execution of operation */
		
		
		cmd_ptr->intrpt = 1;
		cmd_ptr->flags |= DK_READY;
		scdisk_start_disable(diskinfo);
		
		/* Wait for completion */
		scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
				     (uint *)&(cmd_bp->b_event));

		if (( cmd_bp->b_error) && (cmd_bp->b_error != ETIMEDOUT)) {
			/*
			 * Do not filter ETIMEDOUT errors
			 */
			errnoval = EIO;
		}
		else {
			errnoval = cmd_bp->b_error;
		}
		

		if ((cmd_bp->b_error) && 
		    (scbuf->status_validity & SC_SCSI_ERROR) &&
		    ((scbuf->scsi_status == SC_CHECK_CONDITION) ||
		     (scbuf->scsi_status == SC_COMMAND_TERMINATED))) {
			/*
			 * If this command failed with a check condition
			 * or command terminated then this implies the
			 * drive does not like these bit settings.  So
			 * try another permutation for the FmtData and 
			 * CmpLst bits.
			 */
			tries++;
			scdisk_free_cmd_disable(cmd_ptr);
		}
		else {
			scdisk_free_cmd_disable(cmd_ptr);
			break;
		}
			
	} /* while */
	return (errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_dkaudio 					        */
/*									*/
/*	FUNCTION: This routine will issue an audio command to 		*/
/*		  a device.						*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*       CD-ROM Audio Control Parameters Page (Mode Page E)		*/
/* +=====-======-======-======-======-======-======-======-======+	*/
/* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |	*/
/* |Byte |      |      |      |      |      |      |      |      |	*/
/* |=====+======+======+=========================================|	*/
/* | 0   |  PS  |Reserv|      Page Code (0Eh)                    |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 1   |                    Parameter Length (0Eh)             |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 2   |       Reserved                   |Immed | SOTC |Reserv|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 3   |                    Reserved                           |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 4   |                    Reserved                           |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 5   |APRVal|     Reserved       |   Format of LBAs / Sec.   |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 6   | (MSB)                                                 |	*/
/* |-----+---  Logical Blocks per Second of Audio Playback    ---|	*/
/* | 7   |                                                  (LSB)|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 8   |      Reserved             |Output Port 0 Channel Selec|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 9   |                    Output Port 0 Volume               |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 10  |      Reserved             |Output Port 1 Channel Selec|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 11  |                    Output Port 1 Volume               |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 12  |      Reserved             |Output Port 2 Channel Selec|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 13  |                    Output Port 2 Volume               |	*/
/* |-----+-------------------------------------------------------|	*/
/* | 14  |      Reserved             |Output Port 3 Channel Selec|	*/
/* |-----+-------------------------------------------------------|	*/
/* | 15  |                    Output Port 3 Volume               |	*/
/* +=============================================================+	*/
/*									*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg	  - address of user argument structure		*/
/*		devflag	  - File parameter word				*/
/*		diskinfo  - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	    - reset cycle failed			*/
/*			    - operation failed				*/
/*			    - copyin failed				*/
/*		EINVAL	    - Invalid parameter in request	        */
/*		EACCES	    - Device not open in SC_SINGLE mode		*/
/*		EBUSY	    - Commands are already queued to us		*/
/*		ENOTREADY   - Media Missing.				*/
/*		EWRPROTECT  - Write protected device			*/
/*									*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		copyin		bcopy					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dkaudio(
int			arg, 
int			devflag,
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	struct cd_audio_cmd  audio_op;
	int		start,end;
	int		allocation_length;
	uchar		minutes,seconds,frames;
	uchar		pg_e_vol[DK_MAX_SIZE_PG_E-DK_VOL_START_PG_E+1];
	int		msf;
	short		p_length,index;
	int		i;
	uchar		audio_status;
	uchar		volume_set = FALSE;


	if (diskinfo->mode != DK_SINGLE_MD) {
		/*
		 * This ioctl requires SC_SINGLE open.
		 * If this is not the case then fail it.
		 */
		return (EACCES);
	}

	if (!diskinfo->play_audio) {
		/*
		 * This ioctl requires the
		 * device to support SCSI-2 play
		 * audio SCSI commands.  If this
		 * is not the case, then fail
		 * it.
		 */
		return(EINVAL);
	}

	if ((diskinfo->dk_cmd_q_head != NULL) ||
	    (diskinfo->cmds_out) ||
	    (diskinfo->raw_io_cmd != NULL)  ||
	    (diskinfo->dk_bp_queue.in_progress.head != NULL) ||
	    (diskinfo->currbuf != NULL)) {

	  	/*
		 * Even though this ioctl requires an
		 * exclusive open, it is possible an
		 * application could have forked
		 * off multiple processes with the
		 * same file descriptor. If this is the
		 * case and if these other processes
		 * have outstanding commands to the device
		 * or to us then we need to fail this
		 * ioctl.  We can not let this ioctl continue
		 * since we may be re-issuing commands from
		 * the reset sequence which could step on
		 * other active threads in the driver.
		 */

	  	return(EBUSY);
	}


	/*
	 * Copy data in from user.
	 */

	if (devflag & DKERNEL) {
		bcopy((caddr_t) arg, (caddr_t) &audio_op,
		      sizeof(struct cd_audio_cmd));
	} else {

		if (copyin(arg, &audio_op, sizeof(struct cd_audio_cmd))) {
			return (EIO);
		}
	}



	if (audio_op.audio_cmds & CD_SET_VOLUME ) {
		/*
		 *  Volume needs to be adjusted.
		 */
	  	
	  	/*
		 * Initialize page_e_vol to the current volume
		 * settings in mode page E's desired settings.
		 */

	  	index = diskinfo->dd.page_index[0xe];
                p_length = diskinfo->mode_buf[++index];

                if (p_length > DK_MAX_SIZE_PG_E) {
                        p_length = DK_MAX_SIZE_PG_E;
                }

		
               for (i=DK_VOL_START_PG_E;i<=p_length;i++) {
                        pg_e_vol[i-DK_VOL_START_PG_E] =
				diskinfo->mode_buf[index + i];
                                          
		}

		if ((audio_op.volume_type & DK_CD_VOL_MASK) 
						== CD_ALL_AUDIO_MUTE) {
			/*
			 * Mute volume on all output ports.
			 */
			pg_e_vol[1] = 0;
			pg_e_vol[3] = 0;
			pg_e_vol[5] = 0;
			pg_e_vol[7] = 0;
				 				 
		}
		else if ((audio_op.volume_type & DK_CD_VOL_MASK) 
						== CD_VOLUME_ALL) {

			/*
			 * Set volume on all output ports to one common
			 * value.
			 */
		  	pg_e_vol[1] = audio_op.all_channel_vol;
			pg_e_vol[3] = audio_op.all_channel_vol;
			pg_e_vol[5] = audio_op.all_channel_vol;
			pg_e_vol[7] = audio_op.all_channel_vol;

		}
		else if ((audio_op.volume_type & DK_CD_VOL_MASK) 
						== CD_VOLUME_CHNLS) {
			/*
			 * Set volume on individual output ports.
			 */
			pg_e_vol[1] = audio_op.out_port_0_vol;
			pg_e_vol[3] = audio_op.out_port_1_vol;
			pg_e_vol[5] = audio_op.out_port_2_vol;
			pg_e_vol[7] = audio_op.out_port_3_vol;

		}

		if (audio_op.volume_type & CD_SET_AUDIO_CHNLS) {
			/*
			 * Map audio channels to output ports.
			 */
			pg_e_vol[0] = 0xf & audio_op.out_port_0_sel;
			pg_e_vol[2] = 0xf & audio_op.out_port_1_sel;
			pg_e_vol[4] = 0xf & audio_op.out_port_2_sel;
			pg_e_vol[6] = 0xf & audio_op.out_port_3_sel;

		}

                diskinfo->overide_pg_e = TRUE;

		
		/*
		 * First set desired mode data for page e to
		 * match what we built in page_e_vol.
		 */
                for (i=DK_VOL_START_PG_E;i<=p_length;i++) {
                        diskinfo->mode_buf[index + i] =
                                   pg_e_vol[i-DK_VOL_START_PG_E];
                                          
		}

		/*
		 * If we changed the modes above then we must
		 * force the device driver to run through it's
		 * verification sequence to issue the
		 * the necessary mode_select.
		 */
	  	diskinfo->disk_intrpt = 1;
		scdisk_test_unit_ready_disable(diskinfo);
		scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
			     &(diskinfo->open_event));

                errnoval = diskinfo->errno;
 		if (errnoval) {
		  	return(errnoval);
		}
		/*
		 * Clear the set volume bit, since we just set it.
		 */
		audio_op.audio_cmds &= ~CD_SET_VOLUME;
		volume_set = TRUE;

	}

	if (audio_op.audio_cmds == CD_TRK_INFO_AUDIO) {
		/*
		 * Get the number of tracks on the media
		 * or the last MSF on the media
		 *
		 * Issue a Read TOC command.  
		 * NOTE MSF notation requires we get
		 * data for the last track , since the MSF 
		 * of the last track can only be extracted from the
		 * last track's TOC track descriptor.  Thus for MSF
		 * we need to issue two read TOC commands: one
		 * to find the last track and the second
		 * to get the last track numbers MSF.
		 */

	  	/*
		 * Start equals track number for read TOC
		 */
	  	start = 0;  

		
		msf = 0;
		allocation_length =  DK_TOC_LEN;

		if (audio_op.msf_flag) {
		  	/*
			 * Do first read TOC to
			 * find the last track number
			 */
			msf = DK_TOC_MSF;
			errnoval = scdisk_read_toc(diskinfo,(uchar) start,
						   (uchar) msf,
						   allocation_length);
			if (errnoval) {
			  	return(errnoval);
			}

			/*
			 * Get last track number
			 */
			start = diskinfo->ioctl_buf[3];

			/*
			 * Get MSF of first track 
			 */

			minutes  = diskinfo->ioctl_buf[9];
			seconds  = diskinfo->ioctl_buf[10];
			frames   = diskinfo->ioctl_buf[11];

			msf = DK_TOC_MSF;

		}
	  	errnoval = scdisk_read_toc(diskinfo,(uchar) start,
					   (uchar) msf,
					   allocation_length);

       /*
	*                       READ TOC Data Format
	* +=====-======-======-======-======-======-======-======-======+
	* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	* |Byte |      |      |      |      |      |      |      |      |
	* |=====+=======================================================|
	* | 0   | (MSB)                                                 |
	* |-----+---                TOC Data Length                  ---|
	* | 1   |                                                 (LSB) |
	* |-----+-------------------------------------------------------|
	* | 2   |                   First Track Number                  |
	* |-----+-------------------------------------------------------|
	* | 3   |                   Last Track Number                   |
	* |=====+=======================================================|
	* |     |                   TOC Track Descriptor(s)             |
	* |=====+=======================================================|
	* | 0   |                   Reserved                            |
	* |-----+-------------------------------------------------------|
	* | 1   |              ADR           |         Control          |
	* |-----+-------------------------------------------------------|
	* | 2   |                   Track Number                        |
	* |-----+-------------------------------------------------------|
	* | 3   |                   Reserved                            |
	* |-----+-------------------------------------------------------|
	* | 4   | (MSB)                                                 |
	* |- - -+---                Absolute CD-ROM Address          ---|
	* | 7   |                                                 (LSB) |
	* +=============================================================+
	*/

		if (audio_op.msf_flag) {
		  	/*
			 * User request MSF of first track and
			 * of last track.
			 */
			audio_op.indexing.msf.first_mins = minutes;
			audio_op.indexing.msf.first_secs = seconds;
			audio_op.indexing.msf.first_frames = frames;

			audio_op.indexing.msf.last_mins =
			  		diskinfo->ioctl_buf[9];
			audio_op.indexing.msf.last_secs =
			  		diskinfo->ioctl_buf[10];
			audio_op.indexing.msf.last_frames =
			  		diskinfo->ioctl_buf[11];


		}
		else {
		  	/*
			 * User request first track number and last
			 * track number.
			 */
		  	audio_op.indexing.track_index.first_track =
			  			diskinfo->ioctl_buf[2];

			audio_op.indexing.track_index.last_track =
		  				diskinfo->ioctl_buf[3];

		  	audio_op.indexing.track_index.first_index = 0;
		  	audio_op.indexing.track_index.last_index = 0;

		}

	}
	else if (audio_op.audio_cmds == CD_GET_TRK_MSF) {
		/*
		 * Get the MSF of the specified track.
		 *
		 * Issue a Read TOC command.  
		 */

	  	/*
		 * Start equals track number for read TOC
		 */
	  	start = audio_op.indexing.track_msf.track;  

		msf = DK_TOC_MSF;

		allocation_length =  DK_TOC_LEN;

		errnoval = scdisk_read_toc(diskinfo,(uchar) start,
					   (uchar) msf,
					   allocation_length);
		if (errnoval) {
			return(errnoval);
		}

		/*
		 * Get MSF of track 
		 */


		audio_op.indexing.track_msf.mins =
			  		diskinfo->ioctl_buf[9];
		audio_op.indexing.track_msf.secs =
			  		diskinfo->ioctl_buf[10];
		audio_op.indexing.track_msf.frames =
			  		diskinfo->ioctl_buf[11];

		
	}
	else if (audio_op.audio_cmds == CD_PLAY_AUDIO ) {
		/*
		 *  Issue a SCSI play audio command.
		 */
		diskinfo->play_audio_started = TRUE;
	  	if (audio_op.msf_flag) {
			/*
			 * If MSF format is specified then
			 * issue a play audio MSF SCSI command.
			 */
		  	start = audio_op.indexing.msf.first_mins << 16;
			start |= audio_op.indexing.msf.first_secs << 8;
			start |= audio_op.indexing.msf.first_frames;

		  	end = audio_op.indexing.msf.last_mins << 16;
			end |= audio_op.indexing.msf.last_secs << 8;
			end |= audio_op.indexing.msf.last_frames ;	
	
			errnoval = scdisk_audio_msf(diskinfo,start,end);
		}
		else {
			/*
			 *  Issue a play audio track/index SCSI command.
			 */
		  	errnoval = scdisk_audio_trk_indx(diskinfo,
		             audio_op.indexing.track_index.first_track,
			     audio_op.indexing.track_index.first_index,
			     audio_op.indexing.track_index.last_track,
			     audio_op.indexing.track_index.last_index);
		}


	}
	else if (audio_op.audio_cmds == CD_PAUSE_AUDIO ) {
		/*
		 *  Issue Pause SCSI command.
		 */
	errnoval = scdisk_pause_resume(diskinfo,(uchar)DK_PAUSE);

	}
	else if (audio_op.audio_cmds == CD_RESUME_AUDIO) {
		/*
		 *  Issue Resume SCSI command.
		 */
	  	errnoval = scdisk_pause_resume(diskinfo,(uchar) DK_RESUME);

	}
	else if (audio_op.audio_cmds == CD_STOP_AUDIO) {
		/*
		 *  Stop Audio operation.
		 */

		diskinfo->play_audio_started = FALSE;
		diskinfo->errno = 0x00;
		diskinfo->disk_intrpt = 1;
		(void) scdisk_start_unit_disable(diskinfo,(uchar) DK_STOP,
						 (uchar) FALSE);		
		scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
			     &(diskinfo->open_event));


		errnoval = diskinfo->errno;


	}
	else if (audio_op.audio_cmds == CD_INFO_AUDIO) {
		/*
		 * Get information on status of Play Audio
		 * operation.
		 */

		/*
		 * First lets get current volume.
		 */
		errnoval = scdisk_ioctl_mode_sense(diskinfo,
						   (uchar) DK_SENSE_CURRENT,
						   (uchar) 0xe,
						   (int) DK_MSENSE_E_LEN);

		if (errnoval) {
		  	return(errnoval);
		}
		/*
		 * Extract volume data from ioctl_buffer
		 */

		audio_op.out_port_0_vol = diskinfo->ioctl_buf[21];
		audio_op.out_port_1_vol = diskinfo->ioctl_buf[23];
		audio_op.out_port_2_vol = diskinfo->ioctl_buf[25];
		audio_op.out_port_3_vol = diskinfo->ioctl_buf[27];

		audio_op.out_port_0_sel = diskinfo->ioctl_buf[20];
		audio_op.out_port_1_sel = diskinfo->ioctl_buf[22];
		audio_op.out_port_2_sel = diskinfo->ioctl_buf[24];
		audio_op.out_port_3_sel = diskinfo->ioctl_buf[26];

			
		msf = DK_SUBQ_MSF;

		errnoval = scdisk_read_subchnl(diskinfo,(uchar)DK_SUBQ_FMT,
					       (uchar) 0,
					       (uchar) DK_SUBQ,
					       (uchar) msf,
					       (int) DK_SUBQ_LEN);
       /*
        *              Sub-Q Channel Data Format
	* +=====-======-======-======-======-======-======-======-======+
	* |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	* |Byte |      |      |      |      |      |      |      |      |
	* |=====+=======================================================|
	* |     |                 Sub-channel Data Header               |
	* |=====+=======================================================|
	* | 0   |                        Reserved                       |
	* |-----+-------------------------------------------------------|
	* | 1   |                      Audio Status                     |
	* |-----+-------------------------------------------------------|
	* | 2   | (MSB)                                                 |
	* |-----+---              Sub-channel Data Length            ---|
	* | 3   |                                                 (LSB) |
	* |=====+=======================================================|
	* |     |                 Sub-Q Channel Data Block              |
	* |=====+=======================================================|
	* | 4   |                Sub Channel Data Format code (00h)     |
	* |-----+-------------------------------------------------------|
	* | 5   |       ADR                |        Control             |
	* |-----+-------------------------------------------------------|
	* | 6   |                      Track Number                     |
	* |-----+-------------------------------------------------------|
	* | 7   |                      Index Number                     |
	* |-----+-------------------------------------------------------|
	* | 8   | (MSB)                                                 |
	* |- - -+---               Absolute CD-ROM Address           ---|
	* | 11  |                                                 (LSB) |
	* |-----+-------------------------------------------------------|
	* | 12  | (MSB)                                                 |
	* |- - -+---             Track Relative CD-ROM Address       ---|
	* | 15  |                                                 (LSB) |
	* |-----+-------------------------------------------------------|
	* | 16  | MCVal|               Reserved                         |
	* |-----+-------------------------------------------------------|
	* | 17  | (MSB)                                                 |
	* |- - -+---          Media Catalog Number (UPC/Bar Code)    ---|
	* | 31  |                                                 (LSB) |
	* |-----+-------------------------------------------------------|
	* | 32  |  TCVal |             Reserved                         |
	* |-----+-------------------------------------------------------|
	* | 33  | (MSB)                                                 |
	* |- - -+--Track International-Standard-Recording-Code (ISRC)---|
	* | 47  |                                                 (LSB) |
	* +=============================================================+
	*/
		
		audio_status = diskinfo->ioctl_buf[1];

		/*
		 * Extract status of audio operation
		 */
		switch (audio_status) {

			case DK_AUDIO_PLAY_IN_PROGRESS: 
				audio_op.status = CD_PLAY_AUDIO;
				break;
			case DK_AUDIO_PLAY_PAUSED:   
				audio_op.status = CD_PAUSE_AUDIO;
				break;
			case DK_PLAY_COMPLETED:      
				audio_op.status = CD_COMPLETED;
				break;
			case DK_NO_AUDIO_STATUS: 
				audio_op.status = CD_NO_AUDIO;
				break;
			case DK_AUDIO_STATUS_INVALID:
				audio_op.status = CD_NOT_VALID;
				break;

			case DK_PLAY_STOP_ERROR:      
			default:
				audio_op.status = CD_STATUS_ERROR;
				break;

		}
		/*
		 * Extract current track/index and MSF
		 */
		audio_op.indexing.info_audio.current_mins =
			  		diskinfo->ioctl_buf[9];
		audio_op.indexing.info_audio.current_secs =
			  		diskinfo->ioctl_buf[10];
		audio_op.indexing.info_audio.current_frames =
			  		diskinfo->ioctl_buf[11];
		audio_op.indexing.info_audio.current_track =
			  			diskinfo->ioctl_buf[6];
		audio_op.indexing.info_audio.current_index =
			  			diskinfo->ioctl_buf[7];
	}
	else {
		if (!volume_set) {
			/*
			 * Invalid operation if no volume adjustment
			 * was made.
			 */
			errnoval = EINVAL;
		}
	}

	/* Return arg stucture to user */

	if (devflag & DKERNEL) {
		bcopy((caddr_t) &audio_op, (caddr_t) arg,
		      sizeof(struct cd_audio_cmd));
	} else {
		copyout(&audio_op, arg, sizeof(struct cd_audio_cmd));
	}


	return(errnoval);
	
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_dk_cd_mode					*/
/*									*/
/*	FUNCTION: This routine allows a user to get the current		*/
/*		  CD-ROM Data Mode.  It also allows them to change	*/
/*		  it.  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*		Changing the CD-ROM data mode may change the block	*/
/*		size used.						*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		arg	  - address of user argument structure		*/
/*		devflag	  - File parameter word				*/
/*		diskinfo  - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EIO	    - reset cycle failed			*/
/*			    - operation failed				*/
/*			    - copyin/copyout failed			*/
/*		EINVAL	    - Invalid parameter in request	        */
/*		EACCES	    - Device not open in SC_SINGLE mode		*/
/*		EBUSY	    - Commands are already queued to us		*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		copyin		bcopy					*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_dk_cd_mode(
int			arg, 
int			devflag,
struct scdisk_diskinfo	*diskinfo)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	struct mode_form_op mode_op;
	int		chg_mode_flg = FALSE;




	/*
	 * Copy data in from user.
	 */


	if (devflag & DKERNEL) {
		bcopy((caddr_t) arg, (caddr_t) &mode_op,
		      sizeof(struct mode_form_op));
	} else {

		if (copyin(arg, &mode_op, sizeof(struct mode_form_op))) {
			return (EIO);
		}
	}
	
	if (mode_op.action == CD_GET_MODE) {
		/*
		 * Get the current CD-ROM Data Mode
		 */
	  	mode_op.cd_mode_form = diskinfo->current_cd_mode;
	}
	else if (mode_op.action == CD_CHG_MODE) {
		if (diskinfo->mode != DK_SINGLE_MD) {
			/*
			 * This ioctl requires SC_SINGLE open.
			 */
			return (EACCES);
		}

		if ((diskinfo->dk_cmd_q_head != NULL) ||
		    (diskinfo->cmds_out) ||
		    (diskinfo->raw_io_cmd != NULL)  ||
		    (diskinfo->dk_bp_queue.in_progress.head != NULL) ||
		    (diskinfo->currbuf != NULL)) {
			/*
			 * Even though this ioctl requires an
			 * exclusive open, it is possible an
			 * application could have forked
			 * off multiple processes with the
			 * same file descriptor. If this is the
			 * case and if these other processes
			 * have outstanding commands to the device
			 * or to us then we need to fail this
			 * ioctl.  We can allow this ioctl to
			 * continue if other commands are pending
			 * since this ioctl may change the block size.
			 */


		  	return (EBUSY);
		}

		if (mode_op.cd_mode_form == CD_MODE1) {

			/* 
			 * The mode data string extracted from
			 * ODM by the configuration method
			 * contains the valide CD-ROM Data Mode 1
			 * density code, so we can always change
			 * to this one.
			 * Change block size and the density code
			 * to the valid values for CD-ROM Data Mode 1.
			 */

		  	diskinfo->block_size = diskinfo->cfg_block_size;
			diskinfo->current_cd_code =  
					diskinfo->cd_mode1_code;
			diskinfo->current_cd_mode = CD_MODE1;
			chg_mode_flg = TRUE;

		}
		else if ((mode_op.cd_mode_form == CD_MODE2_FORM1) &&
			 (diskinfo->valid_cd_modes & DK_M2F1_VALID)) {

			/* 
			 * If the configuration method specified a
			 * valid value for CD-ROM XA Mode 2 Form 1,
			 * then change block size to the valid
			 * value for CD-ROM XA Mode 2 Form 1  
			 * and to the correct density code.
			 */

		  	diskinfo->block_size = DK_M2F1_BLKSIZE;
			diskinfo->current_cd_code = 
					diskinfo->cd_mode2_form1_code;
			diskinfo->current_cd_mode = CD_MODE2_FORM1;
			chg_mode_flg = TRUE;
		}
		else if ((mode_op.cd_mode_form == CD_MODE2_FORM2) &&
			 (diskinfo->valid_cd_modes & DK_M2F2_VALID)) {

			/* 
			 * If the configuration method specified a
			 * valid value for CD-ROM XA Mode 2 Form 2,
			 * then change block size to the valid
			 * value for CD-ROM XA Mode 2 Form 2  
			 * and to the correct density code.
			 */

		  	diskinfo->block_size = DK_M2F2_BLKSIZE;
			diskinfo->current_cd_code = 
					diskinfo->cd_mode2_form1_code;
			diskinfo->current_cd_mode = CD_MODE2_FORM2;
			chg_mode_flg = TRUE;
		}
		else if ((mode_op.cd_mode_form == CD_DA)  &&
			 (diskinfo->valid_cd_modes & DK_CDDA_VALID)) {

			/* 
			 * If the configuration method specified a
			 * valid value for CD-DA, then change block 
			 * size to the valid value for CD-DA  
			 * and to the correct density code.
			 */

		  	diskinfo->block_size = DK_CDDA_BLKSIZE;
			diskinfo->current_cd_code = 
					diskinfo->cd_da_code;
			diskinfo->current_cd_mode = CD_DA;
			chg_mode_flg = TRUE;
		}
		else {
		  	/*
			 * Invalid operation
			 */
	  		errnoval = EINVAL;
		}

	}
	else {
		/*
		 * Invalid operation
		 */
	  	errnoval = EINVAL;
	}

	if (chg_mode_flg) {

		diskinfo->ioctl_chg_mode_flg = TRUE;
		diskinfo->mult_of_blksize = 
                                diskinfo->block_size/DK_BLOCKSIZE;

		/*
		 * If we changed the modes above then we must
		 * force the device driver to run through it's
		 * verification sequence to issue the
		 * the necessary mode_select.
		 */
	  	diskinfo->disk_intrpt = 1;
		scdisk_test_unit_ready_disable(diskinfo);
		scdisk_sleep(diskinfo,
			     &(diskinfo->disk_intrpt),&(diskinfo->open_event));

                errnoval = diskinfo->errno;
 
	}


	/*
	 * Copy data out for user.
	 */
	 
	if (devflag & DKERNEL) {
		bcopy((caddr_t) &mode_op, (caddr_t) arg,
		      sizeof(struct mode_form_op));
	} else {
		if (copyout(&mode_op, arg, sizeof(struct mode_form_op))) {
			errnoval = EIO;
		}
	}

	return (errnoval);
		
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_alloc						*/
/*									*/
/*	FUNCTION: Allocates memory from the pinned heap for		*/
/*		  the ioctl command blocks and buf command blocks.      */
/*		  It also initializes this command blocks		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*		dk_cmd	        Disk device command block               */
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		ENOMEM  - Xmalloc failed				*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		xmalloc			bzero				*/
/*									*/
/*									*/
/************************************************************************/

int scdisk_alloc(struct scdisk_diskinfo *diskinfo)
{

	int 	i;				/* general counter 	*/



	/* 
	 * Allocate cmd blocks for buf's
	 */

	diskinfo->cmd_pool = 
		(struct dk_cmd *) xmalloc(
		(uint) ((diskinfo->queue_depth)*(sizeof(struct dk_cmd))),
		2,pinned_heap);

	if (diskinfo->cmd_pool == NULL) {
		/*
		 * Xmalloc failed
		 */
		return(ENOMEM);
	}
	


	/*
	 * Initialize_cmds
	 */

	bzero((caddr_t) diskinfo->cmd_pool, 
	      ((diskinfo->queue_depth)*(sizeof(struct dk_cmd))));

	for (i = 0; i< (diskinfo->queue_depth); i++) {

		diskinfo->cmd_pool[i].status = DK_FREE;
		diskinfo->cmd_pool[i].diskinfo = diskinfo;
		diskinfo->cmd_pool[i].scbuf.flags = 
		    diskinfo->reset_delay;
		diskinfo->cmd_pool[i].scbuf.q_tag_msg = SC_NO_Q;
		diskinfo->cmd_pool[i].scbuf.bufstruct.b_event = 
		    EVENT_NULL;
	}
	diskinfo->pool_index = 0;

	return(0);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_init_cmds					*/
/*									*/
/*	FUNCTION: This routine initializes the diskinfo structure's    	*/
/*		  commands at config init time.			       	*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void scdisk_init_cmds(struct scdisk_diskinfo *diskinfo)
{

	
	diskinfo->ioctl_cmd.status = DK_FREE;
	diskinfo->ioctl_cmd.diskinfo = diskinfo;
	diskinfo->ioctl_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->ioctl_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->ioctl_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;
	
	diskinfo->reset_cmd.status = DK_FREE;
	diskinfo->reset_cmd.diskinfo = diskinfo;
	diskinfo->reset_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->reset_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->reset_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;

	diskinfo->reqsns_cmd.status = DK_FREE;
	diskinfo->reqsns_cmd.diskinfo = diskinfo;
	diskinfo->reqsns_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->reqsns_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->reqsns_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;
	
	diskinfo->q_recov_cmd.status = DK_FREE;
	diskinfo->q_recov_cmd.diskinfo = diskinfo;
	diskinfo->q_recov_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->q_recov_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->q_recov_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;
	
	diskinfo->writev_cmd.status = DK_FREE;
	diskinfo->writev_cmd.diskinfo = diskinfo;
	diskinfo->writev_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->writev_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->writev_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;

	
	diskinfo->reassign_cmd.status = DK_FREE;
	diskinfo->reassign_cmd.diskinfo = diskinfo;
	diskinfo->reassign_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->reassign_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->reassign_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;


	diskinfo->dmp_cmd.status = DK_FREE;
	diskinfo->dmp_cmd.diskinfo = diskinfo;
	diskinfo->dmp_cmd.scbuf.flags =
		diskinfo->reset_delay;
	diskinfo->dmp_cmd.scbuf.q_tag_msg = SC_NO_Q;
	diskinfo->dmp_cmd.scbuf.bufstruct.b_event =
		EVENT_NULL;

	return;
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_release_allow					*/
/*									*/
/*	FUNCTION: This routine determines if a release and/or allow	*/
/*		  media removal command is necessary.  If so it will    */
/*		  issue the necessary command(s).			*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		None							*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None							*/
/*									*/
/************************************************************************/

void scdisk_release_allow(struct scdisk_diskinfo  *diskinfo)
{



	if (diskinfo->prevent_eject) {

		/*
		 * We have issued a prevent media removal command
		 * so undo this by issuing an allow media removal
		 * command.
		 * NOTE: scdisk_process_reset will check this
		 * particular case to determine if a release 
		 * command is necessary.
		 */

		diskinfo->disk_intrpt = 1;
                scdisk_prevent_allow_disable(diskinfo,(uchar) DK_ALLOW);
		scdisk_start_disable(diskinfo);
                scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
                             &(diskinfo->open_event));
	
	}
	else if (diskinfo->reserve_lock) {
		/*
		 * We have issued a reserve command with out a 
		 * prevent media removal command. So undo this by issuing
		 * a release command.
		 */
		diskinfo->disk_intrpt = 1;
		scdisk_release_disable(diskinfo);
		scdisk_sleep(diskinfo,&(diskinfo->disk_intrpt),
			     &(diskinfo->open_event));
		
	}

	return;
	
}

/************************************************************************/
/*									*/
/*	NAME:	scdisk_ioctl_mode_sense					*/
/*									*/
/*	FUNCTION:							*/
/*		This routine builds a MODE SENSE cmd     		*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine is called on an interrupt level.		*/
/*		It cannot page fault.					*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		None.							*/
/*									*/
/*									*/
/************************************************************************/
int
scdisk_ioctl_mode_sense(
struct scdisk_diskinfo  *diskinfo,
uchar  			pc,
uchar			page_code,
int			allocation_length)
{
	struct dk_cmd	*cmd_ptr;
	struct buf	*cmd_bp;
	struct scsi	*scsi;
	int		errnoval = 0;

	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 



	/* Initialize cmd block for mode sense cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_options = 0x00;
	cmd_bp->b_work = 0x00;
	cmd_bp->b_error = 0x00;
	cmd_bp->b_blkno = 0x00;
	cmd_bp->b_bcount = allocation_length;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->ioctl_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = DK_TIMEOUT;
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.status_validity = 0x00;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->scbuf.adap_q_status = 0;
	cmd_ptr->scbuf.flags = 0;
	cmd_ptr->retry_count = 0;
	cmd_ptr->segment_count = 0x00;
	cmd_ptr->soft_resid = 0x00;
	cmd_ptr->bp = NULL;

	/* Initialize SCSI cmd for operation */

     /*
      *                            MODE SENSE(6)  Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                       Operation Code (1Ah)                    |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |Resvd  | DBD   |       Reserved        | 
      *|-----+---------------------------------------------------------------|
      *| 2   |      PC       |         Page Code                             |
      *|-----+---------------------------------------------------------------|
      *| 3   |                           Reserved                            |
      *|-----+---------------------------------------------------------------|
      *| 4   |                           Allocation Length                   |
      *|-----+---------------------------------------------------------------|
      *| 5   |                           Control                             |
      *+=====================================================================+
      */

	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 6;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_MODE_SENSE;

        /*
         * Set Page Code byte to request all supported pages....set Page
         * Control bits appropriately depending if we want changeable or
         * current mode data
         */
        scsi->scsi_cmd.scsi_bytes[0] = (page_code | pc);
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = (uchar)cmd_bp->b_bcount;
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;


	/*
	 * Clear out the sense data buffer
	 */
	bzero(diskinfo->ioctl_buf, allocation_length);

       /* Start execution of operation */
        
        
        cmd_ptr->intrpt = 1;
        cmd_ptr->flags |= DK_READY;
        scdisk_start_disable(diskinfo);
        
        /* Wait for completion */
        scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
                     (uint *)&(cmd_bp->b_event));
        
	errnoval = cmd_bp->b_error;

        scdisk_free_cmd_disable(cmd_ptr);

        return (errnoval);

}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_audio_msf					*/
/*									*/
/*	FUNCTION: Issues a play audio msf SCSI command	        	*/
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:	 						        */
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_audio_msf(
struct scdisk_diskinfo	*diskinfo,
int			start_msf,
int			end_msf)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;


	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for read sub-channel SCSI cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);


     /*
      *                         PLAY AUDIO MSF Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                     Operation Code (47h)                      |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |               Reserved                |
      *|-----+---------------------------------------------------------------|
      *| 2   |                        Reserved                               |
      *|-----+---------------------------------------------------------------|
      *| 3   |                        Starting M Field                       |
      *|-----+---------------------------------------------------------------|
      *| 4   |                        Starting S Field                       |
      *|-----+---------------------------------------------------------------|
      *| 5   |                        Starting F Field                       |
      *|-----+---------------------------------------------------------------|
      *| 6   |                        Ending M Field                         |
      *|-----+---------------------------------------------------------------|
      *| 7   |                        Ending S Field                         |
      *|-----+---------------------------------------------------------------|
      *| 8   |                        Ending F Field                         |
      *|-----+---------------------------------------------------------------|
      *| 9   |                        Control                                |
      *+=====================================================================+
      */




	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 0xa;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_PLAY_AUDIO_MSF; 

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = ((start_msf >> 16) & 0xff);
	scsi->scsi_cmd.scsi_bytes[2] = ((start_msf >> 8) & 0xff); 
	scsi->scsi_cmd.scsi_bytes[3] = start_msf & 0xff;
	scsi->scsi_cmd.scsi_bytes[4] = ((end_msf >> 16) & 0xff);
	scsi->scsi_cmd.scsi_bytes[5] = ((end_msf >> 8) & 0xff);
	scsi->scsi_cmd.scsi_bytes[6] = (end_msf & 0xff);
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	scsi->scsi_cmd.scsi_bytes[8] = 0x00;
	scsi->scsi_cmd.scsi_bytes[9] = 0x00;

	

	/* Start execution of operation */


	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	errnoval = cmd_bp->b_error;

	scdisk_free_cmd_disable(cmd_ptr);

	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_audio_trk_indx					*/
/*									*/
/*	FUNCTION: Issues a play audio track/index SCSI command.		*/
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_audio_trk_indx(
struct scdisk_diskinfo	*diskinfo,
uchar			start_trk,
uchar			start_indx,
uchar			end_trk,
uchar			end_indx)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;


	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for read sub-channel SCSI cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);


     /*                         PLAY AUDIO TRACK INDEX Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                     Operation Code (48h)                      |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |               Reserved                |
      *|-----+---------------------------------------------------------------|
      *| 2   |                         Reserved                              |
      *|-----+---------------------------------------------------------------|
      *| 3   |                         Reserved                              |
      *|-----+---------------------------------------------------------------|
      *| 4   |                         Starting Track                        |
      *|-----+---------------------------------------------------------------|
      *| 5   |                         Starting Index                        |
      *|-----+---------------------------------------------------------------|
      *| 6   |                         Reserved                              |
      *|-----+---------------------------------------------------------------|
      *| 7   |                         Ending Track                          |
      *|-----+---------------------------------------------------------------|
      *| 8   |                         Ending Index                          |
      *|-----+---------------------------------------------------------------|
      *| 9   |                         Control                               |
      *+=====================================================================+
      */
  

	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 0xa;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_PLAY_AUDIO_TRK_INDX; 

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = start_trk; 
	scsi->scsi_cmd.scsi_bytes[3] = start_indx;
	scsi->scsi_cmd.scsi_bytes[4] = 0x00;
	scsi->scsi_cmd.scsi_bytes[5] = end_trk;
	scsi->scsi_cmd.scsi_bytes[6] = end_indx;
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	scsi->scsi_cmd.scsi_bytes[8] = 0x00;
	scsi->scsi_cmd.scsi_bytes[9] = 0x00;

	

	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	errnoval = cmd_bp->b_error;

	scdisk_free_cmd_disable(cmd_ptr);

	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_pause_resume					*/
/*									*/
/*	FUNCTION: Issues a Pause/Resume SCSI command.			*/
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_pause_resume(
struct scdisk_diskinfo	*diskinfo,
uchar                   pause_resume_flag)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;


	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for read sub-channel SCSI cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);




        /*                      PAUSE RESUME Command
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+=======================================================|
	 * | 0   |                Operation Code (4Bh)                   |
	 * |-----+-------------------------------------------------------|
	 * | 1   | Logical Unit Number |             Reserved            |
	 * |-----+-------------------------------------------------------|
	 * | 2   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 5   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 6   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 7   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 8   |                Reserved                          |Resu|
	 * |-----+-------------------------------------------------------|
	 * | 9   |                Control                                |
	 * +=============================================================+
	 */


	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 0xa;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_PAUSE_RESUME; 

	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00; 
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	scsi->scsi_cmd.scsi_bytes[4] = 0x00;
	scsi->scsi_cmd.scsi_bytes[5] = 0x00;
	if (pause_resume_flag == DK_RESUME) {
		/*
		 * Resume command
		 */
		scsi->scsi_cmd.scsi_bytes[6] = 0x1;
	}
	else {
		/*
		 * Pause command
		 */
		scsi->scsi_cmd.scsi_bytes[6] = 0x0;
	}
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	scsi->scsi_cmd.scsi_bytes[8] = 0x00;
	scsi->scsi_cmd.scsi_bytes[9] = 0x00;



	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	errnoval = cmd_bp->b_error;

	scdisk_free_cmd_disable(cmd_ptr);

	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_read_subchnl					*/
/*									*/
/*	FUNCTION: Issues a read sub-channel SCSI command		*/
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_read_subchnl(
struct scdisk_diskinfo	*diskinfo,
uchar			sub_chnl_fmt,
uchar			trk_num,
uchar			subq,
uchar			msf,
int			allocation_length)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;


	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for read sub-channel SCSI cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
	cmd_bp->b_flags = (B_READ | B_BUSY);
	cmd_bp->b_bcount = allocation_length;
	cmd_bp->b_un.b_addr = (caddr_t)diskinfo->ioctl_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);



        /* 
	 *		READ SUB-CHANNEL Command
	 * +=====-======-======-======-======-======-======-======-======+
	 * |  Bit|   7  |   6  |   5  |   4  |   3  |   2  |   1  |   0  |
	 * |Byte |      |      |      |      |      |      |      |      |
	 * |=====+=======================================================|
	 * | 0   |                Operation Code (42h)                   |
	 * |-----+-------------------------------------------------------|
	 * | 1   | Logical Unit Number|    Reserved        |  MSF |Reserv|
	 * |-----+-------------------------------------------------------|
	 * | 2   |Reserv| SubQ |            Reserved                     |
	 * |-----+-------------------------------------------------------|
	 * | 3   |                Sub-channel Data Format                |
	 * |-----+-------------------------------------------------------|
	 * | 4   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 5   |                Reserved                               |
	 * |-----+-------------------------------------------------------|
	 * | 6   |                Track Number                           |
	 * |-----+-------------------------------------------------------|
	 * | 7   | (MSB)                                                 |
	 * |-----+---             Allocation Length                   ---|
	 * | 8   |                                                 (LSB) |
	 * |-----+-------------------------------------------------------|
	 * | 9   |                Control                                |
	 * +=============================================================+
	 */


	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 0xa;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_READ_SUB_CHANNEL; 
	scsi->scsi_cmd.lun |= (msf & 0x2);
	scsi->scsi_cmd.scsi_bytes[0] = (subq << 6) & 0x40;
	scsi->scsi_cmd.scsi_bytes[1] = sub_chnl_fmt;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00; 
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	scsi->scsi_cmd.scsi_bytes[4] = trk_num;
	scsi->scsi_cmd.scsi_bytes[5] = 0x00;
	scsi->scsi_cmd.scsi_bytes[6] = allocation_length;
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	scsi->scsi_cmd.scsi_bytes[8] = 0x00;
	scsi->scsi_cmd.scsi_bytes[9] = 0x00;

	

	/*
	 * Clear out the sense data buffer
	 */
	bzero(diskinfo->ioctl_buf, allocation_length);

	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	

	errnoval = cmd_bp->b_error;

	scdisk_free_cmd_disable(cmd_ptr);

	return (errnoval);
}


/************************************************************************/
/*									*/
/*	NAME:   scdisk_read_toc						*/
/*									*/
/*	FUNCTION: Issues a read TOC SCSI command			*/
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*	NOTES:								*/
/*									*/
/*	DATA STRUCTURES:						*/
/*		diskinfo	Disk device specific information	*/
/*									*/
/*	INPUTS:								*/
/*		diskinfo   - Disk device specific information		*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		cmd_bp->b_error						*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*		NONE							*/
/*									*/
/*									*/
/************************************************************************/

int
scdisk_read_toc(
struct scdisk_diskinfo	*diskinfo,
uchar			start_trk,
uchar			msf,
int			allocation_length)
{
	int		errnoval = 0;
	struct dk_cmd	*cmd_ptr;
	struct sc_buf	*scbuf;
	struct scsi	*scsi;
	struct buf	*cmd_bp;
	char		amr_flg;


	cmd_ptr = scdisk_cmd_alloc_disable(diskinfo,(uchar) DK_IOCTL);
        cmd_ptr->subtype = DK_ERP_IOCTL; 


	/* Initialize cmd block for read sub-channel SCSI cmd */
	cmd_bp = &(cmd_ptr->scbuf.bufstruct);
	cmd_bp->b_dev = diskinfo->adapter_devno;
        cmd_bp->b_flags = (B_READ | B_BUSY);
        cmd_bp->b_bcount = allocation_length;
        cmd_bp->b_un.b_addr = (caddr_t)diskinfo->ioctl_buf;
	cmd_bp->b_xmemd.aspace_id = XMEM_GLOBAL;
	cmd_bp->b_iodone = ((void(*)())scdisk_iodone);
	cmd_ptr->scbuf.timeout_value = diskinfo->start_timeout; 
	cmd_ptr->scbuf.bp = NULL;
	cmd_ptr->scbuf.q_tag_msg = SC_NO_Q;
	cmd_ptr->bp = NULL;

	scbuf = &(cmd_ptr->scbuf);



     /*                         READ TOC Command
      *+=====-=======-=======-=======-=======-=======-=======-=======-=======+
      *|  Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
      *|Byte |       |       |       |       |       |       |       |       |
      *|=====+===============================================================|
      *| 0   |                     Operation Code (43h)                      |
      *|-----+---------------------------------------------------------------|
      *| 1   | Logical Unit Number   |            Reserved   |  MSF  |  RSVD |
      *|-----+---------------------------------------------------------------|
      *| 2   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 3   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 4   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 5   |                            Reserved                           |
      *|-----+---------------------------------------------------------------|
      *| 6   |                         Starting Track                        |
      *|-----+---------------------------------------------------------------|
      *| 7   | (MSB)                                                         |
      *|-----+---                    Allocation Length                  -----|
      *| 8   |                                                       (LSB)   |
      *|-----+---------------------------------------------------------------|
      *| 9   |                           Control                             |
      *+=====================================================================+
      */



	/* Initialize SCSI cmd for operation */
	scsi = &(cmd_ptr->scbuf.scsi_command);
	scsi->scsi_length = 0xa;
	scsi->scsi_id = diskinfo->scsi_id;
	scsi->flags = (ushort) diskinfo->async_flag;
	scsi->scsi_cmd.scsi_op_code = SCSI_READ_TOC; 
	scsi->scsi_cmd.lun |= (msf & 0x2);
	scsi->scsi_cmd.scsi_bytes[0] = 0x00;
	scsi->scsi_cmd.scsi_bytes[1] = 0x00;
	scsi->scsi_cmd.scsi_bytes[2] = 0x00; 
	scsi->scsi_cmd.scsi_bytes[3] = 0x00;
	scsi->scsi_cmd.scsi_bytes[4] = start_trk;
	scsi->scsi_cmd.scsi_bytes[5] = 0x00;
	scsi->scsi_cmd.scsi_bytes[6] = allocation_length;
	scsi->scsi_cmd.scsi_bytes[7] = 0x00;
	scsi->scsi_cmd.scsi_bytes[8] = 0x00;
	scsi->scsi_cmd.scsi_bytes[9] = 0x00;

	

	/*
	 * Clear out the sense data buffer
	 */
	bzero(diskinfo->ioctl_buf, allocation_length);

	/* Start execution of operation */
	
	
	cmd_ptr->intrpt = 1;
	cmd_ptr->flags |= DK_READY;
	scdisk_start_disable(diskinfo);
	
	/* Wait for completion */
	scdisk_sleep(diskinfo,(uchar *)&(cmd_ptr->intrpt),
		     (uint *)&(cmd_bp->b_event));
	
	
	errnoval = cmd_bp->b_error;


	
	scdisk_free_cmd_disable(cmd_ptr);

	return (errnoval);
}

/************************************************************************/
/*									*/
/*	NAME:   scdisk_pin_buffer					*/
/*									*/
/*	FUNCTION: Attempts to pin a user buffer for all or a portion of */
/*		  specified length.                                     */
/*		  							*/
/*									*/
/*	EXECUTION ENVIRONMENT:						*/
/*		This routine can be called by a process.		*/
/*		It can page fault only if called under a process and	*/
/*		the stack is not pinned.				*/
/*									*/
/*									*/
/*	INPUTS:								*/
/*		buffer_addr  - Buffer address.				*/
/*		length	     - Length of buffer.			*/
/*		segflag	     - segmetnt flag.				*/
/*		blocksize    - Device's block size.			*/
/*									*/
/*	RETURN VALUE DESCRIPTION:					*/
/*		The errno values listed in the 'error description'	*/
/*		will be returned to the caller if there is an error.	*/
/*		a value of zero will be returned to indicate		*/
/*		successful completion.					*/
/*									*/
/*	ERROR DESCRIPTION:						*/
/*		The following errno values may be returned:		*/
/*		EFAULT - pinu failed.					*/
/*									*/
/*	EXTERNAL PROCEDURES CALLED:					*/
/*									*/
/*		pinu							*/
/*									*/
/************************************************************************/

int
scdisk_pin_buffer(
    caddr_t         buffer_addr,
    int             *length,
    short           segflag,
    register int    blocksize)

{
     	int             rc;

	/* 
	 * This routine will attempt to pin a specified length    
	 * (*length) of the user buffer.  The size of this buffer 
	 * pin will be reduced (if pinu fails) until the *length  
	 * filed is reduced to 0.                                 
	 */

	/* 
	 * Attempt to pin the full request length of the buffer.  
	 */
	rc = pinu(buffer_addr, *length, segflag);
	if (rc != 0) {  
     		/* 
		 * pinu failed 

		 */
	  	/*
		 * If the drive is configured to use variable length  
		 * records, fail here.  ALL of the buffer must be pin-
		 * ned when using variable blocksizes.     
		 */
	  	if (blocksize == 0) {
	    		return (rc);
		}
		/* 
		 * Reduces the requested pin length by half and in-   
		 * sures that the reduced length is on a block        
		 * boundary.                                          
		 */
		*length = ((*length >> 1) - ((*length >> 1) % blocksize));
		while (*length != 0) {
		  	/* 
			 * As long as pinu fails, this loop will reduce  
			 * length by half (always on a block boundary)    
			 * until length is reduced to zero.               
			 */
		  	rc = pinu(buffer_addr, *length, segflag);
			if (rc == 0) {   
			  	/* 
				 * success 
				 */
		    		return (0);
		      	}
			*length = 
				((*length >> 1) - ((*length >> 1) % blocksize));
		}
		/* 
		 * Pinu was unable to pin any of the requested buffer.
		 * The last error received from pinu is returned to   
		 * the calling routine.                               
		 */
		return (rc);
	}
	return (0);  /* success */
}
