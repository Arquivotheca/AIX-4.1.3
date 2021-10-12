static char sccsid[] = "@(#)98  1.7.2.3  src/bos/kernext/disk/sd/sdopen.c, sysxdisk, bos411, 9428A410j 3/16/94 10:13:40";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_open(), sd_adap_open(), sd_ctrl_open(), sd_dasd_open()
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
#include <sys/priv.h>
#include <sys/intr.h>
#include <sys/ddtrace.h>
#include <sys/errno.h>
#include <sys/malloc.h>
#include <sys/lockl.h>
#include <sys/trchkid.h>
#include <sys/sd.h>


extern	uchar	sd_open_adaps;
extern	uchar	sd_open_ctrls;
extern	uchar	sd_open_dasd;
extern	struct watchdog	activity_timer;
extern	struct intr	*sd_epow_ptr;

/*
 * NAME: sd_open
 *
 * FUNCTION: Open Entry Point for Serial DASD Subsystem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                             
 *
 * (NOTES:) Possible operations : This routine determines which layer of
 *	the subsytem is being opened (adapter, controller, or dasd), and
 *	calls the appropriate open routine.
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * RETURNS: 	result of adapter, controller, or dasd open call
 */

int sd_open(
dev_t	devno,
int	rwflag,
int	chan,
int	ext)
{
	int			rc;		/* return code 		*/
	int			min;		/* minor number 	*/
	struct sd_adap_info 	*ap;            /* adapter pointer 	*/
	struct sd_ctrl_info 	*cp;		/* controller pointer 	*/
	struct sd_dasd_info 	*dp;		/* DASD pointer 	*/

	DDHKWD5(HKWD_DD_SERDASDD,DD_ENTRY_OPEN,0,devno,rwflag,chan,ext,0);

	if ( ext && (rc = privcheck(RAS_CONFIG))) { 
		/*
		 * if extended open, but no authority..
		 */
		DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_OPEN, rc, devno);
		return(rc);
	}
    
	/* 
	 * get major number 
	 */
	min = minor(devno);
	if (ext & SD_DAEMON)
		chan = 0;
	else
		chan = 1;   /* set chan to 1 to represent file system open */
	lockl ((&sd_global_lock), LOCK_SHORT); /* serialize using global lock */

if (min & SD_DASD_MASK) {
		/*
		 * if dasd call
		 */
		/*
		 * get dasd info pointer 
		 */
		dp = (struct sd_dasd_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (dp == NULL)
			return(ENXIO);
		lockl(&dp->dev_lock,LOCK_SHORT);
		rc = sd_dasd_open(dp, rwflag, chan, ext);
		unlockl(&dp->dev_lock);
	} else if (min & SD_CTRL_MASK) {
		/*
		 * else if controller call, notice the chan field is
		 * set to 1 to represent a file system open.
		 */
		/*
		 * get controller info pointer 
		 */
		cp = (struct sd_ctrl_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (cp == NULL)
			return(ENXIO);
		lockl(&cp->dev_lock,LOCK_SHORT);
		rc = sd_ctrl_open(cp, rwflag, chan, ext);
		unlockl(&cp->dev_lock);
	} else {
		/*
		 * else adapter call
		 */
		/*
		 * get adapter info pointer 
		 */
		ap = (struct sd_adap_info *)sd_hash(devno); 
		unlockl(&sd_global_lock);	/* unlock the global lock */
		if (ap == NULL)
			return(ENXIO);
		lockl(&ap->dev_lock,LOCK_SHORT);
		rc = sd_adap_open(ap, minor(devno), rwflag, chan, ext);
		unlockl(&ap->dev_lock);
	}

	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_OPEN, rc, devno);
	return(rc);			/* return our return code */
}


/*
 * NAME: sd_adap_open
 *
 * FUNCTION: Opens Serial DASD Adapter
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                             
 *
 * (NOTES:) Possible operations : Extended parameter could specify specific
 *	type of open :
 *
 *	SC_DIAGNOSTIC:	Places the adapter in diagnostic mode
 *	SD_DAEMON:	Specifies that this is the daemon opening the adapter
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *
 * RETURNS: 	Results of privcheck call, if improper authority
 *		Results of i_init, if fail
 *		0         - successful completion
 *		EIO	  - PIO error        
 *			  - error writing POS reg
 * 			  - pincode failure                   
 * 			  - setup of adapter mailbox chain failed
 * 			  - setup of adapter parameters failed
 *			  - d_init failed
 * 		ENOMEM	  - couldn't malloc epow structure
 *			  - couldn't malloc Small Transfer Area
 *		ENXIO	  - adapter not configured
 *		EACCES	  - adapter already open in diagnostic mode, and caller
 *			    is not daemon.
 *			  - adapter already has one daemon open, and caller is
 *			    requesting daemon open.
 *			  - adapter has one open, but not daemon open, and
 * 			    caller is requesting diagnostic mode
 *			  - adapter has more than one open, and caller is 
 *			    requesting diagnostic mode
 *			  - caller requesting daemon mode, but not daemon
 */

int sd_adap_open(
struct sd_adap_info *ap,
int   	minorno,
int     rwflag,
int     chan,
int     ext)
{
	int	ii,			/* counter                       */
		rc,			/* return code                   */
	        daemon_minor = FALSE;   /* whether the minor number is a */
					/* daemon entry point or nor     */
	uint	dma_addr,		/* dma address */
		base;			/* base address */


	/*
	 * Check if call is through
	 * the daemon special file
	 */
	if (minorno & SD_DAEMON_MASK)
		daemon_minor = TRUE;

	if ((ap->diag_mode) && (!(ext & SD_DAEMON))) 	
		/*
		 * if adapter already open in diagnostic mode, 
		 * and this is not the DAEMON
		 */
		return(EACCES);

	if ((ap->daemon_open) && (ext & SD_DAEMON))
		/*
		 * if adapter has one daemon open, and this is a daemon
		 */
		return(EACCES);

	if ((ap->internal_open || ap->fs_open) && (ext & SC_DIAGNOSTIC))	
		/*
		 * if adapter open, and diagnostic mode requested
		 */
		return(EACCES);

	if ((ext & SD_DAEMON) && !daemon_minor) {
		/*
		 * if Daemon open ...
		 */
		return(EACCES);		
	}

	if (!ap->opened) {
		/*
		 * If first open to this adapter
		 */
		if (!sd_open_adaps) {

			/*
			 * if first open to any adapter
			 */
			rc = pincode((int(*)()) sd_intr);/* pin bottom half */
			if ( rc)			/* if pincode failed */
				return(EIO);
			/*
			 * get memory for epow structure
			 */
			if ((sd_epow_ptr = (struct intr *)xmalloc((uint)
				sizeof(struct intr), 2, 
				(heapaddr_t)pinned_heap)) == NULL) {
				/*
				 * if not enough memory
				 */
                                unpincode((int(*)())sd_intr);
				return(ENOMEM);       /* not enough memory */
			}
			bzero(sd_epow_ptr, sizeof(struct intr ));
			INIT_EPOW((sd_epow_ptr), (int(*)())sd_epow, 
				ap->dds.bus_id);
			sd_epow_ptr->flags |= INTR_MPSAFE;
			if ((rc = i_init(sd_epow_ptr) == INTR_FAIL)) {
				/*
				 * if failure initting epow
				 */
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
                                unpincode((int(*)())sd_intr);
				return(rc);
			}
			/* 
			 * Start global Activity Timer Here ... 
			 */
        		activity_timer.restart = 60; 
        		activity_timer.func = (void(*)())sd_activity;

			/*
			 * Loop until w_init succeeds
			 */
			while(w_init(&activity_timer));
			w_start(&activity_timer);

			/* 
			 * Register Component Dump Table ... 
			 */
			(void) dmp_add((struct cdt (*)())sd_adap_cdt_func);
		}

		if (!ap->ever_open) {
			/*
			 * if this is the first ever open, then we
			 * need to initialize the mailbox chain for
			 * this adapter
			 *
		 	 * enable this card 
		 	 */
			ap->pos2 |= SD_CARD_ENABLE;
			if (sd_write_POS(ap,SD_POS2,ap->pos2)) {
				/*
				 * if POS write failed
				 */
				if (!sd_open_adaps) {
					
					/*
					 * clear the epow hndlr
					 */
					i_clear(sd_epow_ptr); 
					xmfree((char *)sd_epow_ptr, (heapaddr_t)
					       pinned_heap);
					/*
					 * unpin modules code
					 */
					unpincode((int(*)())sd_intr);
					/*
					 * Stop global Activity Timer Here
					 */
					w_stop(&activity_timer);
					/* 
					 * Loop until w_clear succeeds.
					 * Clear watchdog timer.
					 */
					while(w_clear(&activity_timer)); 
					/* 
					 * De-Register Component Dump Table 
					 */
					(void) dmp_del((struct cdt (*)())
						       sd_adap_cdt_func);
				}
				return(EIO);
			}
			/* 
			 * Reset the adapter here because we dont know what 
			 * state the bootros will have left it in.
			 */
			sd_reset_quiesce_disable( ap,
					  (uchar)SD_RESET_OP,
					  (uchar)SD_ADAP_CMD ); 
			ap->open_no1_intrpt = 1;		
			sd_sleep( ap,
				  &ap->open_no1_intrpt,
				  &ap->open_no1_event);		
			/* 
			 * Disable Interrupts on Adapter ... 
			 * This is not a critical path
			 */
			base = (uint)BUSIO_ATT(ap->dds.bus_id, 
				ap->dds.base_addr);
			if (SD_PUTC((base + SD_CTRL), SD_INT_DISABLE)){
				/*
				 * if PIO failed
				 */
		               	BUSIO_DET(base);
				ap->pos2 &= ~SD_CARD_ENABLE;
				sd_write_POS(ap,SD_POS2,ap->pos2);
				if (!sd_open_adaps) {
					
					/*
					 * clear the epow hndlr
					 */
					i_clear(sd_epow_ptr); 
					xmfree((char *)sd_epow_ptr, (heapaddr_t)
					       pinned_heap);
					/*
					 * unpin modules code
					 */
					unpincode((int(*)())sd_intr);
					/*
					 * Stop global Activity Timer Here
					 */
					w_stop(&activity_timer);

					/* 
					 * Loop until w_clear succeeds.
					 * Clear watchdog timer.
					 */
					while(w_clear(&activity_timer)); 
					/* 
					 * De-Register Component Dump Table 
					 */
					(void) dmp_del((struct cdt (*)())
						       sd_adap_cdt_func);
				}
				return(EIO);
			}
	               	BUSIO_DET(base);

                	ap->base_MB_dma_addr = SD_DMA_ADDR(
				ap->dds.tcw_start_addr,ap->mb_tcw_start);

			if(rc = sd_restart_adap(ap)) {
				/*
				 * if error configuring adapter, 
				 * disable card,
				 * free adapter structure, remove
				 * from devsw table if last one, 
				 * return code.
				 */
				ap->pos2 &= ~SD_CARD_ENABLE;
				sd_write_POS(ap,SD_POS2,ap->pos2);
				if (!sd_open_adaps) {
					
					/*
					 * clear the epow hndlr
					 */
					i_clear(sd_epow_ptr); 
					xmfree((char *)sd_epow_ptr, (heapaddr_t)
					       pinned_heap);
					/*
					 * unpin modules code
					 */
					unpincode((int(*)())sd_intr);
					/*
					 * Stop global Activity Timer Here
					 */
					w_stop(&activity_timer);

					/* 
					 * Loop until w_clear succeeds.
					 * Clear watchdog timer.
					 */
	
					while(w_clear(&activity_timer)); 
					/* 
					 * De-Register Component Dump Table 
					 */
					(void) dmp_del((struct cdt (*)())
						       sd_adap_cdt_func);
				}
				return(EIO);
			}

			/* 
			 * disable this card 
			 */
			ap->pos2 &= ~SD_CARD_ENABLE;
			if (sd_write_POS(ap,SD_POS2,ap->pos2)) {
				/*
				 * if pos write failed, 
				 */
				if (!sd_open_adaps) {
					
					/*
					 * clear the epow hndlr
					 */
					i_clear(sd_epow_ptr); 
					xmfree((char *)sd_epow_ptr, (heapaddr_t)
					       pinned_heap);
					/*
					 * unpin modules code
					 */
					unpincode((int(*)())sd_intr);
					/*
					 * Stop global Activity Timer Here
					 */
					w_stop(&activity_timer);

					/* 
					 * Loop until w_clear succeeds.
					 * Clear watchdog timer.
					 */

					while(w_clear(&activity_timer)); 
					/* 
					 * De-Register Component Dump Table 
					 */
					(void) dmp_del((struct cdt (*)())
						       sd_adap_cdt_func);
				}
				return(EIO);
			}
			/*
			 * set one-shot flag to tell us we've done this
			 * at least once. Notice this flag is never
			 * cleared
			 */
			ap->ever_open = TRUE;
		}
                /*
                 *  call i_init to initialize the interrupt handler
                 */
                if ((rc = i_init((struct intr *)&ap->sdi) == INTR_FAIL)) {
                        if (!sd_open_adaps) {
                                /*
                                 * unpin modules code
                                 */
				i_clear(sd_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
                                unpincode((int(*)())sd_intr);
				/*
				 * Stop global Activity Timer Here
				 */
				w_stop(&activity_timer);

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&activity_timer)); 
				/* 
				 * De-Register Component Dump Table 
				 */
				(void) dmp_del((struct cdt (*)())
					sd_adap_cdt_func);
                        }
                        return(rc);                        /* i_init failure */
		}


		/* 
		 * enable this card 
		 */
        	ap->pos2 |= SD_CARD_ENABLE;
	        if (sd_write_POS(ap,SD_POS2,ap->pos2)) {
			/* 
			 * Problem enabling card
			 */
                        if (!sd_open_adaps) {
                                /*
                                 * unpin modules code
                                 */
				i_clear(sd_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
                                unpincode((int(*)())sd_intr);
				/*
				 * Stop global Activity Timer Here
				 */
				w_stop(&activity_timer);

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&activity_timer)); 

				/* 
				 * De-Register Component Dump Table 
				 */
				(void) dmp_del((struct cdt (*)())
					sd_adap_cdt_func);
                        }
			return(EIO);
		}

		ap->opened = TRUE;  	/* flag this adapter as open  */
		/* 
		 * increment number of adapters open in system 
		 */
		sd_open_adaps++;	

		/*
		 * Initialize any first open variables ... adapter state, 
		 * watchdog routines, etc....
		 */
		ap->status = 0;

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
                while(w_init(&ap->ioctl_timer)); 

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
 
                while(w_init(&ap->cmd_timer));  

		/*
		 * Allocate Small Transfer Area 
		 */
                ap->STA[0].stap = (char *)xmalloc(SD_STA_ALLOC_SIZE,PGSHIFT,
			pinned_heap);
                if (ap->STA[0].stap == NULL){
                        ap->opened = FALSE;  /* mark adapter closed */
                        sd_open_adaps--;   /* dec opened adapters counter */
			/* 
			 * disable this card 
			 */
	        	ap->pos2 &= ~SD_CARD_ENABLE;
		        sd_write_POS(ap,SD_POS2,ap->pos2);
			/* 
			 * clear interrupt handler 
			 */
                        i_clear((struct intr *)&(ap->sdi)); 
			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */

			while(w_clear(&ap->ioctl_timer)); 

			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */

			while(w_clear(&ap->cmd_timer)); 

                        if (!sd_open_adaps) {
				i_clear(sd_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
                                (void)unpincode((int (*)()) sd_intr);
				/*
				 * Stop global Activity Timer Here
				 */
				w_stop(&activity_timer);

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

	
				while(w_clear(&activity_timer)); 
				
				/* 
				 * De-Register Component Dump Table 
				 */
				(void) dmp_del((struct cdt (*)())
					sd_adap_cdt_func);
                        }
                        return(ENOMEM);
                } /* successfully malloc'ed and pinned STA */

        	/*
	         * Initialize the Free TCW List
	         */
		for (ii=0; ii< ap->num_tcw_words; ii++)
		        ap->tcw_free_list[ii] = 0xffffffff;
		/*
		 * Now clear remainder bits
		 */
		ap->tcw_free_list[ap->num_tcw_words-1] &= 
			(0xFFFFFFFF << (SD_BITS_PER_WORD - 
			(ap->num_tcws % SD_BITS_PER_WORD))); 

                /*
                 * init small DMA transfer area mgmt table
                 */
                for (ii = 0; ii < SD_NUM_STA; ii++) {
                        ap->STA[ii].stap = ap->STA[0].stap + (ii * SD_STA_SIZE);
                        ap->STA[ii].in_use = FALSE;
                }

                ap->dma_channel = d_init(ap->dds.dma_lvl,SD_DMA_INIT,
			ap->dds.bus_id);
                if (ap->dma_channel == DMA_FAIL) {
                        /*
                         * handle failed dma service call
                         */
                        (void)xmfree((caddr_t)ap->STA[0].stap,pinned_heap);
                        ap->opened = FALSE;     /* mark adapter closed */
                        sd_open_adaps--;    /* dec opened adapters counter */
			/* 
			 * disable this card 
			 */
	        	ap->pos2 &= ~SD_CARD_ENABLE;
		        sd_write_POS(ap,SD_POS2,ap->pos2);
                        i_clear((struct intr *)&(ap->sdi));

			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */

			while(w_clear(&ap->ioctl_timer)); 

			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */


			while(w_clear(&ap->cmd_timer)); 
                        if (!sd_open_adaps) {
				i_clear(sd_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
                                (void)unpincode((int (*)())sd_intr);
				/*
				 * Stop global Activity Timer Here
				 */
				w_stop(&activity_timer);

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&activity_timer)); 

				/* 
				 * De-Register Component Dump Table 
				 */
				(void) dmp_del((struct cdt (*)())
					sd_adap_cdt_func);
                        }
                        return(EIO);
                }
                d_unmask((int)ap->dma_channel);      /* enable this DMA chan */
                /*
                 * Map TCW's for the small DMA transfer area, and then
                 * unhide the page to allow access to it.           
                 */
                dma_addr = SD_DMA_ADDR(ap->dds.tcw_start_addr,
			ap->sta_tcw_start);
                d_master((int)ap->dma_channel,(int)(SD_DMA_TYPE | DMA_NOHIDE), 
			(char *)ap->STA[0].stap, (size_t)SD_STA_ALLOC_SIZE, 
			&ap->xmem_buf, (char *)dma_addr);
                (void)d_complete((int)ap->dma_channel,(int)(SD_DMA_TYPE | 
			DMA_NOHIDE), (char *)ap->STA[0].stap, 
			(size_t)SD_STA_ALLOC_SIZE, &ap->xmem_buf, 
			(char *)dma_addr);

                /*
                 * Map TCW's for the Mailbox area, and then
                 * unhide the page to allow access to it.           
                 */
                dma_addr = ap->base_MB_dma_addr;
                d_master((int)ap->dma_channel,(int)(SD_DMA_TYPE | DMA_NOHIDE),
			(char *)ap->MB, (size_t)SD_MBA_ALLOC_SIZE,
			&ap->xmem_buf,(char *)dma_addr);
                (void)d_complete((int)ap->dma_channel,(int)(SD_DMA_TYPE | 
			DMA_NOHIDE), (char *)ap->MB, (size_t)SD_MBA_ALLOC_SIZE,
			&ap->xmem_buf, (char *)dma_addr);


		/*
		 * Enable Interrupts on Adapter ... 
		 * This is not a critical path
		 */
                base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
                if (SD_PUTC((base + SD_CTRL), SD_INT_ENABLE)) {
			/*
			 * if PIO failed
			 */
			BUSIO_DET(base);
			(void)xmfree((caddr_t)ap->STA[0].stap, pinned_heap);
			ap->opened = FALSE;  /* mark adapter closed */
			sd_open_adaps--;   /* dec opened adaps cnt */
			/* 
			 * disable this card 
			 */
		       	ap->pos2 &= ~SD_CARD_ENABLE;
		        sd_write_POS(ap,SD_POS2,ap->pos2);
			i_clear((struct intr *)&(ap->sdi));

			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */

			while(w_clear(&ap->ioctl_timer)); 

			/* 
			 * Loop until w_clear succeeds.
			 * Clear watchdog timer.
			 */

			while(w_clear(&ap->cmd_timer)); 

			if (!sd_open_adaps) {
				i_clear(sd_epow_ptr); /*clear the epow hndlr*/
				xmfree((char *)sd_epow_ptr, (heapaddr_t)
                                        pinned_heap);
				(void)unpincode((int (*)()) sd_intr);
				/*
				 * Stop global Activity Timer Here
				 */
				w_stop(&activity_timer);

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&activity_timer)); 

				/* 
				 * De-Register Comp Dump Tbl 
				 */
				(void) dmp_del((struct cdt (*)())
					sd_adap_cdt_func);
			}
                        d_mask(ap->dma_channel); /* mask dma channel*/
	                d_clear(ap->dma_channel); /* clear dma channel*/
			return(EIO);
		}
                BUSIO_DET(base);

                /*
                 * reserve TCWs for the small xfer area in the tcw table
                 */
                for (ii=ap->sta_tcw_start; ii<(ap->sta_tcw_start + 
		   SD_NUM_STA_TCWS) ; ii++)
        	   SD_GETTAG(ap->tcw_free_list[(int)(ii/SD_BITS_PER_WORD)],ii);

                /*
                 * reserve TCW for the mailbox xfer area in the tcw table
                 */
                for (ii=ap->mb_tcw_start; ii<(ap->mb_tcw_start + 
		   SD_NUM_MBA_TCWS); ii++)
        	   SD_GETTAG(ap->tcw_free_list[(int)(ii/SD_BITS_PER_WORD)],ii);
		/*
		 * if not diagnostic open, Set up the adapter parameters ...
		 */ 
		if (!(ext & SC_DIAGNOSTIC))
			if ( rc = sd_set_adap_parms_disable(ap, (char)TRUE)) {
                        	/*
	                         * couldn't issue set adap parms
				 */
				(void)xmfree((caddr_t)ap->STA[0].stap,
					pinned_heap);
				ap->opened = FALSE;  /* mark adapter closed */
				sd_open_adaps--;   /* dec opened adaps cnt */
				/* 
				 * Disable Interrupts on Adapter ... 
				 * Notice, not critical path
				 */
		                base = (uint)BUSIO_ATT(ap->dds.bus_id,
					ap->dds.base_addr);
		                SD_PUTC((base + SD_CTRL), SD_INT_DISABLE);
		                BUSIO_DET(base);
				/* 
				 * disable this card 
				 */
		        	ap->pos2 &= ~SD_CARD_ENABLE;
			        sd_write_POS(ap,SD_POS2,ap->pos2);
				i_clear((struct intr *)&(ap->sdi));

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&ap->ioctl_timer)); 

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

				while(w_clear(&ap->cmd_timer)); 
	                        if (!sd_open_adaps) {
					i_clear(sd_epow_ptr);
					xmfree((char *)sd_epow_ptr, (heapaddr_t)
       						pinned_heap);
	                                (void)unpincode((int (*)()) sd_intr);
					/*
					 * Stop global Activity Timer Here
					 */
					w_stop(&activity_timer);

					/* 
					 * Loop until w_clear succeeds.
					 * Clear watchdog timer.
					 */

					while(w_clear(&activity_timer));

					/* 
					 * De-Register Comp Dump Tbl 
					 */
					(void) dmp_del(
					  (struct cdt (*)())sd_adap_cdt_func);
	                        }
                        	d_mask(ap->dma_channel); /* mask dma channel*/
	                        d_clear(ap->dma_channel); /* clear dma channel*/
				return(EIO);
			}

	}

	/*
	 * We've successfully opened the adapter, 
	 * so set necessary variables ....
	 */
	if ( chan ) {
		/*
		 * if this is an open from the file system, then set the
		 * fs_open flag
		 */
		ap->fs_open = TRUE;
		if ( ext & SC_DIAGNOSTIC )
			/* 
			 * if this is diag open, set flag 
			 */
			ap->diag_mode = TRUE;	
	} else if (ext & SD_DAEMON) {
		/* 
		 * if this is the daemon, set daemon open flag 
		 */
		ap->daemon_pid = getpid();
		ap->daemon_open = TRUE;	 
	}
	
	else
		/* 
		 * else must be internal open, set internal open flag 
		 */
		ap->internal_open = TRUE;   


	return(0);
}


/*
 * NAME: sd_ctrl_open
 *
 * FUNCTION: Opens Serial DASD Controller
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                             
 *
 * (NOTES:) Possible operations : Extended parameter could specify specific
 *	type of open :
 *
 *	SC_DIAGNOSTIC:	Places the controller in diagnostic mode
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_ctrl_info - controller info structure
 *
 * RETURNS: 	Results of privcheck call, if improper authority
 *		Results of sd_adap_open call, if error
 *		0         - successful completion
 *		ENXIO	  - controller not configured
 *		EACCES	  - controller already open in diagnostic mode, or this
 *			    controller's adapter is open in diagnostic mode.
 *			  - diagnostic mode requested and controller open
 */

int sd_ctrl_open(
struct sd_ctrl_info *cp,
int     rwflag,
int     chan,
int     ext)
{

	int	rc;			/* return code */
	
	if ((cp->ap->diag_mode) || (cp->diag_mode))	
		/*
		 * if this controllers adapter is in diagnostic mode
		 * or this controller is in diagnostic mode return error 
		 */
		return(EACCES);

	if ((cp->opened) && (ext & SC_DIAGNOSTIC))	
		/*
		 * if controller open, and diagnostic mode requested, 
		 * return error.
		 */
		return(EACCES);

	if (!cp->opened) {		
		/*
		 * If first open to this controller
		 */

		/*
		 * open this controllers adapter 
		 */
		lockl(&(cp->ap->dev_lock),LOCK_SHORT);
		rc = sd_adap_open(cp->ap,minor(cp->dds.adapter_devno), rwflag, 
			0, 0);
		unlockl(&(cp->ap->dev_lock));

		if ( rc )
			return(rc);

		cp->opened = TRUE;  	/* flag this controller as open  */
		/* 
		 * increment number of open controllers on this adapter
		 */
		cp->ap->open_ctrls++;	
		/*
		 * increment number of open controllers in system
		 */
		if (!sd_open_ctrls) {
			/* 
			 * if this is the first controller open in the system
			 * then register component dump
			 */

			/* 
			 * Register Component Dump Table Function
			 */
			(void) dmp_add((struct cdt (*)())sd_ctrl_cdt_func);
		}
		sd_open_ctrls++;		

		/*
		 * Initialize any first open variables ... 
		 * controller state, watchdog routines, etc....
		 */
		cp->status = 0;

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
 
                while(w_init(&cp->ioctl_timer));  

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
 
                while(w_init(&cp->cmd_timer));    
	}

	/*
	 * We've successfully opened the controller, 
	 * so set necessary variables ....
	 */
	if ( chan ) {
		/*
		 * if this is an open from the file system, then set the
		 * fs_open flag
		 */
		cp->fs_open = TRUE;
		if ( ext & SC_DIAGNOSTIC )
			/* 
			 * if this is diag open, set flag 
			 */
			cp->diag_mode = TRUE;	
	} else
		/* 
		 * else must be internal open, set internal open flag 
		 */
		cp->internal_open = TRUE;   

	return(0);
}
	
/*
 * NAME: sd_dasd_open
 *
 * FUNCTION: Opens Serial DASD
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	and can page fault.                             
 *
 * (NOTES:) Possible operations : Extended parameter could specify specific
 *	type of open :
 *
 *	SC_DIAGNOSTIC:	Places the dasd in diagnostic mode
 *	SC_FORCED_OPEN: Performs controller reset to override reservation
 *	SD_NO_RESERVE:  Does not reserve dasd on open
 *	SC_RETAIN_RESERVATION :  Does not release reservation on close.
 *
 * (RECOVERY OPERATION:)  If a failure occurs, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_dasd_info - dasd info structure
 *
 * RETURNS: 	Results of privcheck call, if improper authority
 *		Results of sd_ctrl_open call, if error
 *		0         - successful completion
 *		ENXIO	  - dasd not configured
 *		EACCES	  - dasd already open in diagnostic mode, or this
 *			    dasd's controller is open in diagnostic mode.
 *			  - diagnostic mode requested and dasd open
 *		EIO	  - error starting command
 */

int sd_dasd_open(
struct sd_dasd_info *dp,
int     rwflag,
int     chan,
int     ext)
{
	int	rc;			/* return code */


	if ((dp->cp->diag_mode) || (dp->diag_mode))	
		/*
		 * if this dasds controller is in diagnostic mode
		 * or this dasd is in diagnostic mode return error 
		 */
		return(EACCES);	


	if ((dp->opened) && (ext & SC_DIAGNOSTIC))	
		/*
		 * if dasd open, and diagnostic mode requested, return error.
		 */
		return(EACCES);

	if (!dp->opened) {		
		/* 
		 * If first open to this dasd
		 */
		/*
		 * open this dasd's controller
		 */
		lockl(&(dp->cp->dev_lock),LOCK_SHORT);
		rc = sd_ctrl_open(dp->cp, rwflag, 0, 0);
		unlockl(&(dp->cp->dev_lock));

		if (rc)
			/* 
			 * controller open failed
			 */
			return(rc);

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
 
                while(w_init(&dp->cmd_timer));    

		if (ext & SC_FORCED_OPEN) {
			/*
			 * This is a forced open, issue dasd reset
			 * to clear reservation   
			 */
			dp->forced_open = TRUE;
			dp->dasd_result = FALSE;
			sd_reset_quiesce_disable((struct sd_adap_info *)dp,
				(uchar)SD_RESET_OP, (uchar)SD_DASD_CMD); 
			sd_start_disable(dp->ap);
			sd_sleep(dp->ap,&dp->forced_open, &dp->dasd_event);
			if (dp->dasd_result) {
				/*
				 * if bad results from reset 
				 */
				lockl(&(dp->cp->dev_lock),LOCK_SHORT);
				sd_ctrl_close(dp->cp,0, 0);
				unlockl(&(dp->cp->dev_lock));

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

                		while(w_clear(&dp->cmd_timer));  
				return(EIO);
			}
		}

		if (!(ext & SC_DIAGNOSTIC)) {		
			/*
			 * if Diagnostic mode not requested ...
			 */
			if (ext & SD_NO_RESERVE)	
				/*
				 * if no reserve requested, set flag
				 */
				dp->no_reserve = TRUE;

			/*
			 * Start the reset cycle Here ... 
			 * Start Unit, Test Unit Ready, Reserve, 
			 * Mode Sense, Mode Select, Inquiry, Read Capacity.
			 */
			dp->seq_not_done = TRUE;
			dp->dasd_result = FALSE;
			if (sd_start_unit_disable(dp, (char)TRUE)) {
				/*
				 * if start unit already active
				 * NOTICE: This should NEVER happen.
				 */

				lockl(&(dp->cp->dev_lock),LOCK_SHORT);
				sd_ctrl_close(dp->cp,0, 0);
				unlockl(&(dp->cp->dev_lock));

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

                		while(w_clear(&dp->cmd_timer));  
				return(EIO);
			}
			sd_start_disable(dp->ap);
			sd_sleep(dp->ap,&dp->seq_not_done,
					 &dp->dasd_event);
			if (dp->dasd_result) {
				/*
				 * if bad results from sequence
				 * close his controller
				 */

				lockl(&(dp->cp->dev_lock),LOCK_SHORT);
				sd_ctrl_close(dp->cp,0, 0);
				unlockl(&(dp->cp->dev_lock));

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

                		while(w_clear(&dp->cmd_timer));  
				return(dp->dasd_result);
			}


		       /*
		 	* Read the capacity of the first cylinder for our
		 	* Elevator Algorithm
		 	*/
			dp->seq_not_done = TRUE;
			dp->dasd_result = FALSE;
			sd_read_cap_disable(dp,TRUE); 
			sd_start_disable(dp->ap);
			sd_sleep(dp->ap,&dp->seq_not_done,&dp->dasd_event);
			if (dp->dasd_result) {
				/*
			 	* if bad results from sequence
			 	* close his controller
			 	*/
				dp->seq_not_done = TRUE;
				sd_release_disable(dp,SD_RELEASE);
				sd_start_disable(dp->ap);
				sd_sleep(dp->ap,&dp->seq_not_done,
					&dp->dasd_event);

				lockl(&(dp->cp->dev_lock),LOCK_SHORT);
				sd_ctrl_close(dp->cp,0, 0);
				unlockl(&(dp->cp->dev_lock));

				/* 
				 * Loop until w_clear succeeds.
				 * Clear watchdog timer.
				 */

                		while(w_clear(&dp->cmd_timer));  
				return(EIO);
			}
		  
		}
		dp->opened = TRUE;  	/* flag this dasd as open */ 
		/*
		 * increment number of dasd open on his controllers 
		 */
		dp->cp->open_dasd++;	
		/*
		 * increment number of open dasd in system
		 */
		if (!sd_open_dasd) {
			/* 
			 * if this is the first open to any dasd in the
			 * system, then register comp dump
			 */

			/* 
			 * Register Component Dump Table Function 
			 */
			(void) dmp_add((struct cdt (*)())sd_dasd_cdt_func);
		}
		sd_open_dasd++;		
		/*
		 * Initialize any first open variables ... 
		 * watchdog routines, etc....
		 */
		dp->status = 0;

		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
                while(w_init(&dp->ioctl_timer)); 


		/*
		 * Loop until w_init succeeds. 
		 * init a watchdog timer service.
		 */ 
                while(w_init(&dp->query_timer));    

		/*
		 * Set DASD's maximum transfer size
		 */
		dp->max_transfer = SD_DASD_MAX_TRANSFER;

		if (dp->dds.max_coalesce > dp->max_transfer) {
			/*
			 * if max coalesce was defined larger than our 
			 * maximum transfer, reset max coalesce to our
			 * maximum transfer.
			 */
			dp->dds.max_coalesce = dp->max_transfer;
		}
	}

	/*
	 * We've successfully opened the dasd, 
	 * so set necessary variables ....
	 */
	if ( ext & SC_DIAGNOSTIC ) {
		if ((dp->errhead != NULL) && 
		    (dp->errhead->cmd_info == SD_START_UNIT)) {
			/*
			 * we may have left a start unit on the errqueue if
			 * we failed the dasd, but that's OK, just dequeue it.
			 */
			sd_d_q_cmd_disable(dp->errhead);
			sd_free_cmd_disable(&dp->restart);
		}
		dp->diag_mode = TRUE; /* if this is diag open, set flag */
	}
	if ( ext & SC_RETAIN_RESERVATION )
		dp->retain_reservation = TRUE;	/*  set flag */

	return(0);
}




