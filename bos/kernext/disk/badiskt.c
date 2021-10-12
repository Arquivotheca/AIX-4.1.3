static char sccsid[] = "@(#)38	1.13  src/bos/kernext/disk/badiskt.c, sysxdisk, bos411, 9428A410j 8/30/90 14:16:57";

/*
 * COMPONENT_NAME: (SYSXDISK) Bus Attached Disk Device Driver
 *                            Top Half - Process Level
 *
 * FUNCTIONS: ba_config(), ba_open(), ba_close(), ba_read(),
 *            ba_write(),  ba_ioctl(), ba_mincnt(), ba_pos_setup()  
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

#include <sys/adspace.h>
#include <sys/buf.h> 
#include <sys/ddtrace.h>
#include <sys/device.h> 
#include <sys/devinfo.h> 
#include <sys/dma.h> 
#include <sys/errids.h>
#include <sys/errno.h> 
#include <sys/intr.h> 
#include <sys/ioacc.h> 
#include <sys/ioctl.h> 
#include <sys/lockl.h> 
#include <sys/lvdd.h> 
#include <sys/malloc.h> 
#include <sys/param.h> 
#include <sys/pin.h> 
#include <sys/priv.h> 
#include <sys/sysdma.h> 
#include <sys/sysmacros.h> 
#include <sys/timer.h> 
#include <sys/trchkid.h>
#include <sys/types.h> 
#include <sys/uio.h> 
#include <sys/watchdog.h> 
#include "badiskdd.h" 

#ifdef DEBUG
extern int	badebug;                 /* debug flag for print statements */
#endif

extern int	nodev();                 /* for null device assignments */

/*
 ****************  Static Variable Declarations  ****************************
 */

static char	ba_units_open = 0; /* count of open disks in system */
extern char	ba_count;	   /* count of disks in system */
extern lock_t ba_lock;	           /* code lock */
extern struct ba_info *ba_list[BA_NDISKS];   /* list of info structures */
extern struct cdt *ba_cdt;
struct intr *ba_epow_ptr;          /* pointer to epow structure */


/*
 * NAME: ba_config
 *                                                                    
 * FUNCTION: Configures Bus Attached Disk Device Driver
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by the configuration process at the
 *	process level, and it can page fault.                   
 *                                                                   
 * (NOTES:) Possible operations :                             
 *	CFG_INIT : Adds driver to devsw table, allocates memory for
 *                 driver info, initializes ba_info structure, initializes
 *                 ba_dds structure to dds block passes via *uiop. 
 *      CFG_TERM : Terminates driver, deletes driver from devsw table,
 *                 and frees resources.
 *      CFG_QVPD : Returns Vital Product Data via *uiop.
 *
 * (RECOVERY OPERATION:) If a failure occurs, the appropriate errno is
 *	returned and handling of the error is the callers responsibility.
 *
 * (DATA STRUCTURES:) ba_info - initialized       
 *                    devsw   - driver entry added/deleted
 *                    ba_list - pointer to info structure saved/removed
 *                    uio     - copied
 *                    ba_dds  - initialized
 *
 * RETURNS:  Result of devswadd() if error results
 *           ENXIO     - devno is greater than allowed number of disks
 *                     - attempt to term a non-existent device
 *                     - attempt to query a non-existent device
 *           EINVAL    - device already configured
 *                     - attempt to term an open device       
 *                     - invalid operation requested          
 *           ENOMEM    - not enough memory to allocate info structure
 *           0         - successful completion                        
 */  

int	ba_config(
dev_t   devno,			     /* major and minor device numbers */
int	op,		             /* choice of operation (INIT,TERM,QVPD) */
struct  uio *uiop)		     /* Pointer to Config Data */
{
	int	dev,                 /* minor number */
                  i,                 /* counter */
                  rc = OK;           /* return code */
	struct devsw tempdevsw;      /* temporary devsw table */
	struct ba_info *ba;          /* pointer to disk info structure */

#ifdef DEBUG 
	BADBUG(("Entering ba_config, %x %x %x\n", devno, op, uiop));
#endif 
	DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_CONFIG, 0, devno, op, uiop, 0, 0);
	dev = minor(devno);    	     /* get minor number */
	if (dev >= BA_NDISKS) {      /* if minor number > allowed # of disks */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CONFIG, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  1  ba_config ENXIO\n"));
#endif 
		return(ENXIO);
	}
	lockl(&ba_lock, LOCK_SHORT);            /* request lock to serialize */

	switch (op) {
	case CFG_INIT :                         /* Config Init */
		if (ba_count == BA_NDISKS) {    /* if already supporting max */
			rc = ENXIO;             /* if so, set rc and leave */
			break;
		}
		if (ba_count == 0) {                /* if first disk */
			/* 
			 * set up my devsw entry 
			 */
			tempdevsw.d_open = (int(*)())ba_open; 
			tempdevsw.d_close = (int(*)())ba_close;
			tempdevsw.d_read = (int(*)())ba_read;
			tempdevsw.d_write = (int(*)())ba_write;
			tempdevsw.d_ioctl = (int(*)())ba_ioctl;
			tempdevsw.d_strategy = (int(*)())ba_strategy;
			tempdevsw.d_ttys = (struct tty *)NULL;
			tempdevsw.d_select = (int(*)())nodev;
			tempdevsw.d_config = (int(*)())ba_config;
			tempdevsw.d_print = (int(*)())nodev;
			tempdevsw.d_dump = (int(*)())ba_dump;
			tempdevsw.d_mpx = (int(*)())nodev;
			tempdevsw.d_revoke = (int(*)())nodev;
			tempdevsw.d_dsdptr = NULL;
			tempdevsw.d_opts = 0;
                        /*
                         * Add driver entry to devsw table 
                         */
			rc = devswadd(devno, &tempdevsw); 
			if (rc) 
				break;                  /* if error, leave */
			for (i = 0; i < BA_NDISKS; i++) /* clear pointer list*/
				ba_list[i] = NULL;
			/*
	                 * get memory for dump table 
	                 */
			if ((ba_cdt = (struct cdt *)xmalloc((uint)
				(sizeof(struct cdt_head) + (8 * sizeof(struct 
				cdt_entry))), 2, (heapaddr_t)pinned_heap)) 
				== NULL) {
				rc = ENOMEM;	      /* not enough memory */
				break;
			}
			bzero(ba_cdt, (sizeof(struct cdt_head) + (8 * 
				sizeof(struct cdt_entry))));
			ba_cdt->cdt_magic = DMP_MAGIC;
			bcopy("badisk", ba_cdt->cdt_name, 6);
			ba_pos_clean();
		} /* if ba_count == 0 */
		ba = ba_list[dev];                    /* get this list entry */
		if (ba != NULL && ba->devno == devno) {/* if already configed*/
			                               /* set rc, leave error*/
			rc = EINVAL;
			break;
		}
		/*
                 * get memory for structure 
                 */
		if ((ba = (struct ba_info *)xmalloc((uint)sizeof(struct 
			ba_info), 2, (heapaddr_t)pinned_heap)) 
			== NULL) {
			rc = ENOMEM;	               /* not enough memory */
			break;
		}
		bzero(ba, sizeof(struct ba_info ));    /* zero out memory */
		/*
                 * copy config info into our structure 
                 */
		uiomove((caddr_t)&ba->dds, (int) sizeof(struct ba_dds ),
			UIO_WRITE, (struct uio *) uiop); 
		ba_list[dev] = ba;                 /* store pointer in list */
		if (ba->dds.intr_priority != INTCLASS2) {
			/*
			 * Validation of interrupt priority passed in
			 * failed, so free memory, remove driver from table,
			 * clear pointer and exit
			 */
			xmfree((char *)ba, (heapaddr_t)pinned_heap);
			xmfree((char *)ba_cdt, (heapaddr_t)pinned_heap);
			if (ba_count == 0) 
				devswdel(devno);
			ba_list[dev] = NULL;  
			rc = EINVAL;	     
 			break;              
		}
		/*
		 * Set up POS registers
		 */
		rc = ba_pos_setup(ba);
		if (rc) {
			/*
			 * POS register setup failed, possibly wrong slot
			 * number passed in, or disk is not present, so free
			 * memory, remove driver from table, clear pointer 
			 * and exit
			 */
			xmfree((char *)ba, (heapaddr_t)pinned_heap);
			xmfree((char *)ba_cdt, (heapaddr_t)pinned_heap);
			if (ba_count == 0) 
				devswdel(devno);
			ba_list[dev] = NULL;   
 			break;              
		}
                /* 
                 * set up intr structure 
                 */
	    	ba->intr_st.next = (struct intr *)NULL;
		ba->intr_st.flags = 0;         /* 0 = shared intr level */
		ba->intr_st.bus_type = ba->dds.bus_type;
		ba->intr_st.level = ba->dds.intr_level;
		ba->intr_st.priority = ba->dds.intr_priority;
		ba->intr_st.bid = ba->dds.bus_id;
		ba->intr_st.handler = (int(*)())ba_intr;
		/*
		 * set up watchdog structure
		 */
		ba->mywdog.watch.restart = 10; /* average timeout 10 sec */ 
		ba->mywdog.watch.func = (void(*)())ba_watch;
		ba->mywdog.ba_pointer = (ulong)ba;/*save pointer for watchdog*/
		/*
		 * set up Device Idle watchdog structure
		 */
		ba->idle.watch.restart = 90; /* average timeout 90 sec */ 
		ba->idle.watch.func = (void(*)())ba_idle;
		ba->idle.ba_pointer = (ulong)ba;/*save pointer for watchdog*/
                /*
                 * copy resource name for later error logging
                 */
		bcopy(ba->dds.resource_name, ba->elog.resource_name, 8);
		ba->devno = devno;            /* devno for this device */
		/*
		 * set up IOSTAT structure and register service
		 */
		ba->dkstat.dk_bsize = BA_BPS;
		bcopy(ba->dds.resource_name, ba->dkstat.diskname,8);
		iostadd( DD_DISK, &(ba->dkstat));
		ba_count++;                   /* increment count of in system*/
		break;

	case CFG_TERM :                     /* Terminate the device */
		ba = ba_list[dev];          /* get pointer to struct */
		if (ba == NULL) {                       
			rc = ENXIO;         /* device not inited */
			break;
		}
		if (ba->opened) {                                           
			rc = EBUSY;         /* device is still open */
			break;
		}
		if (--ba_count == 0) {
			devswdel(devno);    /* if last, delete from devsw tbl*/
			xmfree((char *)ba_cdt, (heapaddr_t)pinned_heap);
		}
		iostdel( &(ba->dkstat));
		xmfree((char *)ba, (heapaddr_t)pinned_heap);/* free memory */
		ba_list[dev] = NULL;        /* take pointer out of list*/
		break;

	case CFG_QVPD :                     /* Query Vital Product Data */
		ba = ba_list[dev];          /* get pointer for this device*/
		if (ba == NULL) {        
			rc = ENXIO;         /* if device not inited */
			break;
		}
		/* 
                 * copy our config info into uio struct 
                 */
		uiomove((caddr_t)&ba->dds, sizeof(struct ba_dds ), UIO_READ, 
			uiop);
		break;

	default : 
		rc = EINVAL;               /* Invalid option, set return code*/
		break;
	} /* switch(op) */
	unlockl(&ba_lock);                 /* unlock ba resources */
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CONFIG, rc, devno);
#ifdef DEBUG 
	BADBUG(("Exiting  2  ba_config %d\n", rc));
#endif 
	return(rc);
}


/*
 * NAME: ba_open  
 *                                                                    
 * FUNCTION: Open Bus Attached Disk                           
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by a process at the process level
 *	through the devsw table, and can page fault.             
 *                                                                   
 * (NOTES:) This routine handles the singularity of a diagnostic open through
 *	the ext parameter, pins the drivers code , registers and initializes
 *      the following :interrupt handler, EPOW handler, DMA channel, watchdog
 *      timer, and system timers. It then prepares the disk for use, and reads
 *      the configuration information from the disk.
 *    
 * (RECOVERY OPERATION:) If a failure occurs, the appropriate errno 
 *	is returned, and the caller is responsible for proper recovery.
 *
 * (DATA STRUCTURES:) ba_info     - initialized      
 *                    badisk_spec - drive characteristics stored
 *                    intr_st     - interrupt handler initialized
 *                    ba_epow_ptr - epow handler structure pointer
 *                    ba_watchdog - watchdog handler initialized
 *                    trb         - timer structures initialized
 *
 * RETURNS: Returns the result of pincode() if error occurs.          
 *          Returns the result of i_init() if error occurs.
 *          Returns the result of ba_issue_cmd() if error occurs.
 *          ENXIO       - devno out of range
 *                      - d_init failure
 *          ENODEV      - valid devno but no such device
 *          EACCES      - open attempted while in diagnostic mode    
 *                      - diagnostic open attempted when device already open
 *          EPERM       - diagnostic open attempted without proper authority
 *          EIO         - unusual device condition
 *          ENOTREADY   - timed out trying to read device characteristics
 */  

int	ba_open(
dev_t   devno,                    /* major/minor descriptor */
int	rwflag,                   /* mode of open read/write/rdonly */
int     chan,                     /* unused */
int     ext)                      /* extended parameter used for diagnos open*/
{
	int	counter = 0,      /* counter */
                dev,              /* minor number */
                rc = OK;          /* return code */
	struct ba_info *ba;       /* pointer to info structure */

#ifdef DEBUG 
	BADBUG(("Entering ba_open %x %x %x %x\n", devno, rwflag, chan, ext));
#endif 
	DDHKWD5(HKWD_DD_BADISKDD,DD_ENTRY_OPEN,0,devno,rwflag,chan,ext,0);
	dev = minor(devno);          /* get minor number for this device */
	if (dev >= BA_NDISKS) {
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  1 ba_open ENXIO\n"));
#endif 
		return(ENXIO);       /* minor number out of range */
	}
	lockl(&ba_lock, LOCK_SHORT); /* lock resources */
	ba = ba_list[dev];           /* get structure pointer for this one*/
	if (ba == NULL) {            
		unlockl(&ba_lock);   /* unlock resources */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, ENODEV, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  2 ba_open ENODEV\n"));
#endif 
		return(ENODEV);      /* dist not configured */
	}
	/*
         * if device in diagnostic mode or diagnostic mode is requested and the
	 * device is already opened, set error code and return   
         */
	if ((ba->diagnos_mode) || (ext && ba->opened)) {
		unlockl(&ba_lock);       /* unlock resources */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, EACCES, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  3 ba_open EACCES\n"));
#endif 
		return(EACCES);
	}
	if (!ba->opened) {
		if (ext) {
			rc = privcheck(RAS_CONFIG);  
			if (rc) {
				unlockl(&ba_lock);
				DDHKWD1(HKWD_DD_BADISKDD,DD_EXIT_OPEN,
					rc,devno);
#ifdef DEBUG 
				BADBUG(("Exiting  4 ba_open EPERM\n"));
#endif 
				return(rc); /*not authorized for diag open*/
			} else
			 {
				ba->diagnos_mode = TRUE;  /* set diag flag */
			}
		} /* if (ext) */

		if (!ba_units_open) {       /* first open to disk */
			if (rc = pincode((int(*)())ba_intr)) {   
				ba->diagnos_mode = FALSE; /* clear diag flag */
				unlockl(&ba_lock);        /* unlock resources*/
				DDHKWD1(HKWD_DD_BADISKDD,DD_EXIT_OPEN,
					rc,devno);
#ifdef DEBUG 
				BADBUG(("Exiting  4a ba_open %d\n", rc));
#endif 
				return(rc);               /* pincode failure */
			} /* pincode failure */
			/*
                	 * get memory for structure 
	                 */
			if ((ba_epow_ptr = (struct intr *)xmalloc((uint)
				sizeof(struct intr), 2, 
				(heapaddr_t)pinned_heap)) 
				== NULL) {
				return(ENOMEM);       /* not enough memory */
			}
			bzero(ba_epow_ptr, sizeof(struct intr ));
			INIT_EPOW((ba_epow_ptr), (int(*)())ba_epow, 
				ba->dds.bus_id);
			if ((rc = i_init(ba_epow_ptr) == INTR_FAIL)) {  
				ba->diagnos_mode = FALSE;/* clear diag flag */
				/* 
				 * unpin modules code
				 */
				unpincode((int(*)())ba_intr);     
				unlockl(&ba_lock);      /* unlock resources */
				DDHKWD1(HKWD_DD_BADISKDD,DD_EXIT_OPEN,
					rc,devno);
#ifdef DEBUG 
				BADBUG(("Exiting  5 ba_open %d\n", rc));
#endif 
				return(rc);                /* i_init failure */
			}
			/*
			 * register dump function
			 */
			dmp_add((struct cdt (*)())ba_dump_func);
		} /* if no ba_units_open */

		/*
                 *  call i_init to initialize the interrupt handler
                 */
		if ((rc = i_init(&ba->intr_st) == INTR_FAIL)) { 
			ba->diagnos_mode = FALSE;      /* clear diag flag */
			if (!ba_units_open) {
				i_clear(ba_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)ba_epow_ptr, (heapaddr_t) 
					pinned_heap);
				/*
				 * unpin modules code 
				 */
				unpincode((int(*)())ba_intr);    
				/*
				 * unregister dump function
				 */
				dmp_del((struct cdt (*)())ba_dump_func);
			}
			unlockl(&ba_lock);             /* unlock resources */
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, rc, devno);
#ifdef DEBUG 
			BADBUG(("Exiting  6 ba_open %d\n", rc));
#endif 
			return(rc);                        /* i_init failure */
		}
		/*
                 *  call d_init to initialize a dma slave service 
                 */
		if ((ba->dma_chn_id = (int)d_init((int)ba->dds.dma_level, 
			(int)DMA_SLAVE, (long)ba->dds.bus_id)) == INTR_FAIL) {
			ba->diagnos_mode = FALSE;      /* clear diag flag */
			if (!ba_units_open) {
				i_clear(ba_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)ba_epow_ptr, (heapaddr_t) 
					pinned_heap);
				/*
				 * unpin modules code 
				 */
				unpincode((int(*)())ba_intr);    
				/*
				 * unregister dump function
				 */
				dmp_del((struct cdt (*)())ba_dump_func);
			}
			i_clear((struct intr *)ba);    /*clear the interrupt */
			unlockl(&ba_lock);             /* unlock resources */
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, ENXIO, devno);
#ifdef DEBUG 
			BADBUG(("Exiting  7 ba_open ENXIO\n"));
#endif 
			return(ENXIO);                 /* d_init failure */
		}
		d_unmask(ba->dma_chn_id);    /* unmask this dma channel */
		ba->opened = TRUE;           /* set opened flag */
		ba->command_cmpl = FALSE;    /* clear command complete flag */
		w_init(&ba->mywdog);         /* init a watchdog timer service*/
		w_init(&ba->idle);           /* init a watchdog timer service*/
		ba->piowtrb = (struct trb *)talloc();  /* allocate the timer */
		ba->piowtrb->flags = 0;      /* for 16 bit PIO transfers */
		ba->piowtrb->ipri = ba->dds.intr_priority;
		ba->piowtrb->func = (void (*)())ba_piowxfer;
		ba->irpttrb = (struct trb *)talloc(); /* allocate the timer */
		ba->irpttrb->flags = 0;      /* for polling for interrupt */
		ba->irpttrb->ipri = ba->dds.intr_priority;
		ba->irpttrb->func = (void (*)())ba_waitirpt;
		ba->cmdtrb = (struct trb *)talloc(); /* allocate the timer */
		ba->cmdtrb->flags = 0;       /* for issuing a command */
		ba->cmdtrb->ipri = ba->dds.intr_priority;
		ba->cmdtrb->func = (void (*)())ba_cmdxfer;
		ba->irpttrb->func_data = (ulong)ba; /* save pointer to struct*/
		ba_waitirpt(ba->irpttrb);    /* start timer to wait for irpt */
		/*
                 *  this is to handle an initial or pending reset condition 
                 * 
		 *  wait for RESET to complete 
                 */
		while ((++counter < BA_TO1200) && (!ba->command_cmpl)) 
			delay(1);
		if (counter == BA_TO1200) { 
			ba->diagnos_mode = FALSE;   /* clear diagnostic flag */
			d_clear(ba->dma_chn_id);    /* clear this dma channel*/
			if (!ba_units_open) {
				i_clear(ba_epow_ptr);/* clear the epow */
				xmfree((char *)ba_epow_ptr, (heapaddr_t) 
					pinned_heap);
				/* 
				 * unpin this mods code
				 */
				unpincode((int(*)())ba_intr); 
				/*
				 * unregister dump function
				 */
				dmp_del((struct cdt (*)())ba_dump_func);
			}
			i_clear((struct intr *)ba);  /* clear the irpt hndlr */
			unlockl(&ba_lock);          /* unlock resources */
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, EIO, devno);
#ifdef DEBUG 
			BADBUG(("Exiting  8 ba_open EIO\n"));
#endif 
			ba->opened = FALSE;         /* clear opened flag */
			w_clear(&ba->mywdog.watch); /* clear watchdog service*/
			w_clear(&ba->idle.watch);   /* clear watchdog service*/
			tstop(ba->piowtrb);         /* stop the timer */
			tfree(ba->piowtrb);         /* free the timer struct */
			ba->piowtrb = NULL;         /* clear pointer */
			tstop(ba->irpttrb);         /* stop the timer */
			tfree(ba->irpttrb);         /* free the timer struct */
			ba->irpttrb = NULL;         /* clear pointer */
			tstop(ba->cmdtrb);          /* stop the timer */
			tfree(ba->cmdtrb);          /* free the timer struct */
			ba->cmdtrb = NULL;          /* clear pointer */
			return(EIO);                /* possible hardware fail*/
		} /* if counter = BA_TO1200 (command timed out) */
		delay(1);                           /* allow regs to settle */
                /*
                 * issue command to device to request device configuration info
                 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_DEV_CONFIG, 
			(ushort)BA_DISK)) {
			ba->diagnos_mode = FALSE;     /* clear diag flag */
			d_clear(ba->dma_chn_id);      /* clear this dma chan */
			if (!ba_units_open) {
				i_clear(ba_epow_ptr);/* clear the epow hndlr*/
				xmfree((char *)ba_epow_ptr, (heapaddr_t) 
					pinned_heap);
				/* 
				 * unpin this mods code
				 */
				unpincode((int(*)())ba_intr);   
				/*
				 * unregister dump function
				 */
				dmp_del((struct cdt (*)())ba_dump_func);
			}
			i_clear((struct intr *)ba);   /* clear the irpt hndlr*/
			ba->opened = FALSE;           /* clear opened flag */
			w_clear(&ba->mywdog.watch);   /* clear watchdog srvce*/
			w_clear(&ba->idle.watch);     /* clear watchdog srvce*/
			tstop(ba->piowtrb);           /* stop the timer */
			tfree(ba->piowtrb);           /* free timer struct*/
			ba->piowtrb = NULL;           /* clear pointer */
			tstop(ba->irpttrb);           /* stop the timer */
			tfree(ba->irpttrb);           /* free  timer struct*/
			ba->irpttrb = NULL;           /* clear the pointer */
			tstop(ba->cmdtrb);            /* stop the timer */
			tfree(ba->cmdtrb);            /* free timer struct*/
			ba->cmdtrb = NULL;            /* clear the pointer */
			unlockl(&ba_lock);            /* unlock resources */
			DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, rc, devno);
#ifdef DEBUG 
			BADBUG(("Exiting  9a ba_open %d\n", rc));
#endif 
			return(rc);
		} /* if issue command fails */ 
		counter = 0;              
                /* 
                 * Wait for command to complete with timeout 
                 */
		while ((++counter < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (counter == BA_TO100) {
			ba->diagnos_mode = FALSE;   /* clear diagnostic flag */
			d_clear(ba->dma_chn_id);    /* clear this dma channel*/
			if (!ba_units_open) {
				i_clear(ba_epow_ptr);/* clear the epow hndlr*/
				xmfree((char *)ba_epow_ptr, (heapaddr_t) 
					pinned_heap);
				/* 
				 * unpin this mods code
				 */
				unpincode((int(*)())ba_intr); 
				/*
				 * unregister dump function
				 */
				dmp_del((struct cdt (*)())ba_dump_func);
			}
			i_clear((struct intr *)ba); /* clear the irpt handler*/
			ba->opened = FALSE;         /* clear opened flag */
			w_clear(&ba->mywdog.watch); /* clear watchdog service*/
			w_clear(&ba->idle.watch);   /* clear watchdog service*/
			tstop(ba->piowtrb);         /* stop the timer */
			tfree(ba->piowtrb);         /* free the timer struct */
			ba->piowtrb = NULL;         /* clear pointer */
			tstop(ba->irpttrb);         /* stop the timer */
			tfree(ba->irpttrb);         /* free the timer struct */
			ba->irpttrb = NULL;         /* clear pointer */
			tstop(ba->cmdtrb);          /* stop the timer */
			tfree(ba->cmdtrb);          /* free the timer struct */
			ba->cmdtrb = NULL;          /* clear pointer */
			unlockl(&ba_lock);          /* unlock resources */
			DDHKWD1(HKWD_DD_BADISKDD,DD_EXIT_OPEN,ENOTREADY,devno);
#ifdef DEBUG 
			BADBUG(("Exiting  10 ba_open ENOTREADY\n"));
#endif 
			return(ENOTREADY);          /* cmd did not complete */
		} /* if (counter==BA_TO100) */
		ba_units_open++;
		/*
		 * Start the Device Idle watchdog timer
		 */
		w_start(&ba->idle.watch);
	} /* if (!ba->opened) */
	unlockl(&ba_lock);                           /* unlock resources */
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_OPEN, 0, devno);
#ifdef DEBUG 
	BADBUG(("Exiting  11 ba_open OK\n"));
#endif 
	return(OK);
}


/*
 * NAME: ba_close
 *                                                                    
 * FUNCTION: Close Bus Attached Disk
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by a process at the process level
 *	through the devsw table and can page fault.             
 *                                                                   
 * (NOTES:) This routine first checks to see if any I/O or other
 *	command is in progress on the device, and if so waits for
 *      completion.  If no other units are open, then it will 
 *      clear the epow handler reservation, and unpin the drivers
 *      code.  Then various flags are cleared, the interrupt,watchdog,
 *      DMA, and system timer services are released, and the devices
 *      interrupts are disabled.
 *
 * (RECOVERY OPERATION:) If a failure occurs, the proper errno is 
 *	returned and recovery is the responsibility of the caller.
 *
 * (DATA STRUCTURES:) ba_info   - various flags and pointers cleared 
 *
 * RETURNS: ENXIO   - invalid devno                                 
 *                  - device not inited
 *              0   - successful completion 
 */  

int	ba_close(
dev_t   devno,                                /* major/minor numbers */
int	chan,                                 /* unused */
int     ext)                                  /* extended parameter, unused */ 
{
	struct ba_info *ba;                   /* pointer to info structure */
	int	dev;                          /* minor number */

#ifdef DEBUG 
	BADBUG(("Entering ba_close %x\n", devno));
#endif 
	DDHKWD1(HKWD_DD_BADISKDD, DD_ENTRY_CLOSE, 0, devno);
	dev = minor(devno);               /* get minor number for device */
	if (dev >= BA_NDISKS) {
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CLOSE, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  1 ba_close ENXIO\n"));
#endif 
		return(ENXIO);             /* minor number out of range */
	} /* if dev >= BA_NDISKS */
	lockl(&ba_lock, LOCK_SHORT);       /* lock resources */
	ba = ba_list[dev];                 /* get info pointer for minor */
	if (ba == NULL) {
		unlockl(&ba_lock);         /* unlock resources */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CLOSE, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  2 ba_close ENXIO\n"));
#endif 
		return(ENXIO);             /* device not inited */
	} /* if ba==NULL */
	if (!ba->opened) {
		/* 
		 * if this disk not open, specifically, this was
		 * added to handle the file systems call of the
		 * close routine upon a failed open. 
		 */
		unlockl(&ba_lock);         /* unlock resources */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CLOSE, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  1a ba_close NOCLOSE\n"));
#endif 
		return(OK);
	}
	/* 
         * wait for outstanding io to complete - while I/O queue not empty,
         *    command in progress, or open as dump device 
         */
	while ((ba->head != NULL) || (!ba->command_cmpl) || (ba->dumpdev))
		delay(10);

	/*
	 * Stop the Device Idle timer
	 */
	w_stop(&ba->idle.watch); 
	ba->diagnos_mode = FALSE;          /* clear diagnostic flag */
	ba->opened = FALSE;                /* clear opened flag */
	if (--ba_units_open == 0) {        /* if this is the last unit in use*/
		i_clear(ba_epow_ptr);     /* clear the epow handler */
		xmfree((char *)ba_epow_ptr, (heapaddr_t) pinned_heap);
		unpincode((int(*)())ba_intr);  /* unpin this modules code */
		/*
		 * unregister dump function
		 */
		dmp_del((struct cdt (*)())ba_dump_func);
	}
	i_clear((struct intr *)ba);        /* clear interrupt handler */
	w_clear(&ba->mywdog.watch);        /* clear watchdog timer service */
	w_clear(&ba->idle.watch);          /* clear watchdog timer service */
	d_clear(ba->dma_chn_id);           /* clear this dma channel */
	tstop(ba->piowtrb);                /* stop the timer */
	tfree(ba->piowtrb);                /* free the timer structure */
	ba->piowtrb = NULL;                /* clear pointer */
	tstop(ba->irpttrb);                /* stop the timer */
	tfree(ba->irpttrb);                /* free the timer structure */
	ba->irpttrb = NULL;                /* clear pointer */
	tstop(ba->cmdtrb);                 /* stop the timer */
	tfree(ba->cmdtrb);                 /* free the timer structure */
	ba->cmdtrb = NULL;                 /* clear pointer */
        /*
         * Disable Interrupts 
         */
	ba->pio_address = ba->dds.base_address + BA_BCR; 
	ba->pio_data = BA_INT_DISABLE;
	ba->pio_size = BA_1BYTE;              
	ba->pio_op = BA_PIO_W;
	pio_assist((caddr_t)ba, ba_piofunc, ba_piorecov);

	unlockl(&ba_lock);                  /* unlock resources */
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_CLOSE, 0, devno);
#ifdef DEBUG 
	BADBUG(("Exiting  3 ba_close OK\n"));
#endif 
	return(OK);
}


/*
 * NAME: ba_read
 *                                                                    
 * FUNCTION: Perform RAW READ on Bus Attached Disk            
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by a process at the process level
 *	through the devsw table and can page fault.             
 *                                                                   
 * (NOTES:) This routine calls uphysio to convert the data to be
 *	read into a buf structure to be passed to the ba_strategy
 *      routine. Uphysio also calls ba_mincnt which preserves any
 *      extended parameters received in the read system call.
 *
 * (RECOVERY OPERATION:) If a failure occurs, the proper errno is
 *	returned and the caller is left responsible for recovery.
 *
 * (DATA STRUCTURES:) NONE    
 *
 * RETURNS: Returns the result of uphysio() if error occurs.        
 */  

int	ba_read(
dev_t   devno,                     /* major/minor numbers */
struct  uio *uiop,                 /* pointer to struct w/ xfer specifics */
int	chan,                      /* channel, unused */
int     ext)                       /* extended parameter for special requests*/
{
	int	rc;                /* return code */

#ifdef DEBUG 
	BADBUG(("Entering ba_read %x %x\n", devno, uiop));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_ENTRY_READ, 0, devno);

	/*
         * call uphysio to handle RAW I/O with my strategy and mincnt routines
         */

	rc = uphysio(uiop, B_READ, 1, devno, ba_strategy, ba_mincnt, ext);

#ifdef DEBUG 
	BADBUG(("Exiting ba_read %d\n", rc));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_READ, rc, devno);
	return(rc);
}


/*
 * NAME: ba_write
 *                                                                    
 * FUNCTION: Perform RAW WRITE on Bus Attached Disk            
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by a process at the process level
 *	through the devsw table and can page fault.             
 *                                                                   
 * (NOTES:) This routine calls uphysio to convert the data to be
 *	written into a buf structure to be passed to the ba_strategy
 *      routine. Uphysio also calls ba_mincnt which preserves any
 *      extended parameters received in the write system call.
 *
 * (RECOVERY OPERATION:) If a failure occurs, the proper errno is
 *	returned and the caller is left responsible for recovery.
 *
 * (DATA STRUCTURES:) NONE       
 *
 * RETURNS: Returns the result of uphysio() if error occurs.        
 */  

int	ba_write(
dev_t   devno,                   /* major/minor numbers */
struct  uio *uiop,               /* pointer to structure w/ xfer specifics */
int	chan,                    /* channel, unused */
int     ext)                     /* extended parameter for special requests */
{
	int	rc;              /* return code */

#ifdef DEBUG 
	BADBUG(("Entering ba_write %x %x\n", devno, uiop));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_ENTRY_WRITE, 0, devno);

	/*
         * call uphysio to handle RAW I/O with my strategy and mincnt routines
         */

	rc = uphysio(uiop, B_WRITE, 1, devno, ba_strategy, ba_mincnt, ext);

#ifdef DEBUG 
	BADBUG(("Exiting ba_write %d\n", rc));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_WRITE, rc, devno);
	return(rc);
}


/*
 * NAME: ba_ioctl
 *                                                                    
 * FUNCTION: Performs Bus Attached Disk Specific Ioctls       
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called by a process at the process level
 *	and it can page fault.                                  
 *                                                                   
 * (NOTES:) This routine performs the following Bus Attached Disk
 *	Ioctls :                                             
 *          IOCINFO     - Returns device specific info and transfer stats
 *          BAREADV     - Performs Read Verify 
 *          BAHRESET    - Performs Hardware Reset
 *          BAABORT     - Aborts the command in progress
 *          BASRESET    - Performs Soft Reset
 *          BASEEK      - Performs a Seek
 *          BABUFFW     - Performs a write to the attachment buffer
 *          BABUFFR     - Performs a read of the attachment buffer
 *          BADIAGSTAT  - Returns the Diagnostic Status block
 *          BACCSTAT    - Returns the Command Complete Status block
 *          BADEVSTAT   - Returns the Device Status
 *          BAPOSINFO   - Returns the current values of the POS registers
 *          BADEVCFIG   - Returns the device configuration info
 *          BAMFGH      - Returns the Manufacturers Header
 *          BAWAITCC    - Waits for the completion of a command 
 *          BAPARK      - Parks the heads of the disk
 *          BAFMTPROG   - Returns the current cylinder of a format         
 *          BATRANSRBA  - Translates an RBA to its real ABA (Absolute Blk Addr)
 *          BAFORMAT    - Formats the Disk
 *          BADIAGTEST  - Performs a diagnostic test
 *
 * (RECOVERY OPERATION:) Since most of these ioctls are for diagnostic
 *	purposes only, no retries or error logging takes place.  If a
 *      failure occurs, the proper errno is returned and the caller is
 *      left responsible for recovery.
 *
 * (DATA STRUCTURES:) The following data structures are used.   
 *           devinfo            - system defined device info structure
 *           ba_info            - bus attached disk info structure
 *           badisk_ioctl_parms - struct thru which necessary parameters
 *                                for specific ioctls are passed
 *           badisk_dstat_blk   - diagnostic status block
 *           badisk_cstat_blk   - command complete status block
 *           badisk_pos_regs    - POS registers
 *           badisk_spec        - device configuration info
 *
 * RETURNS: ENXIO       - minor number out of range                 
 *                      - device not inited or open 
 *          EFAULT      - copyout to user space fails
 *                      - attach to user space fails
 *          EACCES      - attempted diagnostic ioctl while not in diagnos mode
 *          ENOMEM      - not enough memory to perform request
 *          ENOTREADY   - command not completed within time allowed
 *          EINVAL      - invalid operation
 *          0           - successful completion
 *          Result of ba_issue_cmd() failure
 *          Result of pin() failure
 *
 */  

int	ba_ioctl(
dev_t   devno,                           /* major/minor numbers */
int	op,                              /* desired ioctl to perform */
int     arga,                            /* pointer to structure */
ulong   flag)                            /* called from user or system space */
{
	struct ba_info *ba;              /* pointer to info structure */ 
					 /* ioctl parameter structure */
	struct badisk_ioctl_parms ba_ioparms;   
	struct devinfo df;               /* local devinfo structure */
	int	i = 0,                   /* counter */
                dev,                     /* minor number */
                rc = OK,                 /* return code */
                clock_ticks;             /* millisecs convert to clock ticks*/
	short	segflg;			 /* either user or system space */

#ifdef DEBUG 
	BADBUG(("Entering ba_ioctl : %x %x %x %x\n", devno, op, arga, flag));
#endif DDHKWD5(HKWD_DD_BADISKDD, DD_ENTRY_IOCTL, 0, devno, op, arga, flag, 0);
	dev = minor(devno);              /* get minor number for this device */
	if (dev >= BA_NDISKS) {
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_IOCTL, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  1 ba_ioctl : ENXIO\n"));
#endif
		return(ENXIO);
	}
	lockl(&ba_lock, LOCK_SHORT);     /* lock resources for this device */
	ba = ba_list[dev];               /* get pointer to info struct */
	if ((ba == NULL) || (!ba->opened)) {
		unlockl(&ba_lock);       /* unlock resources */
		DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_IOCTL, ENXIO, devno);
#ifdef DEBUG 
		BADBUG(("Exiting  2 ba_ioctl : ENXIO\n"));
#endif
		return(ENXIO);
	}
	/*
	 * set up the segment flag to the proper value depending on
	 * whether we were called from user or system address space
	 */
	if (flag & DKERNEL) 
		segflg = SYS_ADSPACE;
	else
		segflg = USER_ADSPACE;
	switch (op) { 
	case IOCINFO:                       /* copy out device specific info */
		df.devtype = DD_DISK;
/*		df.devsubtype = DS_BADISK;  */
		df.flags = DF_RAND | DF_FIXED;
		df.un.dk.bytpsec = BA_BPS;
		df.un.dk.secptrk = ba->ba_cfig.sectors_per_trk;
		df.un.dk.trkpcyl = (short)(ba->ba_cfig.tracks_per_cyl);
		df.un.dk.numblks = ba->ba_cfig.total_blocks;
		df.un.dk.segment_size = ba->dds.bucket_size;
		df.un.dk.segment_count = ba->dds.bucket_count;
		df.un.dk.byte_count = ba->dds.byte_count;
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&df), (char *)(arga), sizeof(df));
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&df), (char *)(arga), 
				sizeof(df));
		}	
		break;
	case BAREADV:                      /* Read Verify */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
		ba->rba = ba_ioparms.rba;                 /* store the RBA */
                /*
                 * set byte count to number of blocks requested * bytes per
                 * sector 
                 */
		ba->byte_count = ba_ioparms.num_blks * BA_BPS; 
		rc = ba_issue_cmd(ba, (int)BA_READ_VERIFY, (ushort)BA_DISK);
		break;
	case BAHRESET:                     /* issue a Hardware reset */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		rc = ba_issue_cmd(ba, (int)BA_HARD_RESET, (ushort)BA_DISK);
		break;
	case BAABORT:                      /* issue an Abort command */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		rc = ba_issue_cmd(ba, (int)BA_ABORT_CMD, (ushort)ba->lcmd_dev);
		break;
	case BASRESET:                     /* issue a Soft Reset */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		rc = ba_issue_cmd(ba, (int)BA_SOFT_RESET, (ushort)BA_ATTACH);
		break;
	case BASEEK:                       /* issue a Seek command */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0) 
				break;
		}	
		ba->rba = ba_ioparms.rba;                 /* store the RBA */
		rc = ba_issue_cmd(ba, (int)BA_SEEK_CMD, (ushort)BA_DISK); 
		break;
	case BABUFFW:                      /* Buffer Write Test */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if ((ba->mp = (struct xmem *)xmalloc((uint)sizeof(struct xmem),
			(uint)PGSHIFT, (heapaddr_t)pinned_heap)) == NULL) {
			rc = ENOMEM;	   /* set rc to not enough mem */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
                /* 
                 * compute numbers of bytes from number of blocks
                 */
		ba->byte_count = ba_ioparms.num_blks * 512; 
		ba->target_buff = (char *)ba_ioparms.buff_address; 
		if ((rc = pinu((caddr_t)ba->target_buff, ba->byte_count,
			segflg))
			!= 0) {
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                              /* pinu failure */
		}
		ba->mp->aspace_id = XMEM_INVAL;
		if(xmattach((char *)ba->target_buff,ba->byte_count,
			ba->mp,segflg) == XMEM_FAIL) {
			rc = EFAULT;
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                         /* xmattach failure */
		}
		ba->dma_flags = 0;              /* set dma flags for write op*/
		ba->dma_active = TRUE;          /* set dma active flag */
		ba->chained = 0;                /* clear chained count */
                /*
                 * initiate 3rd party dma transfer
                 */
		d_slave((int)ba->dma_chn_id, (int)ba->dma_flags,
			 (char *)ba->target_buff, (int)ba->byte_count, ba->mp);
		if (rc = ba_issue_cmd(ba, (int)BA_BUFFW_CMD, 
			(ushort)BA_ATTACH)) { 
			d_complete(ba->dma_chn_id, ba->dma_flags,
				(char *)ba->target_buff, ba->byte_count,
				(struct xmem *)ba->mp, NULL);
			ba->dma_active = FALSE;      /* clear dma active flag*/
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                         /* issue cmd failure */
		}
		/*
                 * wait for command to complete 
                 */
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                         /* cmd didn't complete*/
		}
		unpinu((char *)ba->target_buff, ba->byte_count, segflg);
		xmdetach(ba->mp);
		xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
		break;
	case BABUFFR:                      /* Buffer Read Test */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if ((ba->mp = (struct xmem *)xmalloc((uint)sizeof(struct xmem),
			(uint)PGSHIFT, (heapaddr_t)pinned_heap)) == NULL) {
			rc = ENOMEM;       /* set rc to not enough mem */
			break;             /* malloc failure */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
                /*
                 * compute bytes from number of blocks * bytes per sector 
                 */
		ba->byte_count = ba_ioparms.num_blks * 512;  
		ba->target_buff = (char *)ba_ioparms.buff_address;
		if ((rc = pinu((caddr_t)ba->target_buff, ba->byte_count,
			segflg)) 
			!= 0) {
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                           /* pinu failure */
		}
		ba->mp->aspace_id = XMEM_INVAL;
		if(xmattach((char *)ba->target_buff,ba->byte_count,
			ba->mp,segflg) == XMEM_FAIL) {
			rc = EFAULT;
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                      /* xmattach failure */
		}
		ba->dma_flags = DMA_READ;          /* set dma flags for read */
		ba->dma_active = TRUE;             /* set dma active flag */
		ba->chained = 0;                   /* clear chained count */
		/*
		 * start 3rd party dma transfer
		 */
		d_slave((int)ba->dma_chn_id, (int)ba->dma_flags, 
			(char *)ba->target_buff, (int)ba->byte_count, ba->mp);
		if (rc = ba_issue_cmd(ba, (int)BA_BUFFR_CMD, 
			(ushort)BA_ATTACH)) { 
			d_complete(ba->dma_chn_id, ba->dma_flags,
				(char *)ba->target_buff, ba->byte_count,
			    	(struct xmem *)ba->mp, NULL);
			ba->dma_active = FALSE;      /* clear dma active flag*/
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                       /* issue cmd failure */
		}
		/* 
                 * wait for command completeion 
                 */
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp); 
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                         /* cmd didnt complete */
		}
		unpinu((char *)ba->target_buff, ba->byte_count,
			segflg);
		xmdetach(ba->mp); 
		xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);/*free memory*/
		break;
	case BADIAGSTAT:                   /* Get Diagnostic Status block */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/*
		 * issue the command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_DIAG_STAT, 
			(ushort)ba->lcmd_dev)) 
			break;
		/*
		 * wait for command completion
		 */
		i = 0;
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;        /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&ba->ba_dsb), (char *)(arga), 
				sizeof(struct badisk_dstat_blk ));
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&ba->ba_dsb), (char *)(arga), 
				sizeof(struct badisk_dstat_blk ));
		}
		break;
	case BACCSTAT:                     /* Get Command Complete Status Blk*/
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/*
		 * issue the command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_CC_STAT, 
			(ushort)ba->lcmd_dev))
			break;
		/*
		 * wait for command completion
		 */
                i=0;
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;           /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&ba->ba_ccs), (char *)(arga), 
				sizeof(struct badisk_cstat_blk ));
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&ba->ba_ccs), (char *)(arga), 
				sizeof(struct badisk_cstat_blk ));
		}
		break;
	case BADEVSTAT:                    /* Get Device Status Block */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/*
		 * issue the command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_DEV_STAT, 
			(ushort)BA_DISK))
			break;
		/*
		 * wait for command completion
		 */
                i=0;
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;             /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&ba->ba_ccs), (char *)(arga), 
				sizeof(struct badisk_cstat_blk )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&ba->ba_ccs), (char *)(arga), 
				sizeof(struct badisk_cstat_blk )); 
		}
		break;
	case BAPOSINFO:                    /* Get POS register info */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/*
		 * issue the command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_POS_INFO, 
			(ushort)BA_ATTACH))
			break;
		/*
		 * wait for command completion
		 */
                i=0;
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;             /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&ba->ba_pos), (char *)(arga), 
				sizeof(struct badisk_pos_regs )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&ba->ba_pos), (char *)(arga), 
				sizeof(struct badisk_pos_regs )); 
		}
		break;
	case BADEVCFIG:                    /* Get Device Configuration */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/*
		 * issue the command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_DEV_CONFIG, 
			(ushort)BA_DISK))
			break;
		/*
		 * wait for command completion
		 */
                i=0;
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;             /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((char *)(&ba->ba_cfig), (char *)(arga), 
				sizeof(struct badisk_spec )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyout((char *)(&ba->ba_cfig), (char *)(arga), 
				sizeof(struct badisk_spec )); 
		}
		break;
	case BAMFGH:                       /* Get Manufacturer Header */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if ((ba->mp = (struct xmem *)xmalloc((uint)sizeof(struct xmem),
			(uint)PGSHIFT, (heapaddr_t)pinned_heap)) == NULL) {
			rc = ENOMEM;	      /* set rc to not enough mem */
			break;                /* xmalloc failure */
		}
		ba->byte_count = BA_BPS;      /* bytes  = 1 sector ( 512 )*/
		ba->target_buff =  (char *)arga; /* get addr of buffer */
#ifdef DEBUG 
		BADBUG(("Pinning users space %X\n",ba->target_buff));
#endif
		if ((rc = pinu((caddr_t)ba->target_buff, ba->byte_count,
			segflg)) != 0) {
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                         /* pinu failure */
		}
		ba->mp->aspace_id = XMEM_INVAL;
#ifdef DEBUG 
		BADBUG(("Attaching users space %X\n",ba->target_buff));
#endif
		if(xmattach((char *)ba->target_buff,ba->byte_count,
			ba->mp,segflg) == XMEM_FAIL) {
			rc = EFAULT;
			unpinu((char *)ba->target_buff, ba->byte_count, 
				segflg);
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                     /* xmattach failure */
		}
		ba->dma_flags = DMA_READ;          /* set dma flags for read */
		ba->dma_active = TRUE;             /* set dma active flag */
		ba->chained = 0;                   /* clear chained count */
		/* 
		 * start 3rd party dma transfer
		 */
		d_slave((int)ba->dma_chn_id, (int)ba->dma_flags, 
			(char *)ba->target_buff, (int)ba->byte_count, ba->mp);
		/*
		 * issue command
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_GET_MFG, (ushort)BA_DISK)) { 
			d_complete(ba->dma_chn_id, ba->dma_flags,
			(char *)ba->target_buff, ba->byte_count,
			    (struct xmem *)ba->mp, NULL);
			ba->dma_active = FALSE;     /* clear dma active flag */
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp); 
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);
			break;                           /* issue cmd failure*/
		}
		/* 
                 * wait for command completion 
                 */
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			unpinu((char *)ba->target_buff, ba->byte_count,
				segflg);
			xmdetach(ba->mp); 
			xmfree((char *)ba->mp, (heapaddr_t)pinned_heap); 
			break;                         /* cmd didnt complete */
		}
		unpinu((char *)ba->target_buff, ba->byte_count, segflg);
		xmdetach(ba->mp);
		xmfree((char *)ba->mp, (heapaddr_t)pinned_heap);    
		break;
	case BAWAITCC:                     /* Wait For Completion of Command */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
		rc = ENOTREADY;         /* initialize return code for TimeOut*/
                /* 
                 * compute clock ticks based on seconds * ticks per second
                 */
		clock_ticks = (int)(ba_ioparms.milli_secs / 1000) * HZ;
		if (!clock_ticks) 
			clock_ticks = 1;          /* minimum of 1 clock tick */
                /*
                 * for the number of clock ticks, check if the command has
		 * completed after each tick, if it has return 0 immediately
		 */
		for (i = 0; i < clock_ticks; i++) {
			if (ba->command_cmpl) {
				rc = OK;
				break;
			} else {
				delay(1);
			}
		}
		break;
	case BAPARK:                       /* park heads in safe place */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		rc = ba_issue_cmd(ba, (int)BA_PARK_HEADS, (ushort)BA_DISK);
		break;
	case BAFMTPROG:                    /* Check Progress of Format */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		/* 
		 * store current cylinder
		 */
		ba_ioparms.curr_cylinder = ba->format_count;   
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)&ba_ioparms, (caddr_t *)arga, 
				sizeof(struct badisk_ioctl_parms ));
		else { 
			/*
			 * if user space
			 */
			rc = copyout((caddr_t *)&ba_ioparms, (caddr_t *)arga, 
				sizeof(struct badisk_ioctl_parms ));
		}
		break;
	case BATRANSRBA:                  /* Translate Relative Block Address*/
		if (!ba->diagnos_mode) {
			rc = EACCES;      /* not in diagnostic mode */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
		ba->rba = ba_ioparms.rba;                       /* store RBA */
		if (rc = ba_issue_cmd(ba, (int)BA_TRANS_RBA, (ushort)BA_DISK)) 
			break;                           /* issue cmd failure*/
                /*
		 * wait for command to complete 
		 */
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;            /* time out */
		}
                /*
		 * concatenate the most and least significant words to 
		 * form the Absolute Block Address
		 */
		ba_ioparms.aba = ba->ba_ccs.lsw_lastrba | 
			((ba->ba_ccs.msw_lastrba << BA_1WORD) & BA_HWMASK); 
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)&ba_ioparms, (caddr_t *)arga, 
				sizeof(struct badisk_ioctl_parms ));
		else { 
			/*
			 * if user space
			 */
			rc = copyout((caddr_t *)&ba_ioparms, (caddr_t *)arga, 
				sizeof(struct badisk_ioctl_parms ));
		}
		break;
	case BAFORMAT:                     /* Format the disk */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
                /*
		 * issue pre-format code 
		 */
		if (rc = ba_issue_cmd(ba, (int)BA_PREFORMAT, (ushort)BA_DISK))
			break;
                /*
		 * wait for command to complete 
		 */
		while ((++i < BA_TO100) && (!ba->command_cmpl)) 
			delay(1);
		if (i == BA_TO100) {
			rc = ENOTREADY;
			break;              /* time out */
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
		/* 
                 * issue format with the desired options
                 */
		rc = ba_issue_cmd(ba, (int)(BA_FORMAT_CMD | 
			ba_ioparms.format_options), (ushort)BA_DISK);
		break;
	case BADIAGTEST:                   /* Issue Diagnostic Test */
		if (!ba->diagnos_mode) {
			rc = EACCES;       /* not in diagnostic mode */
			break;
		}
		if (flag & DKERNEL) 
			/*
			 * if kernel space
			 */
			bcopy((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
		else { 
			/*
			 * if user space
			 */
			rc = copyin((caddr_t *)arga, (caddr_t *)&ba_ioparms, 
				sizeof(struct badisk_ioctl_parms )); 
			if (rc != 0)  
				break;
		}	
                /*
 		 * SELF TEST is the only diagnostic command that is issued
		 * to the attachment, all others are issued to the disk
		 */
		if (ba_ioparms.diag_test == BADIAG_SELFTEST)  
			rc = ba_issue_cmd(ba, (int)(BA_DIAG_TEST | 
				ba_ioparms.diag_test), (ushort)BA_ATTACH);
		else 
			rc = ba_issue_cmd(ba, (int)(BA_DIAG_TEST | 
				ba_ioparms.diag_test), (ushort)BA_DISK);
		break;
	default:
		rc = EINVAL;                         /* invalid operation */
		break;
	} /* switch(op) */
	unlockl(&ba_lock);                           /* unlock resources */
#ifdef DEBUG 
	BADBUG(("Exiting  3 ba_ioctl %d\n", rc));
#endif
	DDHKWD1(HKWD_DD_BADISKDD, DD_EXIT_IOCTL, rc, devno);
	return(rc);
}


/*
 * NAME: ba_mincnt
 *                                                                    
 * FUNCTION: Preserves the extended parameters in a RAW I/O call 
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the process level by uphysio 
 *	and can page fault.                                     
 *                                                                   
 * (NOTES:) This routine's only function is to convert the extended
 *	parameters of a RAW read or write system call into the buf 
 *      flags for the buf structure to be passed to the ba_strategy
 *      routine.
 *
 * (RECOVERY OPERATION:) NONE                                    
 *
 * (DATA STRUCTURES:) buf   - the structure to be passed to ba_strategy  
 *
 * RETURNS: 0    - successful completion                            
 */  

int	ba_mincnt(
struct buf *bp,             /* pointer to buf structure */
void *minparms)             /* extended parameters from readx or writex call */
{

#ifdef DEBUG 
	BADBUG(("Entering ba_mincnt\n"));
#endif
	/*
	 * set buf options to options specified in the 
	 * extended param to readx or writex 
	 */
	bp->b_options = (int) minparms;
	return(OK);
}


/*
 * NAME: ba_pos_setup
 *                                                                    
 * FUNCTION: Sets up the Bus Attached Disk POS Registers      
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the process level by ba_config
 *	at configuration time, and can page fault.              
 *                                                                   
 * (NOTES:) This routine initializes the Programmable Option Select
 *	Registers to the proper values.                  
 *
 * (RECOVERY OPERATION:) NONE                                    
 *
 * (DATA STRUCTURES:) ba_info   - General info structure      
 *
 * RETURNS: 0     - successful completion                           
 */  

int	ba_pos_setup(
struct  ba_info *ba)                /* pointer to general info structure */
{
	uint base;                  /* base address */
	uchar pos0;		    /* POS register 0 value */
	uchar pos1;		    /* POS register 1 value */
	ushort id;		    /* id of device, from POS regs 0,1 */

#ifdef DEBUG 
	BADBUG(("Entering ba_pos_setup\n"));
#endif
	/* 
	 * get segment register for iocc transfers 
	 */
	base = (uint)IOCC_ATT(ba->dds.bus_id, (ba->dds.slot << BA_1WORD));
	/*
	 * Verify that disk is present in the specified slot
	 */
	pos0 = BUSIO_GETC(base + BA_POS0);
	pos1 = BUSIO_GETC(base + BA_POS1);
	id = (pos0 << BA_1BYTE) | pos1;
	if (id != BA_DISK_ID) {
		/*
		 * Either there is nothing in this slot, or its not
		 * a Bus Attached Disk
		 */
		IOCC_DET(base);       /* release segment register */
		return(EINVAL);
	}

	/* 
	 * Set up POS registers 2,3, and 4 
	 */

	/* 
	 * POS2VAL = Card Enable, Base Address,DMA chn, Fairness Enabled 
	 */
	BUSIO_PUTC(base + BA_POS2, BA_POS2VAL | (0x40) | 
		(ba->dds.dma_level << 2) | (ba->dds.alt_address << 1));
	/* 
	 * POS3VAL = Pacing Wait = 24usec, Don't reset DMA on TC    
	 */
	BUSIO_PUTC(base + BA_POS3, BA_POS3VAL);

	/* 
	 * POS4VAL = Pacing Disable, Bus Release after 6 usec 
	 */
	BUSIO_PUTC(base + BA_POS4, BA_POS4VAL);

	IOCC_DET(base);       /* release segment register */
#ifdef DEBUG 
	BADBUG(("Exiting ba_pos_setup\n"));
#endif
	return(0);
}

/*
 * NAME: ba_pos_clean
 *                                                                    
 * FUNCTION: Cleans up POS registers after ROS access         
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This routine is called on the process level by ba_config
 *	at configuration time, and can page fault.              
 *                                                                   
 * (NOTES:) This routine disables the Bus Attached Disk Attachment before
 *	any access is made.                              
 *
 * (RECOVERY OPERATION:) NONE                                    
 *
 * (DATA STRUCTURES:) ba_info   - General info structure      
 *
 * RETURNS: NONE                       
 */  

void	ba_pos_clean() 
{
	uint base;                  /* base address */
	uchar pos0;		    /* POS register 0 value */
	uchar pos1;		    /* POS register 1 value */
	uchar slot;		    /* POS register 1 value */
	ushort id;		    /* id of device, from POS regs 0,1 */

#ifdef DEBUG 
	BADBUG(("Entering ba_pos_clean\n"));
#endif
	/* 
	 * get segment register for iocc transfers 
	 */
	for(slot=6; slot <= 7; slot++) {
		base = (uint)IOCC_ATT(0x820c00e0, (slot << BA_1WORD));
		/*
		 * Verify that disk is present in the specified slot
		 */
		pos0 = BUSIO_GETC(base + BA_POS0);
		pos1 = BUSIO_GETC(base + BA_POS1);
		id = (pos0 << BA_1BYTE) | pos1;
		if (id == BA_DISK_ID) {
			/*
			 * a Bus Attached Disk so disable card
			 */
			BUSIO_PUTC(base + BA_POS2,0x14);
		}
		IOCC_DET(base);       /* release segment register */
	}

	return;
}
