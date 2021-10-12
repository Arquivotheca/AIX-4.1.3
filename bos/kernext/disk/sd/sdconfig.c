static char sccsid[] = "@(#)90	1.15  src/bos/kernext/disk/sd/sdconfig.c, sysxdisk, bos411, 9428A410j 5/17/94 07:48:51";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_config(), sd_adap_config(), sd_ctrl_config(), sd_dasd_config()
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>
#include <sys/ddtrace.h>
#include <sys/errno.h>
#include <sys/lockl.h>
#include <sys/lock_alloc.h>
#include <sys/lockname.h>
#include <sys/malloc.h>
#include <sys/signal.h>
#include <sys/trchkid.h>
#include <sys/uio.h>
#include <sys/sd.h>

/*
 ****************  External Variable Declarations  ****************************
 */

extern int      nodev();                 /* for null device assignments */
extern struct cdt  *sd_adap_cdt;
extern struct cdt  *sd_ctrl_cdt;
extern struct cdt  *sd_dasd_cdt;
extern char     sd_inited_adaps; /* number of inited adapters in system */
extern char     sd_inited_ctrls; /* number of inited ctrls in system */
extern char     sd_inited_dasd; /* number of inited dasd in system */
extern uchar	sd_open_adaps;
extern uchar   	sd_open_ctrls; /* number of opened ctrls in system */
extern uchar   	sd_open_dasd; /* number of opened dasd in system */
extern Simple_lock sd_epow_lock; /* Global lock for EPOW         */

#ifdef DEBUG
extern	uint	*sd_trace;
#endif

/*
 * NAME: sd_config
 *
 * FUNCTION: Config Entry Point for  Serial Dasd Subsytem Device Driver
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is called by the configuration process at the
 *      process level, and it can page fault.
 *
 * (RECOVERY OPERATION:) If a failure occurs, the appropriate errno is
 *      returned and handling of the error is the callers responsibility.
 *
 * RETURNS:  Result of Adapter, Controller or DASD config routine
 */

int	sd_config(
dev_t	devno,
int	op,
struct	uio *uiop)
{
	char	device;		/* device type */
	int	min,
		rc;		/* return code */


	DDHKWD5(HKWD_DD_SERDASDD, DD_ENTRY_CONFIG, 0, devno, op, uiop, 0, 0);

	lockl((&sd_global_lock), LOCK_SHORT); /* serialize using global lock */
	/* 
	 * get minor number 
	 */
	min = minor(devno);

	if (min & SD_DASD_MASK)
		/*
		 * if this is a dasd call
		 */
		rc = sd_dasd_config(devno, op, uiop);
	else if (min & SD_CTRL_MASK)
		/*
		 * else if this is a controller call
		 */
		rc = sd_ctrl_config(devno, op, uiop);
	else
		/*
		 * else this is an adapter call
		 */
		rc = sd_adap_config(devno, op, uiop);

	unlockl(&sd_global_lock);	/* unlock the global lock */
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_CONFIG, rc, devno);
	return(rc);			/* return our return code */
}
	
/*
 * NAME: sd_adap_config
 *
 * FUNCTION: Configures, Terms, or Querys VPD for Serial DASD Adapter
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config to handle an adapter config call.
 *
 * (NOTES:) Possible operations :
 *
 *	CFG_INIT:	Configures the adapter, makes ready for use 
 *	CFG_TERM:	Unconfigures the adapter
 *	CFG_QVPD:	Queries for the adapter Vital Product Data
 *
 * (RECOVERY OPERATION:)  If a failure occures, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	uio - structure used for dds data and vpd data
 *			sd_adap_info - adapter info structure
 *			sd_adap_dds - adapter device define structure
 *			devsw - device switch table setup
 *
 * RETURNS: 	0         - successful completion
 *      		  - already unconfigured on terminate
 *		EIO	  - devswadd failure
 *			  - uiomove failure
 * 			  - error setting up adapter POS regs
 * 			  - couldn't exec daemon              
 *			  - adapter not configured on QVPD
 *  			  - coudn't read VPD
 *		EINVAL	  - adapter already configured
 *			  - invalid dds data		
 *			  - invalid operation
 * 		ENOMEM	  - couldn't malloc adapter info structure
 *		EBUSY	  - device still busy on terminate
 *		EPERM	  - Device Driver can't support this level adapter
 */

int sd_adap_config(
dev_t	devno,
int	op,
struct uio *uiop)
{
	int 	i,
		tcws,
		tcw_words,
		rc;		/* return code */
	uchar	vpd[SD_ADAP_VPD_SIZE];
	struct 	devsw tempdevsw; /* temporary, local devsw table */
	struct	sd_adap_info *ap;
	struct  sd_adap_dds local_dds;
	uint	base;
	int	min;


	switch (op) {
		case CFG_INIT:
			/*
			 * Initialize
			 */
			if (sd_inited_adaps == 0) {
			       
				/* 
				 * if first config call to adapters
				 * initialize local devsw table 
				 * sd_open, sd_close, sd_ioctl sd_config        
				 */
				tempdevsw.d_open = (int(*)())sd_open;
				tempdevsw.d_read = (int(*)())sd_read;
				tempdevsw.d_write = (int(*)())sd_write;
				tempdevsw.d_close = (int(*)())sd_close;
				tempdevsw.d_ioctl = (int(*)())sd_ioctl;
				tempdevsw.d_strategy = (int(*)())sd_strategy;
				tempdevsw.d_ttys = (struct tty *)NULL;
				tempdevsw.d_select = (int(*)())nodev;
				tempdevsw.d_config = (int(*)())sd_config;
				tempdevsw.d_print = (int(*)())nodev;
				tempdevsw.d_dump = (int(*)())sd_dump;
				tempdevsw.d_mpx = (int(*)())nodev;
				tempdevsw.d_revoke = (int(*)())nodev;
				tempdevsw.d_dsdptr = NULL;
				tempdevsw.d_opts = DEV_MPSAFE;

				/* 
				 * add this device to device switch table
				 */
				rc = devswadd (devno, &tempdevsw);
				if (rc != 0) { 
					/*
					 * if error adding to the switch
					 * table, return code.
					 */
					return(EIO);
				}
				/*
				 * clear open adap count for system
				 */
				sd_open_adaps = 0;
				sd_adap_cdt = NULL;
				/*
				 * clear adapter hash table
				 */
				for (i=0; i<SD_ADAP_TBL_SIZE; i++)
					sd_adap_table[i] = 
						(struct sd_adap_info *) NULL;
				
			}
			/* 
			 * get adapter pointer 
			 */
			ap = (struct sd_adap_info *)sd_hash(devno);	 
			if (ap != NULL) { 
				/* 
				 * This adapter is already configured.
				 * so return code
				 */
				if (sd_inited_adaps == 0)	
					devswdel(devno);
				return(EINVAL);			
			}

			rc = uiomove((caddr_t)&local_dds,uiop->uio_resid, 
				UIO_WRITE, uiop); 
			if (rc != 0) {	
				/* 
				 * if move failed 
				 * return code
				 */
				if (sd_inited_adaps == 0)	
					devswdel(devno);
				 return(EIO);
		 	}
			
			/*
			 * perform validation of dds data here ... bus type, 
			 * base address on 4K boundary, interrupt class
			 * and tcw start address.
			 */
                	if ((local_dds.tcw_start_addr & (SD_TCWSIZE - 1)) ||
	                    (local_dds.intr_priority != INTCLASS2) ||
	                    (local_dds.bus_type != BUS_MICRO_CHANNEL) ||
	                    (local_dds.base_addr & 0x3ff)) {
                        	/*
	                         * if a problem found with dds data, 
				 * clean up and exit
				 */
				if (sd_inited_adaps == 0)	
					devswdel(devno);
				return(EINVAL);
                        }
			/*
			 * compute number of tcws 
			 */
			tcws = local_dds.tcw_length / SD_TCWSIZE; 

			/*
			 * allocate memory for this adapter info, 
			 * put in adapter hash table, and initialize key 
			 * variables.
			 */
			ap = sd_alloc_adap(devno, tcws);    
			if (ap == NULL) {
				/* 
				 * if unsuccessful allocation and if no 
				 * adapters left, remove from switch table
				 */
				if (sd_inited_adaps == 0)
					devswdel( devno);
				return(ENOMEM);			
			}

			/*
			 * 	Manipulate component Dump table..... 
			 */
			rc = sd_rebuild_cdt(SD_NUM_ADAP_ENTRIES, 
					    sd_inited_adaps+1, 
					    &sd_adap_cdt,"serdasda");
			if (rc) {	   
				/* 
				 * if error configuring adapter, free 
				 * adapter structure, remove from devsw table 
				 * if last one, set return code, and leave.
				 */
				sd_free_adap(ap);
				if (sd_inited_adaps == 0)
					devswdel( devno);
				return(EIO);
			}
		 
			/*
			 * copy local dds to adapter structure
			 */
			bcopy(&local_dds, &(ap->dds), 
				sizeof(struct sd_adap_dds));

			/*
			 * initialize interrupt structure
			 * and timer interrupt priority
			 */
        		ap->sdi.intr_st.bus_type = ap->dds.bus_type;
		        ap->sdi.intr_st.level = ap->dds.intr_lvl;
		        ap->sdi.intr_st.priority = ap->dds.intr_priority;
		        ap->sdi.intr_st.bid = ap->dds.bus_id;
			ap->sdi.intr_st.flags = INTR_MPSAFE;

        		ap->reset_timer->ipri = ap->dds.intr_priority;
        		ap->halt_timer->ipri = ap->dds.intr_priority;
        		ap->delay_timer->ipri = ap->dds.intr_priority;

        		/*
		         * copy resource name for later error logging
		         */
		        bcopy(ap->dds.resource_name, ap->elog.resource_name, 
				ERR_NAMESIZE);
			
			/*
			 * set maximum transfer size to 1/4 tcw length
			 */
			ap->max_transfer = ap->dds.tcw_length / 4; 
			/* 
			 * calculate TCW specific information ... 
			 */
                	ap->sta_tcw_start = tcws - SD_NUM_STA_TCWS;
                	ap->mb_tcw_start = tcws - SD_NUM_STA_TCWS - 
				SD_NUM_MBA_TCWS;

			/* 
			 * configure adapter POS registers, 
			 * and make ready for first open
			 */
			rc = sd_setup_adap(ap);	
			if (rc != 0) {	
				/*
				 * if error configuring adapter, 
				 * free adapter structure, remove
				 * from devsw table if last one, return code.
				 */
				sd_free_adap(ap);	
				sd_rebuild_cdt(SD_NUM_ADAP_ENTRIES,
					       sd_inited_adaps,
					       &sd_adap_cdt,"serdasda"); 
				if (sd_inited_adaps == 0)
					devswdel( devno);
				return(EIO);
			}
			/* 
			 * increment number of adapters initialized.
			 */
			sd_inited_adaps++;      

			/*
			 * Initialize unconfiguring flag
			 */
			ap->unconfiguring = FALSE;

			/*
			 * Set up Interrupt-thread  synchronization
			 * lock for all serial DASD  on this
			 * adapter.
			 */
			/* 
			 * get minor number 
			 */
			min = minor(devno);
			lock_alloc(&(ap->spin_lock),
					   LOCK_ALLOC_PIN,
				   	   SD_ADAP_LOCK_CLASS,min);

			simple_lock_init(&(ap->spin_lock));

			if (sd_inited_adaps == 1)  {
				/*
				 * If first adapter being configured
				 * then initialize global epow
				 * spin lock.
				 */
				lock_alloc(&(sd_epow_lock),
					   LOCK_ALLOC_PIN,
					   SD_EPOW_LOCK_CLASS,-1);

				simple_lock_init(&(sd_epow_lock));
			}

#ifdef DEBUG
			/*
			 * Malloc Trace Buffer one time only, this is
			 * so we can have the buffer more readable 
			 * 32-byte aligned
			 */
			if (sd_inited_adaps == 1)  {
				sd_trace = (uint *)xmalloc((uint)
				            TRCLNGTH_PTR, 
					5, (heapaddr_t)pinned_heap);
			}
			

			ap->ap_trace = (struct sd_trace *)xmalloc((uint)
                                        (sizeof(struct sd_trace) * TRCLNGTH), 
                                        5, (heapaddr_t)pinned_heap);

			if (sd_inited_adaps <= TRCLNGTH_PTR) {
				/*
				 * If we have room in our list
				 * of adapter trace tables, then
				 * add this one to it.
				 */
				sd_trace[(sd_inited_adaps-1)] = (uint)
					&(ap->ap_trace[0]);
			}

#endif
			break;

		case CFG_TERM:
			/*
			 * Terminate 
			 */
			
			/*
			 * get pointer to this adapters info structure
			 */
			ap = (struct sd_adap_info *)sd_hash(devno);
			if (ap == NULL)  {
				/*
				 * already unconfigured
				 */
				return(0);
			}

			if (ap->fs_open || ap->inited_ctrls) {
				/* 
				 * if this adapter is still open (file system), 
				 * or has children configured, return code	
				 */
				return(EBUSY);
			}

			/*
			 * Wake up daemon with NULL event, so he will die.
			 */
			ap->unconfiguring = TRUE;
			ap->event_head = NULL;
			if (ap->daemon_open)
				pidsig(ap->daemon_pid,SIGKILL);

			/*
			 * delay to allow daemon to die and unlock so the
			 * daemon can close the adapter
			 */
			 
			while (ap->daemon_open) {
				unlockl(&sd_global_lock);
				delay(HZ);
				lockl((&sd_global_lock), LOCK_SHORT); 
			}

			/*
			 * Wait for daemon close to finish
			 */
			delay(4*HZ);

			/*
			 * Now that daemon has closed
			 * adapter we can lock again
			 */
			
			/* Manipulate Component Dump Table .... */
			sd_rebuild_cdt(SD_NUM_ADAP_ENTRIES
				       ,sd_inited_adaps - 1, 
				       &sd_adap_cdt,"serdasda");


			/*
			 * Free Interrupt-thread synchronization 
			 * lock for all serial DASD devices on
			 * this adapter.
			 */
			lock_free(&(ap->spin_lock));

#ifdef DEBUG
			xmfree((caddr_t)(ap->ap_trace),
			       (heapaddr_t)pinned_heap);
#endif

			sd_free_adap(ap); /* free memory for this adapter */
			/* 
			 * decrement number of initialized adapters.
			 */
			sd_inited_adaps--; 


			if (sd_inited_adaps == 0) {
				/*
				 * if last adapter, delete from devsw table.
				 */
				devswdel(devno);

				lock_free(&(sd_epow_lock));


#ifdef DEBUG
				xmfree((caddr_t)sd_trace,
					(heapaddr_t)pinned_heap);
#endif
			}
			break;

		case CFG_QVPD:
			/*
			 * Query Vital Product Data
			 */

			/*
			 * get pointer to this adapters info structure
			 */
			ap = (struct sd_adap_info *)sd_hash(devno);

			if (ap == NULL) {
				/*
				 * this adapter not inited
				 */
				return(EIO);
			}

			/*
			 * get adapters vital product data, and move 
			 * the data to the callers uio structure
			 */
			rc = sd_get_vpd(ap, vpd, 1, (int)SD_ADAP_VPD_SIZE);
			if (rc != 0) {
				/*
				 * Error reading VPD
				 */
				return(EIO);
			}
				
                	/*
	                 * move the VPD to the caller's uio structure
	                 */
	                rc = uiomove((caddr_t)vpd, (int)SD_ADAP_VPD_SIZE, 
				UIO_READ, uiop);
	                if (rc != 0) {
	                        /*
	                         * unsuccessful copy
	                         */
	                        return(EIO);
	                }
			break;

		default:
			return(EINVAL);
	}
	return(0);
}

/*
 * NAME: sd_ctrl_config
 *
 * FUNCTION: Configures, or Terminates Serial DASD Controller
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config to handle a controller config call.
 *
 * (NOTES:) Possible operations :
 *
 *	CFG_INIT:	Configures the controller, makes ready for use 
 *	CFG_TERM:	Unconfigures the controller
 *
 * (RECOVERY OPERATION:)  If a failure occures, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	uio - structure used for dds data and vpd data
 *			sd_ctrl_info - controller info structure
 *			sd_ctrl_dds - controller device define structure
 *
 * RETURNS: 	0         - successful completion
 *      		  - already unconfigured on terminate
 *		EIO	  - uiomove failure
 *			  - this controller's adapter not configured
 *		EINVAL	  - controller already configured
 *			  - invalid dds data		
 *			  - invalid operation
 * 		ENOMEM	  - couldn't malloc controller info structure
 *		EBUSY	  - device still busy on terminate
 */

int sd_ctrl_config(
dev_t	devno,
int	op, 
struct uio *uiop)
{
	int	i,
		rc;	/* return code */
	struct 	devsw tempdevsw; /* temporary, local devsw table */
	struct sd_ctrl_dds local_dds;
	struct 	sd_adap_info *ap;
	struct 	sd_ctrl_info *cp;
	uchar	junk;


	switch (op) {
		case CFG_INIT:
			/* 
			 * Initialize
			 */
			if (sd_inited_ctrls == 0) {
				/* 
				 * clear open controller count
				 */
				sd_open_ctrls = 0; 
				sd_ctrl_cdt = NULL;
				/*
				 * clear controller hash table
				 */
				for (i=0; i<SD_CTRL_TBL_SIZE; i++)
					sd_ctrl_table[i] = 
						(struct sd_ctrl_info *) NULL;
			}
			/*
			 * get pointer to this devnos info structure
			 */
			cp = (struct sd_ctrl_info *)sd_hash(devno);
			if (cp != NULL) {
				/*
				 * This controller is already configured.
				 * return code 
				 */
				return(EINVAL);
			}

			rc = uiomove((caddr_t)&local_dds,uiop->uio_resid, 
				UIO_WRITE, uiop); 
			if (rc != 0) {	
				/* 
				 * if move failed 
				 * return code
				 */
				 return(EIO);
		 	}

			/*
			 * get this controllers adapter pointer
			 */
			ap = (struct sd_adap_info *)
				sd_hash(local_dds.adapter_devno);
			if (ap == NULL)
				/*
				 * Can't find adapter for this controller
				 */
				return(EIO);	

			/*
			 * allocate memory for this controller info, 
			 * put in controller hash table, and initialize 
			 * key variables
			 */  
			cp = sd_alloc_ctrl(devno);
			if (cp == NULL) {
				/*
				 * if unsuccessful allocation 
				 */
				return(ENOMEM);	
			}

			/* 
			 * Manipulate Component Dump Table ....
			 * Allocate memory for component dump table.
			 */

			
			rc = sd_rebuild_cdt(SD_NUM_CTRL_ENTRIES, 
					    sd_inited_ctrls+1, 
					    &sd_ctrl_cdt,"serdasdc");
			if (rc) {	   
				/* 
				 * if error configuring controller, free 
				 * adapter structure, remove from devsw table 
				 * if last one, set return code, and leave.
				 */
				sd_free_ctrl(cp);
				if (sd_inited_ctrls == 0)
					devswdel( devno);
				return(EIO);
			}

			/*
			 * copy local dds to controller structure
			 */
			bcopy(&local_dds, &cp->dds, sizeof(struct sd_ctrl_dds));

        		/*
		         * copy resource name for later error logging
		         */
		        bcopy(cp->dds.resource_name, cp->elog.resource_name, 
				ERR_NAMESIZE);

			/* 
			 * store adapter pointer, and place this controller
			 * in his adapter's ctrllist
			 */
			cp->ap = ap;	
        		cp->delay_timer->ipri = ap->dds.intr_priority;
			ap->ctrllist[cp->dds.target_id] = cp;
			/*
			 * increment number of controllers initialized 
			 * on this adapter .
			 */
			ap->inited_ctrls++;
			/*
			 * increment number of controllers initialized
			 * in the system.
			 */
			sd_inited_ctrls++;

			break;

		case CFG_TERM:
			/*
			 * Terminate
			 */
			
			/*
			 * get pointer to this controllers info structure
			 */
			cp = (struct sd_ctrl_info *)sd_hash(devno);
			if (cp == NULL) 
				/*
				 * this controller already unconfigured
				 */
				return(0);

			if ((cp->opened) || (cp->inited_dasd)) {
				/*
				 * if this controller is still open, 
				 * or has children configured, return code
				 */
				return(EBUSY);
			}

			/*
			 * decrement number of controllers on this adapter.
			 * and remove this one from his adapters ctrl list
			 */
			cp->ap->inited_ctrls--;
			cp->ap->ctrllist[cp->dds.target_id] = NULL;

			/* 
			 * Manipulate Component Dump Table .... 
			 */
			sd_rebuild_cdt(SD_NUM_CTRL_ENTRIES
				       ,sd_inited_ctrls - 1, 
				       &sd_ctrl_cdt,"serdasdc");
			sd_free_ctrl(cp); /* free memory for this controller */
			/*
			 * decrement number of initialized controllers in
			 * the system.
			 */
			sd_inited_ctrls--;
			
			break;

		default:
			return(EINVAL);
	}
	return(0);

}


/*
 * NAME: sd_dasd_config
 *
 * FUNCTION: Configures, or Terminates Serial DASD 
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config to handle a controller config call.
 *
 * (NOTES:) Possible operations :
 *
 *	CFG_INIT:	Configures the dasd, makes ready for use 
 *	CFG_TERM:	Unconfigures the dasd
 *
 * (RECOVERY OPERATION:)  If a failure occures, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	uio - structure used for dds data and vpd data
 *			sd_dasd_info - dasd info structure
 *			sd_dasd_dds - dasd device define structure
 *
 * RETURNS: 	0         - successful completion
 *      		  - already unconfigured on terminate
 *		EIO	  - uiomove failure
 *			  - this dasd's controller not configured
 *		EINVAL	  - dasd already configured
 *			  - invalid dds data		
 *			  - invalid operation
 * 		ENOMEM	  - couldn't malloc dasd info structure
 *		EBUSY	  - device still busy on terminate
 */

int sd_dasd_config(
dev_t	devno,
int	op, 
struct uio *uiop)
{
	int	i,
		rc;	/* return code */
	struct 	devsw tempdevsw; /* temporary, local devsw table */
	struct sd_dasd_dds local_dds; 
	struct	sd_ctrl_info *cp;
	struct	sd_dasd_info *dp;
	uchar	junk;


	switch (op) {
		case CFG_INIT:
			/*
			 * Initialize
			 */
			if (sd_inited_dasd == 0) { 
				sd_open_dasd = 0; /* clear open dasd count */
				sd_dasd_cdt = NULL;
				/*
				 * clear dasd hash table
				 */
				for (i=0; i<SD_DASD_TBL_SIZE; i++)
					sd_dasd_table[i] = 
						(struct sd_dasd_info *) NULL;
			}
			/*
			 * get pointer to this dasd info structure
			 */
			dp = (struct sd_dasd_info *)sd_hash(devno);
			if (dp != NULL) {
				/*
				 * This dasd is already configured.
				 * return code
				 */
				return(EINVAL);
			}

			rc = uiomove((caddr_t)&local_dds,uiop->uio_resid, 
				UIO_WRITE, uiop); 
			if (rc != 0) {	
				/* 
				 * if move failed 
				 * return code
				 */
				 return(EIO);
		 	}
			/* 
			 * make sure queue depth is workable, minimum is 1
			 * maximum is 63 (due to controller limit)
			 */
			if (local_dds.queue_depth <= 0)	
				local_dds.queue_depth = 1;
			if (local_dds.queue_depth > 63)
				local_dds.queue_depth = 63;

			/*
			 * get this dasd's controller pointer
			 */
			cp = (struct sd_ctrl_info *)
				sd_hash(local_dds.controller_devno);
			if (cp == NULL)
				/*
				 * Can't find controller for this dasd
				 */
				return(EIO);

			/*
			 * allocate memory for this dasd info, put in dasd   
			 * hash table, and initialize key variables
			 */
			dp = sd_alloc_dasd(devno,local_dds.queue_depth);
			if (dp == NULL) {
				/*
				 * if unsuccessful allocation
				 */
				return(ENOMEM);		
			}

			/*
			 * Manipulate Component Dump Table ....
			 * Allocate memory for component dump table.
			 */
			rc = sd_rebuild_cdt(SD_NUM_DASD_ENTRIES, 
					    sd_inited_dasd+1, 
					    &sd_dasd_cdt,"serdasdd");
			if (rc) {	   
				/* 
				 * if error configuring controller, free 
				 * adapter structure, remove from devsw table 
				 * if last one, set return code, and leave.
				 */
				sd_free_dasd(dp);
				if (sd_inited_dasd == 0)
					devswdel( devno);
				return(EIO);
			}


			/*
			 * copy local dds to dasd structure
			 */
			bcopy(&local_dds, &dp->dds, sizeof(struct sd_dasd_dds));

        		/*
		         * copy resource name for later error logging
		         */
		        bcopy(dp->dds.resource_name, dp->elog.resource_name, 
				ERR_NAMESIZE);

			/*
			 * format the data base specified mode select data
			 */
			sd_format_mode_data(dp->dds.mode_data, &dp->dd, 
				dp->dds.mode_data_length);

			/*
			 * format the data base specified mode default data
			 */
			sd_format_mode_data(dp->dds.mode_default_data, &dp->df, 
				dp->dds.mode_default_length);
			/* 
			 * Register iostat service 
			 */
			dp->dkstat.dk_bsize = SD_BPS;
			bcopy(dp->dds.resource_name, dp->dkstat.diskname,8);
			iostadd( DD_DISK, &(dp->dkstat));

			/* 
			 * store controller pointer, and place this dasd
			 * in his controllers dasd list
			 */
			dp->cp = cp;	
			dp->ap = cp->ap;
        		dp->delay_timer->ipri = dp->ap->dds.intr_priority;
			cp->dasdlist[dp->dds.lun_id] = dp;
			/*
			 * increment number of dasd on this 
			 * controller initialized.
			 */
			cp->inited_dasd++;
			/*
			 * increment number of dasd initialized in this system.
			 */
			sd_inited_dasd++;
			break;

		case CFG_TERM:
			/*
			 * Terminate
			 */
			
			/*
			 * get pointer to this dasd's info structure
			 */ 
			dp = (struct sd_dasd_info *)sd_hash(devno);
			if (dp == NULL) 
				/*
				 * this dasd already unconfigured
				 */
				return(0);

			if (dp->opened) {
				/*
				 * if this dasd is still open return code
				 */
				return(EBUSY);
			}


			/*
			 * De-register iostat service ...
			 */
			iostdel( &(dp->dkstat));

			/*
			 * Manipulate Component Dump Table ....
			 */
			rc = sd_rebuild_cdt(SD_NUM_DASD_ENTRIES, 
					    sd_inited_dasd-1, 
					    &sd_dasd_cdt,"serdasdd");

			/*
			 * decrement number of dasd on this controller.
			 * and remove this dasd from his controllers dasd list
			 */
			dp->cp->inited_dasd--;
			dp->cp->dasdlist[dp->dds.lun_id] = NULL;

			sd_free_dasd(dp); /* free memory for this dasd */
			/* 
			 * decrement number of initialized 
			 * dasd in the system
			 */
			sd_inited_dasd--; 
			break;

		default:
			return(EINVAL);
	}
	return(0);

}

