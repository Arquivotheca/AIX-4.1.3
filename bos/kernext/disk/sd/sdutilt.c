static char sccsid[] = "@(#)03	1.11  src/bos/kernext/disk/sd/sdutilt.c, sysxdisk, bos411, 9428A410j 3/16/94 10:29:02";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_rebuild_cdt(), sd_alloc_adap(), sd_free_adap()
 *	      sd_alloc_ctrl(), sd_free_ctrl(), sd_alloc_dasd(), sd_free_dasd()
 *	      sd_setup_adap(), sd_get_vpd()
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

#include <sys/sd.h>
#include <sys/malloc.h>
#include <sys/sleep.h>
#include <sys/errno.h>

/*
 * NAME: sd_rebuild_cdt
 *
 * FUNCTION: Reallocates the Component Dump table size after configuring
 *	a new device, or after unconfiguring a device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config to rebuild the component dump table entry.
 *
 * (RECOVERY OPERATION:)  If a failure occures, the correct errno is
 *	returned, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	cdt_head - structure used for component dump
 *			cdt_entry - structure used for component dump 
 *
 * RETURNS: 	0         - successful completion
 * 		ENOMEM	  - couldn't malloc memory
 */
int sd_rebuild_cdt(
		   int statics,
		   int count, 
		   struct cdt **cdt_ptr, 
		   char *id)  
{
	int 		size;        /* new size of component dump table */
	struct cdt  	*old_cdt;    /* previous component dump table 	 */
	
	/*
	 * Allocate memory for component dump table.
	 */

	if (!count) {
		/*
		 * if no more inited
		 */
		xmfree((caddr_t) *cdt_ptr,(heapaddr_t)pinned_heap);
		*cdt_ptr = NULL;
		return(0);
	}
		
	size = sizeof(struct cdt_head) + (statics + count) * 
		sizeof(struct cdt_entry);
	old_cdt = *cdt_ptr;
	*cdt_ptr = (struct cdt *) xmalloc(size, 2, pinned_heap);
	if (*cdt_ptr == NULL) {
		*cdt_ptr = old_cdt;
		return(ENOMEM);
	}
	if (old_cdt != NULL)
		xmfree((caddr_t) old_cdt,(heapaddr_t)pinned_heap);
	bzero(*cdt_ptr, size);
	(*cdt_ptr)->_cdt_head._cdt_magic = DMP_MAGIC;
	bcopy(id, (*cdt_ptr)->_cdt_head._cdt_name, 9);
	(*cdt_ptr)->_cdt_head._cdt_len = size;
	return(0);
}





/*
 * NAME: sd_alloc_adap
 *
 * FUNCTION: Allocates memory for adapter info structure, and initializes
 *	necessary fields.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  Recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *			sd_mbox	     - mailbox structure
 *
 * RETURNS: 	Valid adapter pointer if successful
 *		NULL	  - couldn't malloc memory
 */

struct sd_adap_info *sd_alloc_adap(
dev_t   devno,
uint    tcws)
{
        struct sd_adap_info     *ap,
                                *ptr,
				*prev;
	struct sd_cmd 		*cmd;
        int                     ii,
				index;
	char			done;

	/*
	 * malloc adapter struct space word-aligned from pinned_heap
	 */
	ap = (struct sd_adap_info *)xmalloc(sizeof(struct sd_adap_info), 
		2, pinned_heap);
	if (ap == NULL) {
		/*
		 * xmalloc failed
		 */

		return(ap);
	}       
        /*
         * initially, clear the adapter structure
         */
        bzero(ap, sizeof(struct sd_adap_info));

	/*
	 * Allocate Mailbox Transfer Area , page aligned
	 */
	ap->MB = (struct sd_mbox *)xmalloc(SD_MBA_ALLOC_SIZE, PGSHIFT, 
		pinned_heap);
	if (ap->MB == NULL){
		/*
		 * couldn't allocate mailbox area
		 */
		(void)xmfree((caddr_t)ap,pinned_heap); /* so free adap area */
		return(NULL);
	} /* successfully malloc'ed and pinned STA */

        /*
         * init the needed adapter information
         */
        ap->devno = devno;               /* store the adap devno  */
        ap->asynch_event = EVENT_NULL;   /* event word for async events */
        ap->adap_event = EVENT_NULL;     /* event word for adapter events */
        ap->resources = EVENT_NULL;      /* event word for adapter resources */
        ap->ioctl_event = EVENT_NULL;    /* event word for ioctl events */
        ap->open_no1_event = EVENT_NULL; /* event word for 1st ever open */
	ap->dev_lock = LOCK_AVAIL;	 /* set lock word to available */
	ap->adap_resources = TRUE;	 /* set flag for resources available */

        ap->xmem_buf.aspace_id = XMEM_GLOBAL;

        /*
         * set up intr structure
         */
        ap->sdi.intr_st.next = (struct intr *)NULL;
        ap->sdi.intr_st.flags = 0;         /* 0 = shared intr level */
        ap->sdi.intr_st.handler = (int(*)())sd_intr;
	ap->sdi.ap = ap;
        /*
         * set up adapter ioctl watchdog structure
         */
        ap->ioctl_timer.watch.restart = 10; /* average timeout 10 sec */
        ap->ioctl_timer.watch.func = (void(*)())sd_ioctl_timer;
        ap->ioctl_timer.pointer = (void *)ap;      /*save pointer for watchdog*/
        /*
         * set up adapter cmd watchdog structure
         */
        ap->cmd_timer.watch.restart = 10; /* average timeout 10 sec */
        ap->cmd_timer.watch.func = (void(*)())sd_cmd_timer;
	/* 
	 * allocate timer for resets
	 */
        ap->reset_timer = (struct trb *)talloc();  
        ap->reset_timer->flags = 0; 
        ap->reset_timer->func = (void (*)())sd_wait_reset_disable;
	ap->reset_timer->func_data = (ulong)ap;

	/* 
	 * allocate timer for halt adapter
	 */
        ap->halt_timer = (struct trb *)talloc();  
        ap->halt_timer->flags = 0;   
        ap->halt_timer->func = (void (*)())sd_halt_adap_disable;
	ap->halt_timer->func_data = (ulong)ap;

	/* 
	 * allocate timer for delayed operation
	 */
        ap->delay_timer = (struct trb *)talloc();  
        ap->delay_timer->flags = 0;   
        ap->delay_timer->func = (void (*)())sd_delay_timer;
	ap->delay_timer->func_data = (ulong)ap;

        /*
         * Initialize cmd structures
         */
        for (ii=0; ii < SD_NUM_MB; ii++) {
                cmd = (struct sd_cmd *)&(ap->cmds[ ii]);
		bzero(cmd, sizeof(struct sd_cmd)); /* zero cmd structure */
		cmd->tag = (uchar)ii;		/* reinitialize cmd tags */
                cmd->ap = ap;			/* store this adap pointer */
                cmd->sta_index = (char)-1;	/* index into sta structs */
        }
        /*
         * Initialize the Free CMD List
         */
        ap->free_cmd_list[0] = 0x7fffffff;  /* kill #0 */
	for (ii=1; ii< SD_NUM_CMD_WORDS; ii++)
        	ap->free_cmd_list[ii] = 0xffffffff;

        /*
         * Initialize the Free Event List
	 */
	for (ii=0; ii< SD_NUM_EVENT_WORDS; ii++)
	        ap->free_event_list[ii] = 0xffffffff;

        /*
         * calculate the pointer to the reserved TCW mngmt table
         */
	/*
	 * allocate memory for tcw management words 
	 */
	ap->num_tcws = tcws;
	ap->num_tcw_words = tcws / SD_BITS_PER_WORD; 
	if (tcws % SD_BITS_PER_WORD)
		/*
		 * if there is a remainder,
		 * increment number of words needed.
		 */
		ap->num_tcw_words++;
	ap->tcw_free_list = (uint *)xmalloc((sizeof(uint) * ap->num_tcw_words),
		2, pinned_heap);
	if (ap->tcw_free_list == NULL) {
		/*
		 * xmalloc failed
		 */
		(void)xmfree((caddr_t)ap->MB,pinned_heap); 
		(void)xmfree((caddr_t)ap,pinned_heap); 
		return(NULL);
	}       
        bzero(ap->tcw_free_list, (sizeof(uint) * ap->num_tcw_words));

	/* 
	 * add this adapter pointer to hash table 
	 */

	index = (SD_ADAP_TBL_SIZE - 1) & (minor(devno)); /* compute index */
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_adap_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr != NULL ) { 	     /* if this spot is not empty */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {   /* if table entry was empty */
				sd_adap_table[index] = ap; /* set to this */
			} else {		       /* else put at end */
				prev->hash_next = ap;	   /* of the list */
			}
		}
	} while (!done);
			
        return(ap);

}  /* end sd_alloc_adap */


/*
 * NAME: sd_free_adap
 *
 * FUNCTION: Frees memory for adapter info structure
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  None
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *			sd_mbox	     - mailbox structure
 *
 * RETURNS: 	Void
 */

void sd_free_adap(
struct sd_adap_info *ap)
{
        struct sd_adap_info     *ptr,
				*prev;
        int                     index;
	char			done;

	/* 
	 * remove this adapter pointer from hash table 
	 */

	index = (SD_ADAP_TBL_SIZE - 1) & (minor(ap->devno)); 
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_adap_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr->devno != ap->devno ) { 
			/* 
			 * if this is not the one 
			 */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {      /* if table entry was it */
				sd_adap_table[index] = NULL;  /* clear it */
			} else {		      /* else repair link */
				prev->hash_next = ap->hash_next;	
			}
		}
	} while (!done);

	/*
	 * Stop and free reset timer
	 */
	while(tstop(ap->reset_timer));
	tfree(ap->reset_timer);
	/*
	 * Stop and free halt timer
	 */
	while(tstop(ap->halt_timer));
	tfree(ap->halt_timer);
	/*
	 * Stop and free delay timer
	 */
	while(tstop(ap->delay_timer));
	tfree(ap->delay_timer);
			
	/*
	 * Free the adapter info memory, tcw management area,
	 * and Mailbox area
	 */
	(void)xmfree((caddr_t)ap->MB,pinned_heap);
	(void)xmfree((caddr_t)ap->tcw_free_list,pinned_heap);
	(void)xmfree((caddr_t)ap,pinned_heap);
		
        return;

}  /* end sd_free_adap */


/*
 * NAME: sd_alloc_ctrl
 *
 * FUNCTION: Allocates memory for controller info structure
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  Recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_ctrl_info - controller info structure
 *
 * RETURNS: 	Valid controller info pointer if successful
 *		NULL	- couldn't malloc memory
 */

struct sd_ctrl_info *sd_alloc_ctrl(
dev_t   devno)
{
        struct sd_ctrl_info     *cp,
                                *ptr,
				*prev;
        int                     index;
	char			done;

	/*
	 * malloc controller struct space word-alligned from pinned_heap
	 */
	cp = (struct sd_ctrl_info *)xmalloc(sizeof(struct sd_ctrl_info), 
		2,pinned_heap);
	if (cp == NULL) {
		/*
		 * xmalloc failed
		 */
		return(cp);
	}       

        /*
         * initially, clear the controller structure
         */
        bzero(cp, sizeof(struct sd_ctrl_info));
        /*
         * init the needed controller information
         */
        cp->devno = devno;                     /* store the ctrl devno  */
        cp->ioctl_event = EVENT_NULL;    /* event word for ioctl events */
	cp->dev_lock = LOCK_AVAIL;	  /* set lock word to available */

        /*
         * set up controller ioctl watchdog structure
         */
        cp->ioctl_timer.watch.restart = 10; /* average timeout 10 sec */
        cp->ioctl_timer.watch.func = (void(*)())sd_ioctl_timer;
        cp->ioctl_timer.pointer = (void *)cp;   /*save pointer for watchdog*/
        /*
         * set up controller cmd watchdog structure
         */
        cp->cmd_timer.watch.restart = 10; /* average timeout 10 sec */
        cp->cmd_timer.watch.func = (void(*)())sd_cmd_timer;
	/* 
	 * allocate timer for delayed operation
	 */
        cp->delay_timer = (struct trb *)talloc();  
        cp->delay_timer->flags = 0;   
        cp->delay_timer->func = (void (*)())sd_delay_timer;
	cp->delay_timer->func_data = (ulong)cp;

	/* 
	 * add this controller pointer to hash table 
	 */

	index = (SD_CTRL_TBL_SIZE - 1) & (minor(devno)); /* compute index */
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_ctrl_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr != NULL ) { 	     /* if this spot is not empty */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {   /* if table entry was empty */
				sd_ctrl_table[index] = cp; /* set to this */
			} else {		       /* else put at end */
				prev->hash_next = cp;	   /* of the list */
			}
		}
	} while (!done);

        return(cp);

}  /* end sd_alloc_ctrl */


/*
 * NAME: sd_free_ctrl
 *
 * FUNCTION: Frees memory for controller info structure
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  None.                         
 *
 * (DATA STRUCTURES:) 	sd_ctrl_info - controller info structure
 *
 * RETURNS: 	Void
 */

void sd_free_ctrl(
struct sd_ctrl_info *cp)
{
        struct sd_ctrl_info     *ptr,
				*prev;
        int                     index;
	char			done;

	/* 
	 * remove this controller pointer from hash table 
	 */

	index = (SD_CTRL_TBL_SIZE - 1) & (minor(cp->devno)); 
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_ctrl_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr->devno != cp->devno ) { 
			/* 
			 * if this is not the one 
			 */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {      /* if table entry was it */
				sd_ctrl_table[index] = NULL;  /* clear it */
			} else {		      /* else repair link */
				prev->hash_next = cp->hash_next;	
			}
		}
	} while (!done);
			
	/*
	 * Stop and free delay timer
	 */
	while(tstop(cp->delay_timer));
	tfree(cp->delay_timer);
	/*
	 * Free the controller info memory
	 */
	(void)xmfree((caddr_t)cp,pinned_heap);
		
        return;

}  /* end sd_free_ctrl */


/*
 * NAME: sd_alloc_dasd
 *
 * FUNCTION: Allocates memory for dasd info structure, and initializes 
 *	necessary fields.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  Recovery is up to the caller.
 *
 * (DATA STRUCTURES:) 	sd_dasd_info - dasd info structure
 *
 * RETURNS: 	Valid dasd info pointer if successful
 *		NULL	- couldn't malloc memory
 */

struct sd_dasd_info *sd_alloc_dasd(
dev_t   devno,
int	queue_depth)
{
        struct sd_dasd_info     *dp,
                                *ptr,
				*prev;
        int                     index;
	char			done;

	/*
	 * malloc disk struct space word-alligned from pinned_heap
	 */
	dp = (struct sd_dasd_info *)xmalloc(sizeof(struct sd_dasd_info), 
		2,pinned_heap);
	if (dp == NULL) {
		/*
		 * xmalloc failed
		 */
		return(dp);
	}       

        /*
         * initially, clear the dasd structure
         */
        bzero(dp, sizeof(struct sd_dasd_info));
        /*
         * init the needed dasd information
         */
        dp->devno = devno;                     /* store the ctrl devno  */
        dp->ioctl_event = EVENT_NULL;    /* event word for ioctl events */
	dp->dev_lock = LOCK_AVAIL;	  /* set lock word to available */
        dp->dasd_event = EVENT_NULL;      /* event word for dasd events */

        /*
         * set up dasd ioctl watchdog structure
         */
        dp->ioctl_timer.watch.restart = 10; /* average timeout 10 sec */
        dp->ioctl_timer.watch.func = (void(*)())sd_ioctl_timer;
        dp->ioctl_timer.pointer = (void *)dp;   /*save pointer for watchdog*/
        /*
         * set up dasd cmd watchdog structure
         */
        dp->cmd_timer.watch.restart = 10; /* average timeout 10 sec */
        dp->cmd_timer.watch.func = (void(*)())sd_cmd_timer;
        /*
         * set up dasd query device watchdog structure
         */
        dp->query_timer.watch.restart = 4; /* average timeout 4 sec */
        dp->query_timer.watch.func = (void(*)())sd_query_timer;
	/* 
	 * allocate timer for delayed operation
	 */
        dp->delay_timer = (struct trb *)talloc();  
        dp->delay_timer->flags = 0;   
        dp->delay_timer->func = (void (*)())sd_delay_timer;
	dp->delay_timer->func_data = (ulong)dp;

	/* 
	 * add this dasd pointer to hash table 
	 */

	index = (SD_DASD_TBL_SIZE - 1) & (minor(devno)); /* compute index */
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_dasd_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr != NULL ) { 	     /* if this spot is not empty */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {   /* if table entry was empty */
				sd_dasd_table[index] = dp; /* set to this */
			} else {		       /* else put at end */
				prev->hash_next = dp;	   /* of the list */
			}
		}
	} while (!done);

        return(dp);

}  /* end sd_alloc_dasd */


/*
 * NAME: sd_free_dasd
 *
 * FUNCTION: Frees memory for dasd info structure
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level
 *	by sd_config 
 *
 * (RECOVERY OPERATION:)  None.                         
 *
 * (DATA STRUCTURES:) 	sd_dasd_info - controller info structure
 *
 * RETURNS: 	Void
 */

void sd_free_dasd(
struct sd_dasd_info *dp)
{
        struct sd_dasd_info     *ptr,
				*prev;
        int                     index;
	char			done;

	/* 
	 * remove this dasd pointer from hash table 
	 */

	index = (SD_DASD_TBL_SIZE - 1) & (minor(dp->devno)); 
	done = FALSE;					/* flag when done */
	prev = NULL;			  /* set previous pointer to null */
	ptr = sd_dasd_table[index];	        /* set ptr to table entry */
	do {	 		        /* until we have put it somewhere */
		if ( ptr->devno != dp->devno ) { 
			/* 
			 * if this is not the one 
			 */
			prev = ptr;           /* set previous to this one */	
			ptr = ptr->hash_next;       /* walk the hash list */
		} else {
			done = TRUE;		         /* set done flag */
			if (prev == NULL) {      /* if table entry was it */
				sd_dasd_table[index] = NULL;  /* clear it */
			} else {		      /* else repair link */
				prev->hash_next = dp->hash_next;	
			}
		}
	} while (!done);
			
	/*
	 * Stop and free delay timer
	 */
	while(tstop(dp->delay_timer));
	tfree(dp->delay_timer);
	/*
	 * Free the dasd info memory
	 */
	(void)xmfree((caddr_t)dp,pinned_heap);
		
        return;

}  /* end sd_free_dasd */


/*
 * NAME: sd_setup_adap
 *
 * FUNCTION: Sets up Adapter POS registers
 *
 * EXECUTION ENVIRONMENT: This routine is called in the process environment
 *	by sd_config.
 *
 * (RECOVERY OPERATION:) Recovery is left up to the caller
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *
 * RETURNS: 
 *           0         - successful completion
 *	     EIO       - error reading/writing POS register
 *		       - card diagnostics did not complete
 *	     EPERM     - level of Device Driver won't support this adapter
 */

int
sd_setup_adap(
struct sd_adap_info      *ap)
{
        int     busid;
        uchar   ilvl,
                data0,
		data1,
		vpd[6],
		streaming = 0x00;
	ushort	dd_level;

        /*
         * check here to make sure card is ready for POS reg set-up
         */
        ap->IPL_tmr_cnt = 0;    /* set-up initial value */
	do {
		data0 = sd_read_POS(ap,SD_POS0);
		data1 = sd_read_POS(ap,SD_POS1);
		busid = (data0 << 8) + data1;
		if (busid == SD_ADAP_ID) {
			/*
			 * both POS0 and POS1 been set to proper values,
			 * card is ready for POS reg loading
			 */
			break;
		} else {
			if (++ap->IPL_tmr_cnt > SD_MAX_IPL_RETRIES)
				/*
				 * if out of retries
				 */
				return(EIO);
		}
	} while (TRUE);

        /*
         * set control 1 reg (POS 2) with base address, card disable
         */
        ap->pos2 = ((ap->dds.base_addr>>8) & SD_IOSPACE_MASK);
        if (sd_write_POS(ap,SD_POS2,ap->pos2))
		return(EIO);

        /*
         * set the card address extension regs to 0, to set up interrupt
         * level and arbitration level
         */
        /*
         * point POS6 and POS7 at location 0
         */
        if (sd_write_POS(ap,SD_POS7,(uchar)0x00))
		return(EIO);
        if (sd_write_POS(ap,SD_POS6,(uchar)0x00))
		return(EIO);
        /*
         * set up interrupt level, arbitration level via POS 3
         */
        switch (ap->dds.intr_lvl) {
        case    3:
                ilvl = 0;       break;          /* interrupt level  3 */
        case    4:
                ilvl = 1;       break;          /* interrupt level  4 */
        case    5:
                ilvl = 2;       break;          /* interrupt level  5 */
        case    7:
                ilvl = 3;       break;          /* interrupt level  7 */
        case    10:
                ilvl = 4;       break;          /* interrupt level 10 */
        case    11:
                ilvl = 5;       break;          /* interrupt level 11 */
        case    12:
                ilvl = 6;       break;          /* interrupt level 12 */
        case    14:
                ilvl = 7;       break;          /* interrupt level 14 */
        default:
                return(EIO);    /* catch bad parm   */
        } /* endswitch */

        ap->pos3 = ((ilvl<<4) & SD_ILVL_MASK) | ap->dds.dma_lvl;
        if (sd_write_POS(ap,SD_POS3,ap->pos3))
		return(EIO);

        /*
         * set control 3 (POS 4) reg with DMA enable, Fairness disable
         * streaming disable, mpx streaming disable, DMA mode, no monitor
         * parity enable
         */

	/* 
         * Get the Device Driver Level field out of the Adapter VPD data     
	 * This field starts at sub-address 0x46, for 6 bytes
	 *	Card Level ALC = Streaming Enabled
	 *	Card Level AL8 = 64-bit streaming enabled
	 */ 
	if (sd_get_vpd(ap, vpd, 0x46, 6))
		/*
		 * Error reading Device Driver Level from VPD
		 */
		return(EIO);
	/*
	 * Create comparible Level field
	 */
	dd_level = (vpd[4] << 8) | vpd[5];
	if (dd_level == 0x3030) 
		/*
		 * if this adapter indicates Device Driver level 00, then
		 * disable Streaming
		 */
		streaming = SD_STREAM_ENABLE | SD_MPXSTRM_DISABLE;
   	else if (dd_level == 0x3031)
		/*
		 * else if this adapter indicates Device Driver level 01, then
		 * enable 8-byte streaming
		 */
		streaming = SD_STREAM_ENABLE | SD_MPXSTRM_ENABLE;
	else
		/*
		 * else this adapter not supported by this Device Driver, so
		 * return Error EPERM (Operation not permitted)
		 */
		return(EPERM);

	ap->pos4 = SD_DMA_ENABLE | SD_FAIRNESS_ENABLE | streaming | 
		SD_MONITOR | SD_PARITY_ENABLE | SD_DMA_MODE_3; 

        if (sd_write_POS(ap,SD_POS4,ap->pos4))
		return(EIO);

        /*
         * restore POS6 and POS7 registers
         */
        if (sd_write_POS(ap,SD_POS7,(uchar)0x00))
		return(EIO);
        if (sd_write_POS(ap,SD_POS6,(uchar)0x01))
		return(EIO);

        ap->IPL_tmr_cnt = 0;                    /* reset IPL timer flag */
        return(0);
}  /* end sd_setup_adap */

/*
 * NAME: sd_get_vpd
 *
 * FUNCTION: Reads Adapter Vital Product Data
 *
 * EXECUTION ENVIRONMENT: This routine is called in the process environment
 *	by sd_config and sd_setup_adap, and can page fault.
 *
 * (RECOVERY OPERATION:) Recovery is left up to the caller
 *
 * (DATA STRUCTURES:) 	sd_adap_info - adapter info structure
 *			vpd	     - array of VPD bytes
 *
 * RETURNS: 
 *           0         - successful completion
 *	     EIO       - error reading/writing POS register
 */
int sd_get_vpd(
struct sd_adap_info *ap,
uchar *vpd,
int start,
int count)
{
	int	rc = 0,
		i,
		index;
	char	data;
	
	if (sd_write_POS(ap, (uint)SD_POS7, (uchar)0x00) != 0) {
		return(EIO);
	}
	for (i = start,index = 0; i <= (count+start); i++,index++) {
		if (sd_write_POS(ap, (uint)SD_POS6, (uchar)(i)) != 0) {
			rc = EIO;
			break;
		}
		data = sd_read_POS(ap, SD_POS3);
		vpd[index] = (uchar)data;   /* load the vpd byte */
	}       /* endfor */
	/* 
	* leave POS6 in known state 
	*/
	(void)sd_write_POS(ap, (uint)SD_POS6, (uchar)0x01);
	return(rc);
}
