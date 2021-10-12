static char sccsid[] = "@(#)02  1.20.2.8  src/bos/kernext/disk/sd/sdutilb.c, sysxdisk, bos411, 9428A410j 3/16/94 10:26:33";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_hash(), sd_read_POS(), sd_write_POS(), sd_reload_pos(),
 *      sd_wait_reset_disable(), sd_wait_reset(), sd_restart_adap(), 
 *	sd_q_cmd_disable(), sd_q_cmd(), sd_d_q_cmd_disable() sd_d_q_cmd(),
 *      sd_event_alloc(), sd_cmd_alloc_disable(), sd_cmd_alloc(),
 *      sd_free_cmd_disable(), sd_free_cmd(),sd_fail_cmd(), sd_MB_alloc(), 
 *	sd_free_MB(), sd_TCW_alloc(), sd_TCW_realloc(), sd_TCW_dealloc(), 
 *	sd_STA_alloc(), sd_STA_dealloc(), sd_set_adap_parms_disable(),
 *	sd_set_adap_parms(), sd_cmd_timer(), sd_dma_cleanup(),
 *      sd_request_sense(), sd_start_unit_disable(),sd_start_unit(), 
 *	sd_test_unit_ready(), sd_reserve(),
 *      sd_mode_sense(), sd_mode_select(), sd_mode_select_cmp(),
 *      sd_find_select_index(), sd_compare_blk(), sd_inquiry(), 
 *	sd_read_cap_disable(),sd_read_cap(),sd_release_disable(),sd_release(), 
 *	sd_reset_quiesce_disable(),sd_reset_quiesce(), sd_fail_adap_disable(),
 *	sd_fail_adap(), sd_fail_dasd(),sd_halt_adap_disable(),sd_halt_adap(),
 *	sd_verify(), sd_check_map(),sd_async_event(),sd_sleep(), 
 *	sd_ioctl_timer(), sd_add_chain(), sd_del_chain(), sd_build_cmd(), 
 *	sd_pio_recov(),sd_pio(), sd_epow(), sd_trc_disable(),sd_trc(), 
 *	sd_adap_cdt_func(), sd_ctrl_cdt_func(), sd_dasd_cdt_func(), 
 *	sd_walk_event(), sd_log_dasd_err(), sd_log_dasd_err(), 
 *	sd_log_adap_err(), sd_log_adap_err(), sd_delay(),
 *	sd_delay_timer(), sd_dptrc_disable() sd_dptrc(), sd_fence(),
 *	sd_verify_disable()
 * 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/malloc.h>
#include <sys/sd.h>
#include <sys/errno.h>
#include <sys/lvdd.h>
#include <sys/sleep.h>
#include <sys/except.h>
#include <sys/errids.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/ddconc.h>

extern uchar sd_open_adaps;
extern uchar sd_open_ctrls;
extern uchar sd_open_dasd;
extern char sd_inited_adaps;
extern char sd_inited_ctrls;
extern char sd_inited_dasd;
struct cdt *sd_adap_cdt = NULL;
struct cdt *sd_ctrl_cdt = NULL;
struct cdt *sd_dasd_cdt = NULL;

Simple_lock sd_epow_lock; /* Global EPOW lock for Serial DASD subsystem */
#ifdef DEBUG
char    *topoftrc = "**TOP***";
extern	char	*tcwfree;
#endif

/*
 * NAME: sd_hash
 *
 * FUNCTION: Returns the appropriate info pointer based on the devno
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_adap_table   - Table of adapter info pointers
 *                      sd_ctrl_table   - Table of controller info pointers
 *                      sd_dasd_table   - Table of dasd info pointers
 *
 * RETURNS:     Pointer to info structure for adapter, controller, or dasd
 *              or NULL if devno not found in table.
 */

void *sd_hash(
dev_t   devno)
{
        int     hashvalue,      /* hash value into table */
                index,          /* index into table */
                minor;          /* minor number */
        struct  sd_adap_info **table,   /* pointer to table */
                *ptr;           /* pointer to info structure */
        uint    temp,           /* temp variable */
                old_pri;
        long    majorno;        /* device major number       */
        char    found = FALSE;  /* flag */


        minor = minor(devno);        /* get minor number for this devno */
        if ( minor & SD_DASD_MASK) {
                /*
                 * if this is a dasd
                 */
                temp = (uint)sd_dasd_table;
                hashvalue = SD_DASD_TBL_SIZE - 1;
        } else if ( minor & SD_CTRL_MASK ) {
                /*
                 * if this is a controller
                 */
                temp = (uint)sd_ctrl_table;
                hashvalue = SD_CTRL_TBL_SIZE - 1;
        } else {
                /*
                 * else this is an adapter
                 */
                temp = (uint)sd_adap_table;
                hashvalue = SD_ADAP_TBL_SIZE - 1;
                if (minor & SD_DAEMON_MASK)
                        minor &= ~SD_DAEMON_MASK;
                majorno = major(devno);
                devno = makedev(majorno,minor);
        }

        /*
         * compute the index into the table
         */
        index = minor & hashvalue;

        /*
         * set table pointer to the previously saved pointer.  Notice the
         * casting to allow indexing off of this pointer.  Also, notice that
         * the type of sd_adap_info is used whether this is an adapter,
         * controller, or dasd, relying on the commonality of the tops of each
         * of these structures
         */
        table = (struct sd_adap_info **)temp;
        /*
         * get the pointer to the info structure, again relying on the
         * commonality of the adapter, controller, and dasd structures
         */
        ptr = (struct sd_adap_info *)table[index];
        if ( ptr != NULL ) {
                /*
                 * if the pointer is not null, see if it is the one
                 * we are looking for
                 */
                found = FALSE;
                do {
                        if (ptr->devno == devno)
                                found = TRUE;
                        else
                                ptr = ptr->hash_next;
                } while ((!found) && (ptr != NULL));
        }
        /*
         * Either we found the structure, or there is not one, so return
         * ptr, which will either be NULL or the correct pointer
         */
        return((void *)ptr);
}

/*
 * NAME: sd_read_POS
 *
 * FUNCTION: Reads POS register
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If an error occurs, the correct errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:
 *           Contents of POS register  - successful completion
 *           -1                        - error reading POS register
 */

int sd_read_POS(
struct  sd_adap_info     *ap,
uint                    offset)
{

    int     count, err_flag;
    uchar   val1, val2;
    caddr_t iocc_addr;

    count = 0;
    iocc_addr = IOCC_ATT(ap->dds.bus_id, 0);    /* access the IOCC */
    val1 = BUSIO_GETC((iocc_addr + (ap->dds.slot << 16) + offset));
    val2 = BUSIO_GETC((iocc_addr + (ap->dds.slot << 16) + offset));
    if (val1 != val2) {
        do {
            val1 = val2;
            if (count >= SD_MAX_POS_RETRIES) {
                /*
                 * log as unrecoverable I/O bus problem
                 */
                sd_log_adap_err(ap,(ushort)0x0502,0);
                IOCC_DET(iocc_addr);        /* release the IOCC */
                return(-1);
            }

            count++;
            val2 = BUSIO_GETC((iocc_addr + (ap->dds.slot << 16) + offset));

        } while (val2 != val1);

        /*
         * log as recovered I/O bus problem
         */
        sd_log_adap_err(ap,(ushort)0xF502,0);
    }

    IOCC_DET(iocc_addr);        /* release the IOCC */
    return (val1);
}  /* end sd_read_POS */

/*
 * NAME: sd_write_POS
 *
 * FUNCTION: Writes POS register
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If an error occurs, the correct errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:
 *           0  - successful completion
 *           -1 - error writing POS register
 */

int sd_write_POS(
struct sd_adap_info      *ap,
uint                    offset,
uchar                   data)
{

    int     count;
    caddr_t iocc_addr;
    uchar   val1;

    count = 0;
    iocc_addr = IOCC_ATT(ap->dds.bus_id, 0);    /* access the IOCC */
    BUSIO_PUTC((iocc_addr + (ap->dds.slot << 16) + offset), data);
    if ((offset == SD_POS2) && (data & SD_ADAP_RESET_BIT)) {
        /*
         * if we just reset the adapter, then leave
         */
        IOCC_DET(iocc_addr);        /* remove access to the IOCC */
        return (0);
    }
    val1 = BUSIO_GETC((iocc_addr + (ap->dds.slot << 16) + offset));
    if (data != val1) {
        do {
            BUSIO_PUTC((iocc_addr + (ap->dds.slot << 16) + offset), data);
            if (count >= SD_MAX_POS_RETRIES) {
                /*
                 * log as unrecoverable I/O bus problem
                 */
                sd_log_adap_err(ap,(ushort)0x0502,0);
                IOCC_DET(iocc_addr);        /* remove access to the IOCC */
                return(-1);
            }

            count++;
            val1 = BUSIO_GETC((iocc_addr + (ap->dds.slot << 16) + offset));

        } while (data != val1);

        /*
         * log as recovered I/O bus problem
         */
        sd_log_adap_err(ap,(ushort)0xF502,0);
    }

    IOCC_DET(iocc_addr);        /* remove access to the IOCC */
    return (0);
}



/*
 * NAME: sd_reload_pos
 *
 * FUNCTION: Reloads the POS registers following an adapter reset
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If an error occurs, the correct errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:
 *           0  - successful completion
 *           -1 - error writing POS register(s)
 */

int sd_reload_pos(
struct sd_adap_info *ap)
{

        /*
         * Reload the POS registers, return the result
         * of OR-ing the return codes of each write
         */
        return( sd_write_POS(ap,SD_POS2,ap->pos2)  |
                sd_write_POS(ap,SD_POS7,(uchar)0x00) |
                sd_write_POS(ap,SD_POS6,(uchar)0x00) |
                sd_write_POS(ap,SD_POS3,ap->pos3) |
                sd_write_POS(ap,SD_POS4,ap->pos4) |
                sd_write_POS(ap,SD_POS7,(uchar)0x00) |
                sd_write_POS(ap,SD_POS6,(uchar)0x01));
}

/*
 * NAME: sd_wait_reset_disable
 *
 * FUNCTION: System timer function to determine when an adapter has finished
 *      internal POSTs following an adapter reset.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If the adapter does not come ready in the time allowed
 *      then the timer discontinues to run, and the adapter is reset
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      trb             - System timer structure
 *
 * RETURNS:     Void.
 */

void sd_wait_reset_disable(
struct trb *t)
{
        struct sd_adap_info *ap;
        int     old_pri;

        /*
         * get adapter pointer
         */
        ap = (struct sd_adap_info *)(t->func_data);

        /* disable my irpt level*/
        old_pri = disable_lock(ap->dds.intr_priority, &(ap->spin_lock));

        sd_wait_reset(t);

        unlock_enable(old_pri,&(ap->spin_lock));

}

/*
 * NAME: sd_wait_reset
 *
 * FUNCTION: System timer function to determine when an adapter has finished
 *      internal POSTs following an adapter reset.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault. This routine assumes that 
 *	the caller is disable_locked when calling this routine.
 *
 * (RECOVERY OPERATION:) If the adapter does not come ready in the time allowed
 *      then the timer discontinues to run, and the adapter is reset
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      trb             - System timer structure
 *
 * RETURNS:     Void.
 */

void sd_wait_reset(
struct trb *t)
{
        struct sd_adap_info *ap;
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        int     busid,
	        disk,
	        ctlr;
        uchar   ilvl,
                data0,
                data1;
        uint    base;
        int     old_pri;

        /*
         * get adapter pointer
         */
        ap = (struct sd_adap_info *)(t->func_data);


        /*
         * check here to see if card is ready for POS reg set-up
         */
        data0 = (uchar)sd_read_POS(ap,SD_POS0);
        data1 = (uchar)sd_read_POS(ap,SD_POS1);
        if ((data0 == 0xFF) || (data1 == 0xFF)) {
                /*
                 * Error reading POS, logged in read_POS
                 * This is FATAL, so flush the adapter
                 */
                sd_flush_adap(ap);
                ap->reset_result = -1;

                return;
        }
        busid = (data0 << 8) + data1;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,waitreset, entry,(char)0,(uint)ap->reset_result, 
	       (uint)busid , (uint)ap->IPL_tmr_cnt, (uint)ap->reset_count, 
	       (uint)0);
#endif
#endif
        if (busid == SD_ADAP_ID) {
                /*
                 * both POS0 and POS1 been set to proper values,
                 * card is ready. We can now be in one of 3 situations:
		 * 1. This was the 1st ever open
		 * 2. The reset was due to error recovery.
		 * 3. The reset was due to ioctl request.
                 */
		if( !ap->ever_open ) { /* 116467 (36 lines) */ 
			/*
			 * 1. This was the 1st ever open 
			 */
                        if (++ap->reset_count > SD_MAX_RESET_RETRIES) {
                                /*
                                 * Uh-Oh, what else can we do, we're toes up,
				 * log the error and give up
				 */
                                sd_log_adap_err(ap,(ushort)0x0502,0);

                                return;
                        }
                        if (sd_reload_pos(ap)) {
                                /*
                                 * Couldn't reload POS regs, so
                                 * reset the adapter again
                                 */
                                ap->status &= ~SD_RESET_PENDING;
                                sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                        (uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(ap,waitreset, exit,(char)1, 
				       (uint)ap->reset_count, (uint)0,
				       (uint)0,(uint)0,(uint)0);
#endif
#endif

                                return;
                        }
                        ap->reset_result = TRUE;
                        ap->status &= ~(SD_RESET_PENDING | SD_QUIESCE_PENDING);
			/*
			 * Let sd_open_adap continue.
			 */
			ap->open_no1_intrpt = 0;
			e_wakeup(&ap->open_no1_event);
		} else if (ap->reset_result == SD_RESET_A) {
                        /*
			 * 2. The reset was due to error recovery.
                         * if this is a result of an error recovery reset
                         * make sure our command map is empty, recycle
                         * any commands in the map
                         */
                        sd_fail_adap(ap);
                        /*
                         * increment reset count and see if we've tried this
                         * recovery too many times.  If so, give up
                         */
                        if (++ap->reset_count > SD_MAX_RESET_RETRIES) {
                                /*
                                 * Uh-Oh, what else can we do, we're toes up,
                                 * so flush all commands for this adapter
                                 */
                                sd_flush_adap(ap);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(ap,waitreset, exit,(char)0, 
				       (uint)ap->reset_count, 
				       (uint)0,(uint)0,(uint)0,(uint)0);
#endif
#endif
                                sd_log_adap_err(ap,(ushort)0x0502,0);

                                return;
                        }
                        /*
                         * reload the POS registers
                         */
                        if (sd_reload_pos(ap)) {
                                /*
                                 * Couldn't reload POS regs, so
                                 * reset the adapter again
                                 */
                                ap->status &= ~SD_RESET_PENDING;
                                sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                        (uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(ap,waitreset, exit,(char)1, 
				       (uint)ap->reset_count, (uint)0,
				       (uint)0,(uint)0,(uint)0);
#endif
#endif

                                return;
                        }
                        /*
                         * reinit the mailbox chain
                         */
                        if (sd_restart_adap(ap)) {
                                /*
                                 * Couldn't set up MB chain (PIO error), so
                                 * get out, adapter reset by pio_recov
                                 */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(ap,waitreset, exit,(char)2, 
				       (uint)ap->reset_count, (uint)0,
				       (uint)0,(uint)0,(uint)0);
#endif
#endif

                                return;
                        }
                        /*
                         * clear reset and quiesce pending flags
                         */
                        ap->status &= ~(SD_RESET_PENDING | SD_QUIESCE_PENDING);

                        if (!ap->opened) {
                                /*
                                 * if the adapter isn't open yet, then this
                                 * must be result of PIO error during open,
                                 * don't do any more
                                 */

                                return;
                        }
                        /*
                         * Enable interrupt on the adapter
                         */
                        base = (uint)BUSIO_ATT(ap->dds.bus_id,
                                ap->dds.base_addr);
                        if (SD_PUTC((base + SD_CTRL), SD_INT_ENABLE)) {
                                /*
                                 * if PIO error,get out
                                 */
                                BUSIO_DET(base);
                                return;
                        }
                        BUSIO_DET(base);
			/*
			 * I is just possible that during the 
			 * reset of the adapter card, we missed an
			 * asynchronous event from the subsystem.
			 * If a kernel extension has registered with
			 * any of the disks under this adapter
			 * we must tell it.
			 */
                        for (ctlr=0; ctlr < SD_NUM_CTRLS; ctlr++) {
                            cp = ap->ctrllist[ctlr];
                            if (cp != NULL)
                            for (disk=0; disk < SD_NUM_DASD; disk++) {
                                dp = cp->dasdlist[disk];
                                if (dp != NULL) 
				    if (dp->conc_registered)
					(dp->conc_intr_addr)(NULL,DD_CONC_RESET,0,dp->devno);
			    }
			}
                        			 
                        /*
                         * set up the adapter parameters
                         */
                        sd_set_adap_parms(ap, (uchar)FALSE);
			/*
			 * start delay timer to delay before allowing more
			 * commands 
			 */
			sd_delay(ap, (char)SD_ADAP_CMD, (uint)15000);
                } else {
                        /*
			 * 3. The reset was due to ioctl request.
                         * turn ioctl loose by setting reset_result true
                         */
                        ap->reset_result = TRUE;
                        /*
                         * clear reset and quiesce pending flags
                         */
                        ap->status &= ~(SD_RESET_PENDING | SD_QUIESCE_PENDING);
                }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,waitreset, exit,(char)3, 
		       (uint)ap->reset_count, (uint)0,(uint)0,(uint)0,(uint)0);
#endif
#endif

                return;
        } else {
                /*
                 * card IPL is still going !
                 */
                if (ap->IPL_tmr_cnt >= SD_MAX_IPL_RETRIES) {
                        /*
                         * timed-out waiting for card diag to complete
                         * stop here if we have attempted this too many times
                         */
                        if (++ap->reset_count > SD_MAX_RESET_RETRIES) {
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(ap,waitreset, exit,(char)4, 
				       (uint)ap->reset_count, (uint)0,
				       (uint)0,(uint)0,(uint)0);
#endif
#endif
                                /*
                                 * Time-out waiting for POS value
                                 */
                                /*
                                 * we're toes up,
                                 * so flush all commands for this adapter
                                 */
                                sd_flush_adap(ap);
                                ap->reset_result = -1;
                                sd_log_adap_err(ap,(ushort)0x0502,0);
                                return;
                        }

                        /*
                         * Time out, so log recovered and try again
                         */
                        sd_log_adap_err(ap,(ushort)0xF503,0);
                        /*
                         * otherwise, reset adapter once again.
                         */
                        ap->status &= ~SD_RESET_PENDING;
                        sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                (uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,waitreset, exit,(char)5, 
			       (uint)ap->reset_count, (uint)ap->IPL_tmr_cnt,
			       (uint)0,(uint)0,(uint)0);
#endif
#endif

                        return;
                }
                /*
                 * set timer to come back and check POS0 and POS1 again.
                 * disable interrupts to close window where timer
                 * might complete before executing the sleep
                 */
                ap->reset_timer->timeout.it_value.tv_sec = 1;
                ap->reset_timer->timeout.it_value.tv_nsec = 0;
                if (!(ap->reset_timer->flags & T_ACTIVE))
                        /*
                         * Make sure the timer is not already active before
                         * starting, if it is, then this will eliminate one
                         * of the threads that got us into this situation
                         */
                        tstart(ap->reset_timer);
                ap->IPL_tmr_cnt++;    /* inc the timer counter */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,waitreset, exit,(char)6, 
		       (uint)ap->reset_count, (uint)ap->IPL_tmr_cnt,
		       (uint)0,(uint)0,(uint)0);
#endif
#endif

                return;
        }
}


/*
 * NAME: sd_restart_adap
 *
 * FUNCTION: Re-initializes the adapter Mailbox chain
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *      At the first call, ap->next_tag is zero and tag 1 is assigned as the
 *      first tag to be used.  On subsequent calls, the current value of
 *      ap->next_tag is used to insure that no duplicate tags can exist in the
 *      subsystem.
 *
 * (RECOVERY OPERATION:) If an error occurs, the correct errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           0          - successful completion
 *           EIO        - PIO error
 */

int sd_restart_adap(
struct sd_adap_info   *ap)
{
        int             ii;
        uint            base;
        uchar           ltag;
        struct sd_cmd  *cmd;

        /*
         * init adapter information structure fields
         */

         /*
          * Write 0 to the last tag reg to ensure he's quiet
          * if the last tag register is not already 0.
          */
        base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
        if (SD_GETC(base + SD_LTAG, &ltag)) {
                /*
                 * if PIO error
                 */
                BUSIO_DET(base);
                return(EIO);
        }
        if (ltag != 0)
                if (SD_PUTC(base + SD_LTAG, 0x00)) {
                        /*
                         * if PIO error
                         */
                        BUSIO_DET(base);
                        return(EIO);
                }
        BUSIO_DET(base);

        /*
         * Initialize mailbox tags
         */
        for (ii=0; ii < SD_NUM_MB; ii++) {
                ap->cmd_map[ii] = NULL;       /* init active cmd map to NULL */
                ap->MB[ ii].tag = (uchar)(ii);          /* reinitialize tags */
                ap->MB[ ii].nextmb = (struct sd_mbox *)1;     /* odd pointer */
        }
        /*
         * First time, set next tag to 1, else use current value of next
         * tag to restart the chain.
         */
        if (ap->next_tag == 0)
                ap->next_tag = 1;
        ap->curr_mb = NULL;                /* address of last MB issued */
	ap->mb_word = 0;	   /* set first mb word to look at to 0 */
        /*
         * Initialize the MB Alloc List, making all tags except zero available
         */
        ap->mb_alloc_list[0] = 0x7fffffff;                     /* kill #0 */
        for (ii=1; ii < SD_NUM_MB_WORDS; ii++)
                ap->mb_alloc_list[ii] = 0xffffffff;
        /*
         * Initialize the MB Free List, making all tags zero, (none freed yet)
         */
        for (ii=0; ii < SD_NUM_MB_WORDS; ii++)
                ap->mb_free_list[ii] = 0;

         /*
          * Write the first MB address to MB Pointer Register
          */
        base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
        if (SD_PUTL(base + SD_MBPT,SD_MB_ADDR((uint)ap->base_MB_dma_addr,
                    ap->next_tag))) {
                /*
                 * PIO error
                 */
                BUSIO_DET(base);
                return(EIO);
        }
        BUSIO_DET(base);
        /*
         * open the adapter for business and return
         * protect the quiesce in progress flag so that we do not forget that
         * we are quiescing
         */
        ap->status &= (SD_QUIESCE_PENDING | SD_RESET_PENDING);
        return(0);
}

/*
 * NAME: sd_q_cmd_disable
 *
 * FUNCTION: Places a command on the appropriate device queue or stack
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_q_cmd_disable(
struct  sd_cmd  *cmd,
char    queue)
{
        uint    old_pri;

	/* disable my irpt level*/
        old_pri = disable_lock(cmd->ap->dds.intr_priority,
			       &(cmd->ap->spin_lock));
	sd_q_cmd(cmd,queue);
        unlock_enable(old_pri,&(cmd->ap->spin_lock)); /* reenable interrupts */
}
/*
 * NAME: sd_q_cmd
 *
 * FUNCTION: Places a command on the appropriate device queue or stack
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_q_cmd(
struct  sd_cmd  *cmd,
char    queue)
{
        struct sd_adap_info *ap;
        struct sd_dasd_info *dp;
        struct sd_cmd **head_ptr,
                      **tail_ptr;


        ASSERT(cmd);
	ASSERT(!(cmd->status & SD_QUEUED));
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,qcmd,trc,(char) 0,(uint)cmd, (uint)cmd->cmd_info, 
	       (uint)cmd->type, (uint)queue,(uint)0);
#endif
#endif
        ap = cmd->ap;                           /* set adapter pointer */


        cmd->nextcmd = NULL;   /* set next to null, this will be put on end */

        switch (cmd->type) {            /* depending on the command type */
                case SD_CTRL_CMD:
                        /*
                         * Notice, all controller commands go off of
                         * the controller's adapter queues
                         */
                        cmd->cp->cmds_qed++;
                        if (cmd->cmd_info == SD_IOCTL) {
                                /*
                                 * set head_ptr to adaps ctrl ioctl head and 
				 * tail_ptr to adap's ctrl ioctl tail
                                 */
                                head_ptr = &ap->ctrl_ioctlhead;
                                tail_ptr = &ap->ctrl_ioctltail;
                        } else {
                                /*
                                 * set head_ptr to adaps ctrl error head and 
				 * tail_ptr to adap's ctrl error tail
                                 */
                                head_ptr = &ap->ctrl_errhead;
                                tail_ptr = &ap->ctrl_errtail;
                        }
                        break;
                case SD_ADAP_CMD:
                        if (cmd->cmd_info == SD_IOCTL) {
                                /*
                                 * set head_ptr to adaps ioctlhead and tail_ptr
                                 * to adap's ioctltail
                                 */
                                head_ptr = &ap->ioctlhead;
                                tail_ptr = &ap->ioctltail;
                        } else {
                                /*
                                 * set head_ptr to adaps errhead and tail_ptr
                                 * to adap's errtail
                                 */
                                head_ptr = &ap->errhead;
                                tail_ptr = &ap->errtail;
                        }
                        break;
                case SD_DASD_CMD:
                        dp = cmd->dp;      /* set dasd pointer */
                        if (cmd->cmd_info == SD_IOCTL) {
                                /*
                                 * set head_ptr to dasds ioctlhead and tail_ptr
                                 * to dasds ioctltail
                                 */
                                head_ptr = &dp->ioctlhead;
                                tail_ptr = &dp->ioctltail;
                        } else {
                                /*
                                 * set head_ptr to dasds errhead and tail_ptr
                                 * to dasds errtail
                                 */
                                head_ptr = &dp->errhead;
                                tail_ptr = &dp->errtail;
                        }
                        /*
                         * Make sure the affected dasd is in the start chain
                         */
                        sd_add_chain(dp);
                        break;
                default :
                        ASSERT(FALSE);
        }

        if (queue) {
                /*
                 * Queue this command
                 */
                if (*head_ptr == NULL)
                        /*
                         * if q is empty, set
                         * head to this one
                         */
                        *head_ptr = cmd;
                else
                        /*
                         * else q is not empty, so
                         * put on tail of q
                         */
                        (*tail_ptr)->nextcmd = cmd;
                /*
                 * set new tail to this one
                 */
                *tail_ptr = cmd;
        } else {
                /*
                 * else Stack this command
                 */
                if (*head_ptr == NULL)
                        /*
                         * if q is empty, set
                         * tail to this one also
                         */
                        *tail_ptr = cmd;
                else
                        /*
                         * else not empty, stack this command
                         * on top of the head of
                         */
                        cmd->nextcmd = *head_ptr;

                /*
                 * set new head to this one
                 */
                *head_ptr = cmd;
        }
	/*
	 * Change Commands Status as Queued
	 */
	cmd->status |= SD_QUEUED;


}
/*
 * NAME: sd_d_q_cmd_disable
 *
 * FUNCTION: Removes the specified command from the appropriate device queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_d_q_cmd_disable(
struct sd_cmd *cmd)
{
        uint    old_pri;
        /*
         * Disable Interrupts
         */
        old_pri = disable_lock(cmd->ap->dds.intr_priority,
			       &(cmd->ap->spin_lock));
	sd_d_q_cmd(cmd);
        unlock_enable(old_pri,&(cmd->ap->spin_lock));  
}
/*
 * NAME: sd_d_q_cmd
 *
 * FUNCTION: Removes the specified command from the appropriate device queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. Plus from close on the process level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_d_q_cmd(
struct sd_cmd *cmd)
{

        struct sd_dasd_info *dp=NULL;
        struct sd_cmd **head_ptr,
                      **tail_ptr,
                        *prev=NULL;
        char    done=FALSE;


        ASSERT(cmd);
        ASSERT(cmd->status); /* shouldn't ever dequeue a free command */

        /*
         * set head pointer to prev initially. this will catch an unknown
         * cmd type when compiled without DEBUG
         */
        head_ptr = &prev;
        /*
         * Unlink this command from its list
         */
        switch (cmd->type) {
                case SD_CTRL_CMD:
                        /*
                         * this was a controller command
                         */
                        if (cmd->cmd_info == SD_IOCTL) {
                                /*
                                 * take off controller ioctl queue
                                 */
                                head_ptr = &cmd->ap->ctrl_ioctlhead;
                                tail_ptr = &cmd->ap->ctrl_ioctltail;
                        } else {
                                /*
                                 * take off error queue
                                 */
                                head_ptr = &cmd->ap->ctrl_errhead;
                                tail_ptr = &cmd->ap->ctrl_errtail;
                        }
                        break;
                case SD_ADAP_CMD:
                        /*
                         * this was an adapter command
                         */
                        if (cmd->cmd_info == SD_IOCTL) {
                                /*
                                 * take off ioctl queue
                                 */
                                head_ptr = &cmd->ap->ioctlhead;
                                tail_ptr = &cmd->ap->ioctltail;
                        } else {
                                /*
                                 * take off error queue
                                 */
                                head_ptr = &cmd->ap->errhead;
                                tail_ptr = &cmd->ap->errtail;
                        }
                        break;
                case SD_DASD_CMD:
                        /*
                         * this was a dasd command
                         */
                        if (cmd->cmd_info == SD_IOCTL) {
                                 /*
                                 * take off ioctl queue
                                 */
                                head_ptr = &cmd->dp->ioctlhead;
                                tail_ptr = &cmd->dp->ioctltail;
                        } else {
                                /*
                                 * take off error queue
                                 */
                                head_ptr = &cmd->dp->errhead;
                                tail_ptr = &cmd->dp->errtail;
                        }
                        dp = cmd->dp;    /* set dp to cmd's dp */
                        break;
                default:
                        ASSERT(FALSE);
        }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,dqcmd,trc,(char) 0,(uint)cmd, (uint)cmd->cmd_info, 
	       (uint)cmd->type, (uint)*head_ptr,(uint)((*head_ptr == NULL) ? 0 : (*head_ptr)->nextcmd));
#endif
#endif

        while ((*head_ptr != NULL) && (!done)) {
                if (*head_ptr == cmd) {
                        /*
                         * found it
                         */
                        if (prev == NULL)
                                /*
                                 * Must have been head
                                 */
                                *head_ptr = (*head_ptr)->nextcmd;
                        else
                                /*
                                 * wasn't head, unlink
                                 */
                                prev->nextcmd = (*head_ptr)->nextcmd;
                        if (*tail_ptr == cmd)
			
			    *tail_ptr = prev;
			
			done = TRUE;
		    } else {
                        /*
                         * walk to next command
                         */
                        prev = *head_ptr;
                        head_ptr = &((*head_ptr)->nextcmd);
                }
        }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,dqcmd,trc,(char) 1,(uint)cmd, (uint)cmd->cmd_info, (uint)cmd->type, (uint)*head_ptr,(uint)((*head_ptr == NULL) ? 0 : (*head_ptr)->nextcmd));
#endif
#endif
	/*
	 * ASSERT that if this command WAS queued we actually found it
	 */
	ASSERT ((!(cmd->status & SD_QUEUED)) || done);
	if (done && (cmd->type == SD_CTRL_CMD))
                /*
                 * if it was a controller command, decrement count
                 */
                cmd->cp->cmds_qed--;
        if (cmd->type == SD_DASD_CMD)
                /*
                 * See if we should remove this dasd from
                 * the start chain
                 */
                sd_del_chain(dp, (char) FALSE);

	/*
	 * Change Commands Status as UnQueued
	 */
	cmd->status &= ~SD_QUEUED;

	/* reenable interrupts */


        return;
}


/*
 * NAME: sd_event_alloc
 *
 * FUNCTION: Allocates an Event structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no event structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_event        - Event structure
 *
 * RETURNS:
 *           Pointer to allocated event structure - successful completion
 *           NULL                                 - no free structures
 */

struct sd_event *sd_event_alloc(
struct sd_adap_info *ap)
{
        struct  sd_event *event = NULL;
        uchar   tag;
        int     word;


        /*
         * assume interrupts disabled here
         */
        word = 0;
        do  {
                /*
                 * look for a free tag in this word
                 */
                tag = sd_getfree( ap->free_event_list[word]);
                if ( tag < SD_BITS_PER_WORD) {
                        /*
                         * found free tag within this word
                         * set as in use, reset tag to real tag value
                         * depending on which word it was in, and set
                         * flag that we found one.
                         */
                        SD_GETTAG(ap->free_event_list[word],tag);
                        tag += (SD_BITS_PER_WORD * word);
                        /*
                         * set event pointer to the one just allocated
                         */
                         event = (struct sd_event *)&(ap->sd_event[ tag]);
                        break;
                } else  {
                        /*
                         * try next word
                         */
                        word++;
                }
        }  while (word < SD_NUM_EVENT_WORDS);

        return(event);

}  /* end sd_event_alloc */

/*
 * NAME: sd_cmd_alloc_disable
 *
 * FUNCTION: Allocates a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 */
struct sd_cmd *sd_cmd_alloc_disable(
struct sd_adap_info *ap)
{
        uint    old_pri;
        struct  sd_cmd  *cmd = NULL;

	/* disable my irpt level*/

        old_pri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));
	cmd = sd_cmd_alloc(ap);
        unlock_enable(old_pri,&(ap->spin_lock));
        return(cmd);
}
/*
 * NAME: sd_cmd_alloc
 *
 * FUNCTION: Allocates a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If no cmd structure can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           Pointer to allocated cmd   - successful completion
 *           NULL                       - no free structures
 */
struct sd_cmd *sd_cmd_alloc(
struct sd_adap_info *ap)
{

        struct  sd_cmd  *cmd = NULL;
        uchar   tag;
        int     word;


        word = 0;
        do  {
                /*
                 * look for a free tag in this word
                 */
                tag = sd_getfree( ap->free_cmd_list[word]);
                if ( tag < SD_BITS_PER_WORD) {
                        /*
                         * found free tag within this word
                         * set as in use, reset tag to real tag value
                         * depending on which word it was in, and set
                         * flag that we found one.
                         */
                        SD_GETTAG(ap->free_cmd_list[word],tag);
                        tag += (SD_BITS_PER_WORD * word);
                        /*
                         * set the cmd pointer field to this free one
                         * just allocated
                         */
                        cmd = (struct sd_cmd *)&(ap->cmds[ tag]);
                        break;
                } else  {
                        /*
                         * try next word
                         */
                        word++;
                }
        }  while (word < SD_NUM_CMD_WORDS);

#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,cmdalloc, trc,(char)0,(uint)ap, (uint)cmd, (uint)tag, (uint)ap->cmds_out,(uint)0);
#endif
#endif
        return(cmd);

}  /* end sd_cmd_alloc */

/*
 * NAME: sd_free_cmd_disable
 *
 * FUNCTION: Frees a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void sd_free_cmd_disable(
struct sd_cmd *cmd)
{
        uint    old_pri;
	struct sd_adap_info *ap;

        if (cmd == NULL)
                /*
                 * if cmd is NULL, assume that it came from the cmd map and
                 * is already free
                 */
                return;

        /*
         * disable interrupts
         */
        old_pri = disable_lock(cmd->ap->dds.intr_priority,
			       &(cmd->ap->spin_lock));
	ap = cmd->ap;
	sd_free_cmd(cmd);
        /*
         * enable interrupts
         */
        unlock_enable(old_pri,&(ap->spin_lock));

}
/*
 * NAME: sd_free_cmd
 *
 * FUNCTION: Frees a Command structure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If Command pointer to free is NULL, return.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void sd_free_cmd(
struct sd_cmd *cmd)
{
        int      ii;
        struct  sd_adap_info *ap;
        uchar   tag;

        if (cmd == NULL)
                /*
                 * if cmd is NULL, assume that it came from the cmd map and
                 * is already free
                 */
                return;

        /*
         * get adapter pointer
         */
        ap = cmd->ap;
        tag = cmd->tag; /* save tag */


#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,cmdfree,trc,(char) 0,(uint)ap, (uint)cmd, (uint)cmd->cmd_info, (uint)ap->cmds_out,(uint)0);
#endif
#endif
        ASSERT(!(cmd->status & SD_ACTIVE));
        if (cmd->status & SD_LOG_ERROR)
                /*
                 * If this command has an error to be written to the log,
                 * then do it here
                 */
                sd_log_error(cmd);

        if ((cmd->cmd_info == SD_NORMAL_PATH) || (cmd->cmd_info == SD_IOCTL)) {
                /*
                 * this was a normal path command or ioctl path command
                 * so we know it came out of the command pool, so mark this
                 * command as free
                 */
                SD_FREETAG(ap->free_cmd_list[(int)((tag)/SD_BITS_PER_WORD)],
                        tag);
                /*
                 * wakeup anything waiting for command jackets
                 */
                ap->resource_intrpt = 0;
                if (!ap->adap_resources) {
                        ap->adap_resources = TRUE;
                        e_wakeup((int *)&ap->resources);
                }
        }   /* endif */

        bzero(cmd, sizeof(struct sd_cmd)); /* clear entire command struc     */
        cmd->sta_index = (signed char)-1;  /* allocated STA, -1 if none      */
        cmd->ap = ap;                /* restore adapter pointer              */
        cmd->tag = tag;              /* restore tag pointer                  */

	return;

}  /* end sd_free_cmd */

/*
 * NAME: sd_fail_cmd
 *
 * FUNCTION: Processes Failure of a Command.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_ctrl_info    - Controller info structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void sd_fail_cmd(
struct sd_cmd *cmd,
char    flush)
{
        struct sd_adap_info *ap;
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,cmdfail,entry,(char)0,(uint)cmd, (uint)cmd->cmd_info, (uint)cmd->status, (uint)cmd->retry_count,(uint)0);
#endif
#endif
	ap = cmd->ap;
        switch(cmd->cmd_info) {
                case SD_NORMAL_PATH:
                        /*
                         * This command involves buf's, so set the error field
                         * for the commands, clear the last rba valid field,
                         * set retries to limit, and call process buf error
                         * Also stop command timer in case this was a retry
                         */
			if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
			    w_stop(&cmd->dp->cmd_timer.watch);
                        /*
                         * make sure retry count is exceeded, since we want
                         * to FAIL this command, in case we were called from
                         * a flush queue routine
                         */
                        cmd->retries = 0;
                        sd_fail_buf_cmd(cmd, flush);
                        break;
                case SD_IOCTL:
                        /*
                         * This was an ioctl, and should only get here as a
                         * result of flushing queues, so set validity fields
                         * and wakeup the ioctl process
                         */
                        if (cmd->status & SD_TIMEDOUT) {
                                /*
                                 * if ioctl routine already timed out on this
                                 * command, then just free the command
                                 */
                                sd_free_cmd(cmd);
                        } else {
                                cmd->status_validity = SD_VALID_ADAP_STATUS;
                                cmd->adapter_status = SD_RESET_PURGE_TAG;
				cmd->driver_status = SD_DD_PURGED_TAG;
                                switch(cmd->type) {
                                        case SD_ADAP_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->ap->ioctl_timer.watch);
                                           cmd->ap->ioctl_intrpt = 0;
                                           e_wakeup((int *)
						&cmd->ap->ioctl_event);
                                           break;
                                        case SD_CTRL_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->cp->ioctl_timer.watch);
                                           cmd->cp->ioctl_intrpt = 0;
                                           e_wakeup((int *)
						&cmd->cp->ioctl_event);
                                           break;
                                        case SD_DASD_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->dp->ioctl_timer.watch);
                                           cmd->dp->ioctl_intrpt = 0;
                                           e_wakeup((int *)
						&cmd->dp->ioctl_event);
                                        break;
                                }
                        }
                        break;
                case SD_REASSIGN:
                        /*
                         * This was an internal reassign block that failed,
                         * free the reassign command, set max retries for the
                         * corresponding write so it will be failed by
                         * start_cmd
                         */
			if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
			    w_stop(&cmd->dp->cmd_timer.watch);
                        cmd->dp->status &= ~SD_REASSIGN_PENDING;
                        cmd->dp->reassign_write->retries = -1;
                        cmd->dp->reassign_write->b_error = EIO;
                        sd_free_cmd(cmd);
                        break;
                case SD_REQSNS:
                        /*
                         * This was an internal request sense command that
                         * we are failing, free the request sense , reset
                         * the DASD, and then retry the head of the error queue
                         */
                        if (cmd->type == SD_DASD_CMD) {
                                /*
                                 * this was a request sense to a
                                 * dasd
                                 */
			        if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
				    w_stop(&cmd->dp->cmd_timer.watch);
                                cmd->dp->status &= ~SD_REQ_SNS_PENDING;
				if (!flush)
				   /*
				    * if this is not from a flush of adapter,
				    * controller or dasd, then reset the dasd,
				    * else, we don't want to try further ERP.
				    */
                                   sd_reset_quiesce((struct sd_adap_info *)
					cmd->dp, (uchar)SD_RESET_OP,
					(uchar)SD_DASD_CMD);
                        } else {
                                /*
                                 * this was a request sense to a
                                 * controller, stop timer, controller type 1'ed
                                 * anyway, so no procedure necessary
                                 */
				cmd->cp->status &= ~SD_REQ_SNS_PENDING;
			        if ((struct sd_cmd *)cmd->cp->cmd_timer.pointer == cmd)
				    w_stop(&cmd->cp->cmd_timer.watch);
                        }
                        sd_free_cmd(cmd);  /* free the request sense command*/
                        break;
                case SD_RST_QSC:
                        /*
                         * This was an internal reset/quiesce that we are
                         * failing, stop any possible timer and free command
                         * and excalate recovery to the next higher recovery
                         */
                        switch(cmd->type) {
                                case SD_ADAP_CMD:
                                        ap = cmd->ap;
                                        ap->status &= ~SD_QUIESCE_PENDING;
                                        /*
                                         * stop the timer
                                         */
					if ((struct sd_cmd *)ap->cmd_timer.pointer == cmd)
					    w_stop(&ap->cmd_timer.watch);
                                        if (!(cmd->status & SD_TIMEDOUT))
                                                /*
                                                 * If this is not result of a
                                                 * timeout (cmd still active),
                                                 * free the command, else let
                                                 * escalation handle it.
                                                 */
                                                sd_free_cmd(cmd);
                                        /*
                                         * must have been a quiesce adapter,
                                         * so escalate and try a full reset
                                         */
					if (!flush) {
					   /*
					    * if this is not from a flush of 
					    * an adapter, then reset the 
					    * adapter, else, we don't want to 
					    * try further ERP.
					    */
					    ap->reset_count = 0;
					    sd_reset_quiesce(ap, 
					     	(uchar)SD_RESET_OP, 
					        (uchar)SD_ADAP_CMD);
				       }
					
                                        break;
                                case SD_CTRL_CMD:
                                        cp = cmd->cp;
                                        /*
                                         * stop the timer
                                         */
					if ((struct sd_cmd *)cp->cmd_timer.pointer == cmd)
					    w_stop(&cp->cmd_timer.watch);
                                        if (cmd->mbox_copy.mb6.reset_type ==
                                                SD_RESET_CTRL_MB) {
                                                /*
                                                 * if this was a controller
                                                 * reset, then escalate and
                                                 * try an adapter quiesce
                                                 */
                                                cp->status &= ~SD_RESET_PENDING;
                                                if (!(cmd->status & 
						   SD_TIMEDOUT))
                                                   /*
                                                    * If this is not result
                                                    * of a timeout (cmd still
                                                    * active), free the
                                                    * command, else let
                                                    * escalation handle it.
                                                    */
                                                   sd_free_cmd(cmd);
						if (!flush)
					   	 /*
					    	  * if this is not from a flush
					          * of an adapter or controller
					          * then quiesce the adapter, 
						  * else, we don't want to 
					          * try further ERP.
					          */
                                                  sd_reset_quiesce(cp->ap,
                                                        (uchar)SD_QUIESCE_OP,
                                                        (uchar)SD_ADAP_CMD);
                                        } else {
                                                /*
                                                 * else must have been a
                                                 * controller quiesce, so
                                                 * escalate to controller reset
                                                 */
                                                cp->status &=
                                                        ~SD_QUIESCE_PENDING;
                                                if (!(cmd->status &
                                                        SD_TIMEDOUT))
                                                   /*
                                                    * If this is not result
                                                    * of a timeout (cmd still
                                                    * active), free the
                                                    * command, else let
                                                    * escalation handle it.
                                                    */
                                                   sd_free_cmd(cmd);
						if (!flush)
					   	 /*
					    	  * if this is not from a flush
					          * of an adapter or controller
					          * then reset the controller, 
						  * else, we don't want to 
					          * try further ERP.
					          */
                                                sd_reset_quiesce(
                                                   (struct sd_adap_info *)cp,
                                                   (uchar)SD_RESET_OP,
                                                   (uchar)SD_CTRL_CMD);
                                        }
                                        break;
                                case SD_DASD_CMD:
                                        dp = cmd->dp;
                                        /*
                                         * stop the timer
                                         */
					if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
					    w_stop(&dp->cmd_timer.watch);
                                        if ((cmd->mbox_copy.mb6.reset_type ==
                                             SD_RESET_DASD_MB) ||
                                            (cmd->mbox_copy.mb8.fields.
                                              scsi_cmd.scsi_op_code ==
                                              SCSI_SEND_DIAGNOSTIC)) {
                                           /*
                                            * if this was a dasd
                                            * reset, then escalate and
                                            * try controller quiesce
                                            */
                                           dp->status &= ~SD_RESET_PENDING;
                                           if (!(cmd->status & SD_TIMEDOUT))
                                               /*
                                                * If this is not result
                                                * of a timeout (cmd still
                                                * active), free the
                                                * command, else let
                                                * escalation handle it.
                                                */
                                               sd_free_cmd(cmd);
                                            if (dp->forced_open) {
                                               /*
                                                * if this was from a
                                                * forced open that failed,
                                                */
                                               dp->dasd_result = EIO;
                                               dp->forced_open = FALSE;
                                               e_wakeup((int *)
							&(dp->dasd_event));
                                            } else
					      if (!flush)
					   	/*
					    	 * if this is not from a flush
					         * of an adapter, controller
					         * or dasd, then quiesce the 
						 * controller, else, we don't 
						 * want to try further ERP.
					         */
                                                sd_reset_quiesce(
                                                 (struct sd_adap_info *)dp->cp,
                                                 (uchar)SD_QUIESCE_OP,
                                                 (uchar)SD_CTRL_CMD);
                                     } else {
                                            /*
                                             * else must have been a
                                             * dasd quiesce, so escalate
                                             * to dasd reset
                                             */
                                            dp->status &= ~SD_QUIESCE_PENDING;
                                            if (!(cmd->status & SD_TIMEDOUT))
                                                   /*
                                                    * If this is not result
                                                    * of a timeout (cmd still
                                                    * active), free the
                                                    * command, else let
                                                    * escalation handle it.
                                                    */
                                                   sd_free_cmd(cmd);
					   if (!flush)
					     /*
					      * if this is not from a flush
					      * of an adapter, controller
					      * or dasd, then reset the 
					      * dasd, else, we don't 
					      * want to try further ERP.
					      */
                                             sd_reset_quiesce(
                                               (struct sd_adap_info *)dp,
                                               (uchar)SD_RESET_OP,
                                               (uchar)SD_DASD_CMD);
                                    }
                                    break;
                        }
                        break;
                case SD_START_UNIT:
                case SD_TEST_UNIT_READY:
                case SD_RESERVE:
                case SD_MODE_SENSE:
                case SD_MODE_SELECT:
                case SD_READ_CAPACITY:
                case SD_STOP_UNIT:
                case SD_RELEASE:
                case SD_INQUIRY:
	        case SD_FENCE:
	        case SD_FENCE_POS_CHECK:
			
                        /*
                         * this was part of our reset cycle that failed,
                         * or from a release, stop unit, or inquiry
                         * so wakeup the dasd event, with bad results.
                         * if this was result of unit attention, then
                         * a total reset may be the only way out
                         */
                        dp = cmd->dp;
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
			if (cmd->b_error)
       	                	dp->dasd_result = cmd->b_error;
			else 
       	                	dp->dasd_result = EIO;
			dp->restart_count = 0;
                        if (dp->seq_not_done) {
                                /*
                                 * if this was called from open sequence
                                 * just wake up the event
                                 */
                                dp->seq_not_done = FALSE;
                                e_wakeup((int *)&dp->dasd_event);
                                sd_free_cmd(cmd);
                        } else {
                                /*
                                 * else this came from device verification
                                 * verification failed, we've tried a reset,
                                 * nothing more to do, so give up
                                 */
                                sd_free_cmd(cmd);
                                if (!flush)
                                        /*
                                         * if this command was not failed from
                                         * fail dasd, then call fail dasd
                                         */
                                        sd_fail_dasd(dp);
                        }
                        break;
                case SD_QRYDEV:
                        /*
                         * Make sure the query timer is stopped, and free
                         * this command
                         */
                        w_stop(&(cmd->dp->query_timer.watch));
                        sd_free_cmd(cmd);
                        break;
                case SD_SPECIAL:
                        /*
                         * stop the appropriate timer, set bad result
                         * flag, and wakeup the event
                         */
                        if (cmd->type == SD_ADAP_CMD) {
			        if ((struct sd_cmd *)cmd->ap->cmd_timer.pointer == cmd)
				    w_stop(&cmd->ap->cmd_timer.watch);
                                cmd->ap->adap_result = -1;
                                if (cmd->ap->reset_result == SD_RESET_A) {
                                        /*
                                         * if part of the reset adap
                                         * error recovery, try again
                                         */
				    	if (!flush)
					     /*
					      * only if this is not from a 
					      * flush of an adapter
					      */
                                             sd_reset_quiesce(cmd->ap,
                                                (uchar)SD_RESET_OP,
                                                (uchar)SD_ADAP_CMD);
                                }
                                e_wakeup((int *)&cmd->ap->adap_event);
                        }
                        sd_free_cmd(cmd);
                        break;
		case SD_REFRESH: 
		case SD_LOCK: 
		case SD_UNLOCK: 
		case SD_TEST: 
			if (cmd->b_error == 0)
			    cmd->b_error = EIO;
			sd_process_conc_cmd(cmd);
			break;
                default:
                        break;
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,cmdfail, exit,(char)0,(uint)cmd, (uint)0, (uint)0, (uint)0,(uint)0);
#endif
#endif
}


/*
 * NAME: sd_MB_alloc
 *
 * FUNCTION: Allocates a Mailbox
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt levels,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no mailbox can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           0  - successful completion
 *           -1 - no free mailboxes
 */

int sd_MB_alloc(
struct  sd_cmd *cmd)
{
        uchar   tag;
        uint    tmpword;
        struct sd_adap_info *ap;
        char    already_done_this = 0;

        /*
         * assume interrupts are disabled here
         *
         * get adapter info pointer
         */
        ap = cmd->ap;
        /*
         * promote next mailbox tag to current mailbox pointer
         * and generate pointer to it
         */
        ap->curr_tag = ap->next_tag;
        ap->curr_mb = (struct sd_mbox *)&(ap->MB[ap->curr_tag]);

        /*
         * Make copy of first word of free tag list
         */
        tmpword = ap->mb_alloc_list[ap->mb_word];
        do  {
        	/*
	         * Get first free tag 
	         */
                tag = sd_getfree( tmpword);
                if ( tag < SD_BITS_PER_WORD) {
                        /*
                         * found free tag within this word
                         */
                        tag += (SD_BITS_PER_WORD * ap->mb_word);
                        /*
                         * insure that tag differs from previous tag by
                         * at least 2 so that they can not be in same
                         * IOCC cache line.  (i.e. 64 byte cache, 32 byte MB)
                         */
                        if (( tag & 0xFE) == ( ap->curr_tag & 0xFE)) {
                                /*
                                 * if this mailbox is in the same cache line
                                 * as the previous one, or is the last tag
                                 * written to LTR, then mark this tag used
                                 * and get the next available tag
                                 */
                                SD_GETTAG(tmpword,tag);
                        } else {
                                /*
                                 * else we can use this tag, so make it the
                                 * next mailbox, and put the pointer in ap
                                 */
                                /*
                                 * set the mailbox pointer within the command
                                 * structure to the current one as well as
                                 * the current mailbox dma address and storing
                                 * the cmd pointer in the cmd map
                                 */
                                cmd->mb = (struct sd_mbox *)ap->curr_mb;
                                cmd->mb_dma_addr = (struct sd_mbox *)
                                        SD_MB_ADDR((uint)ap->base_MB_dma_addr,
                                        ap->curr_mb->tag);
                                ap->cmd_map[ap->curr_mb->tag] = cmd;
                                /*
                                 * set status of command to ACTIVE, Notice:
                                 * this is the only place a commands achieves
                                 * this status.
                                 */
                                cmd->status |= SD_ACTIVE;
                                /*
                                 * Increment the appropriate outstanding
                                 * command counts
                                 */
                                ap->cmds_out++;
                                if (cmd->type == SD_DASD_CMD)
                                        cmd->dp->cmds_out++;
                                else if (cmd->type == SD_CTRL_CMD)
                                        cmd->cp->cmds_out++;
                                /*
                                 * set the next mailbox pointer field of the
                                 * current mailbox to the address of this
                                 * new one we just allocated
                                 */
                                ap->curr_mb->nextmb = (struct sd_mbox *)
                                        SD_MB_ADDR((uint)ap->base_MB_dma_addr,
                                        tag);
                                /*
                                 * update next tag, and set curr tag as in
                                 * use in the real free list, and return good
                                 */
                                ap->next_tag = tag;
                                SD_GETTAG(ap->mb_alloc_list[(int)
                                        (ap->curr_tag/SD_BITS_PER_WORD)],
                                        ap->curr_tag);
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                                sd_trc(ap,mballoc, trc,(char)0,(uint)cmd, (uint)cmd->mb, (uint)cmd->mb->tag, (uint)tag,(uint)0);
#endif
#endif
                                return(0);
                        }
                } else  {
                        /*
                         * try next word
                         */
                        if (++ap->mb_word < SD_NUM_MB_WORDS)
                                /*
                                 * more words to check
                                 */
                                tmpword = ap->mb_alloc_list[ ap->mb_word];
                        else {
				if (already_done_this++) {
				        /*
				         * if we get here, it can't be good
				         */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
				        sd_trc(ap,mballoc, trc,(char)1,(uint)cmd, (uint)-1, (uint)0, (uint)0,(uint)0);
#endif
#endif
					ap->mb_word = 0;
				        return(-1);
				}
                                /*
                                 * as long as we haven't already
                                 * done this,since we've  looked through
                                 * all the words,
                                 * OR back the freed mailboxes and try
                                 */
				do {
                                	ap->mb_alloc_list[--ap->mb_word] |= 
						ap->mb_free_list[ap->mb_word];
                                  	ap->mb_free_list[ap->mb_word] = 0;
                                } while (ap->mb_word);
                                tmpword = ap->mb_alloc_list[ 0];
                        }
                }
        }  while (TRUE);
}  /* end sd_MB_alloc */

/*
 * NAME: sd_free_MB
 *
 * FUNCTION: Frees a Mailbox
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_free_MB(
struct sd_cmd    *cmd,
char status)
{
        struct  sd_adap_info *ap;

        /*
         * assume interrupts are disabled here
         */
        ap = cmd->ap;
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,mbfree, trc,(char)0,(uint)ap, (uint)cmd, (uint)cmd->mb, (uint)cmd->mb->tag,(uint)0);
#endif
#endif
        if (status == SD_NOT_USED) {
                /*
                 * if this mailbox was allocated, but not ever used,
                 * mark it as free, and then reset next_tag to curr_tag
                 * so he'll be the first one used next time
                 */
                SD_FREETAG(ap->mb_alloc_list[
                        (int)((ap->curr_tag)/SD_BITS_PER_WORD)], ap->curr_tag);
                ap->next_tag = ap->curr_tag;
        } else
                /*
                 * Mark this Mailbox as free
                 */
                SD_FREETAG(ap->mb_free_list[
                        (int)((cmd->mb->tag)/SD_BITS_PER_WORD)], cmd->mb->tag);
        /*
         * Make the mailbox pointer odd to prevent adapter runaway
         */
        cmd->mb->nextmb = (struct sd_mbox *)1;

        /*
         * Clear this mailbox's command map entry
         */
        ap->cmd_map[cmd->mb->tag] = (struct sd_cmd *)NULL;
        /*
         * set status of command to DEACTIVE, Notice:
         * this is the only place a commands achieves
         * this status.
         */
        cmd->status &= ~SD_ACTIVE;
        /*
         * Decrement the appropriate outstanding
         * command counts
         */
        ap->cmds_out--;
        if (cmd->type == SD_DASD_CMD) {
                cmd->dp->cmds_out--;
                if (cmd->dp->cmds_out == 0)
                        /*
                         * If this DASD has no more commands outstanding
                         * clear BUSY status
                         */
                        cmd->dp->dkstat.dk_status &= ~IOST_DK_BUSY;
        } else if (cmd->type == SD_CTRL_CMD)
                cmd->cp->cmds_out--;

        ap->adap_resources = TRUE;

}  /* end sd_free_MB */




/*
 * NAME: sd_TCW_alloc
 *
 * FUNCTION: Allocates TCW's for a command
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no TCWs can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           TRUE       - successful completion
 *           FALSE      - no free TCWs
 */

int sd_TCW_alloc(
struct sd_cmd *cmd)
{
        int             badr,
                        need,
                        mask,
                        new_need,
                        slide,
                        last_mask,
                        i;
        char            found = FALSE,
                        n_word_1,
                        n_words_touched,
                        shift,
                        word,
                        last_word,
                        curr_word,
                        num_free,
                        new_num_free,
                        start_bit,
			pass2 = FALSE;
        struct  sd_adap_info *ap;

        /*
         * Assume interrupts disabled
         */
        if (cmd->b_length  == 0) {
                /*
                 * if no data transfer involved
                 */
                return(TRUE);
        }
        ap = cmd->ap;

        /*
         * get the starting address of the request
         */
        badr = (int)cmd->b_addr;

        /*
         * Calculate number of TCW's needed with following equation:
         *
         *  ((offset_into_page + length_of_xfer - 1) / (pagesize)) + 1
         *
         *  Example of Ending Page Aligned Transfer:
         *
         *  P              P               P              P
         *  |--------------|---------------|--------------|  = Page boundaries
         *       ^-------------------------^                 = data buffer
         *  ^----^                                           = offset into page
         *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
         *
         *     offset + length - 1 = 0x1FFF;  and (0x1FFF / 0x1000) + 1 = 2
         *
         *  Example of Beginning Page Aligned Transfer:
         *
         *  P              P               P              P
         *  |--------------|---------------|--------------|  = Page boundaries
         *  ^-------------------------^                      = data buffer
         *  ^                                                = offset into page
         *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
         *
         *     0 + length - 1 = 0x1???;  and (0x1??? / 0x1000) + 1 = 2
         *
         *  Example of Page Aligned Transfer:
         *
         *  P              P               P              P
         *  |--------------|---------------|--------------|  = Page boundaries
         *  ^------------------------------^                 = data buffer
         *  ^                                                = offset into page
         *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|--------------|  = TCW's needed
         *
         *     0 + 0x2000 - 1 = 0x1FFF;  and (0x1FFF / 0x1000) + 1 = 2
         *
         *  Example of Un-Aligned Transfer:
         *
         *  P              P               P              P
         *  |--------------|---------------|--------------|  = Page boundaries
         *        ^------------------------------^           = data buffer
         *  ^-----^                                          = offset into page
         *  |XXXXXXXXXXXXXX|XXXXXXXXXXXXXXX|XXXXXXXXXXXXXX|  = TCW's needed
         *
         *     offset + length - 1 = 0x2???;  and (0x2??? / 0x1000) + 1 = 3
         */
        need = (((badr & (SD_TCWSIZE-1)) + cmd->b_length - 1) / SD_TCWSIZE)+1;

#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,tcwalloc, trc,(char)0,(uint)badr, (uint)need, (uint)cmd->b_length, (uint)ap->tcw_free_list[0],(uint)ap->tcw_free_list[1]);
        sd_trc(ap,tcwalloc, trc,(char)1,(uint)ap->tcw_free_list[2], (uint)ap->tcw_free_list[3], (uint)ap->tcw_free_list[4], (uint)ap->tcw_free_list[5],(uint)ap->tcw_free_list[6]);
        sd_trc(ap,tcwalloc, trc,(char)2,(uint)ap->tcw_free_list[7], (uint)ap->tcw_free_list[8], (uint)0,(uint)0,(uint)0);
#endif
#endif
        /*
         * initialize shift to 0
         */
        shift = ap->shift;
        word = ap->tcw_word;
        n_words_touched = 1;    /* number of words touched */
        do {
                /*
                 * set slide to right word
                 */
                slide = ap->tcw_free_list[word] << shift;
                start_bit = sd_getfree(slide);  /* find first free resource */
                if (start_bit == SD_BITS_PER_WORD) {
                        /*
                         * if this word is empty, clear start bit, increment
                         * word and go to top of loop
                         */
                        if (++word >= ap->num_tcw_words) {
				if (pass2)
                                	/*
	                                 * out of words, break out
	                                 */
					break;
				else {
					word = ap->tcw_word = 0;
					shift = ap->shift = 0;
					pass2 = TRUE;
				}
			}
                        shift = 0;
                        continue;
                }
                /*
                 * shift out bits before first free
                 * while complementing -> now 1 = used, 0 = free
                 */
                slide = ~(slide << start_bit);
                shift += start_bit;
                /*
                 * count the number free in this word
                 */
                num_free = sd_getfree(slide);
                if (num_free >= need) {
                        /*
                         * We found as many as we need
                         */
                         found = 1;
                } else if ((num_free + shift) == SD_BITS_PER_WORD){
                        /*
                         * else see if we need to cross a window
                         */
                        /*
                         * compute new need
                         */
                        new_need = need - num_free;
                        new_num_free = 0;
                        curr_word = word;
                        do {
                                if (++curr_word >= ap->num_tcw_words)
                                        /*
                                         * Out of words
                                         */
                                        break;

                                n_words_touched++; /* inc # words touched */
                                /*
                                 * set slide to complement of next word
                                 */
                                slide = ~ap->tcw_free_list[curr_word];
                                /*
                                 * compute how many free in next word
                                 */
                                new_num_free = sd_getfree(slide);
                                if (new_num_free >= new_need) {
                                        /*
                                         * if found what we need, set found
                                         * which will cause us to exit, and
                                         * word and shift will be set
                                         * correctly
                                         */
                                        found = 1;
                                } else if (new_num_free == SD_BITS_PER_WORD) {
                                        /*
                                         * else, didn't find what we need, but
                                         * found all we could in this word,
                                         * update number needed and try next
                                         * word.
                                         */
                                        new_need -= SD_BITS_PER_WORD;
                                        new_num_free = 0;
                                } else
                                        /*
                                         * didn't find them, update word to
                                         * where we are and continue
                                         */
                                        break;
                        } while (new_need > new_num_free);
                        if (!found) {
                                /*
                                 * update word to where we searched to
                                 */
                                word = curr_word;
                                if (word >= ap->num_tcw_words) {
					if (pass2)
	                                	/*
		                                 * out of words, break out
		                                 */
						break;
					else {
						word = ap->tcw_word = 0;
						shift = ap->shift = 0;
						pass2 = TRUE;
					}
				}
                                shift = 0;
                                /*
                                 * reset number of words touched to 1
                                 */
                                n_words_touched = 1;
                                continue;
                        }
                } else {
                        /*
                         * Didn't find the amount we need, so cleanup,
                         * update shift and try again
                         */
                         shift += num_free;
                }
        } while (!found);
        if (found) {
                /*
                 * Mark as in use
                 */
                if (n_words_touched == 1) {
                        /*
                         * only touched 1, all in word 1
                         */
                        n_word_1 = need;
			/*	
			 * Update shift for next time around
			 */
			ap->shift = shift + need;
			if (ap->shift == SD_BITS_PER_WORD)
				ap->shift = 0;
                } else
                        /*
                         * compute # in word 1 (i.e. #bits in word -
                         * # we shifted out
                         */
                        n_word_1 = (SD_BITS_PER_WORD - shift);
                /*
                 * Create bit map to clear appropriate bits, setting those
                 * TCW's as in use
                 */
                mask = ((0xFFFFFFFF <<
                        (SD_BITS_PER_WORD - n_word_1)) >> shift);
                /*
                 * clear appropriate bits
                 */
		ASSERT(((ap->tcw_free_list[word] & mask) == mask));
                ap->tcw_free_list[word] &= ~mask;
                /*
                 * save mask info with cmd
                 */
                cmd->tcw_words = n_words_touched;
                cmd->tcw_mask_first = mask;
                cmd->tcw_first_word = word;
                /*
                 * set starting TCW
                 */
                cmd->tcws_start = shift + (word * SD_BITS_PER_WORD);
                /*
                 * Now Handle spanned words
                 */
                last_word = word + n_words_touched - 1;
		/*
		 * Update starting tcw word for next time around
		 */
		ap->tcw_word = last_word;
                word++;
                for ( ; word <= last_word ; word++) {
                        if (word == last_word){
                                /*
                                 * if this is the last word
                                 * then build a mask for it, i.e. total number
                                 * needed - how many we found in the first word,
                                 * mod word size
                                 * NOTICE: this mask set up so that an OR will
                                 * mark the bits free, to clear we must NOT the
                                 * mask and AND.
                                 */
                                last_mask = (0xFFFFFFFF >>
                                   ((need - n_word_1) % SD_BITS_PER_WORD));


				if (last_mask != 0xFFFFFFFF) {
					/*
					 * NOT the mask to create bit mask
					 * of the TCW's to be used in this
					 * (last) word.
					 */
					cmd->tcw_mask_last = ~last_mask;
					/*
					 * Update shift for the next time 
					 * so we'll immediately look at the 
					 * TCW's following these we've just 
					 * allocated
					 */
					ap->shift = sd_getfree(last_mask);
				} else {
					/*
					 * if the mask for the last word 
					 * turns out to be all 32, then this
					 * is a special case where we don't
					 * need to NOT the mask since we didn't
					 * shift 
					 */
					cmd->tcw_mask_last = last_mask;
					/*
					 * update shift to 0 for next time
					 */
					ap->shift = 0;
				}

                                /*
                                 * clear appropriate bits
                                 */
				ASSERT(((ap->tcw_free_list[word] & 
				   cmd->tcw_mask_last) == cmd->tcw_mask_last));
                                ap->tcw_free_list[word] &= ~cmd->tcw_mask_last;
                        } else {
                                /*
                                 * else mark them all
                                 */
				ASSERT(ap->tcw_free_list[word] == 
					(uint)0xFFFFFFFF);
                                ap->tcw_free_list[word] = 0;
			}
                }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,tcwalloc, trc,(char)3,(uint)cmd->tcw_words, (uint)cmd->tcw_first_word, (uint)cmd->tcw_mask_first, (uint)cmd->tcw_mask_last,(uint)cmd->tcws_start);
        sd_trc(ap,tcwalloc, trc,(char)4,(uint)ap->tcw_free_list[0], (uint)ap->tcw_free_list[1], (uint)ap->tcw_free_list[2], (uint)ap->tcw_free_list[3],(uint)ap->tcw_free_list[4]);
        sd_trc(ap,tcwalloc, trc,(char)5,(uint)ap->tcw_free_list[5], (uint)ap->tcw_free_list[6], (uint)ap->tcw_free_list[7],(uint)ap->tcw_free_list[8],(uint)0);
#endif
#endif
                return(TRUE);
        } else
                /*
                 * TCW's not available
                 */
                return(FALSE);

}


/*
 * NAME: sd_TCW_realloc
 *
 * FUNCTION: Re-Allocates TCW's for a coalesced command
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If unable to reallocate TCW's for the coalesced cmd
 *      then the previously allocated TCW's are restored, and recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           TRUE       - successful completion
 *           FALSE      - couldn't reallocate TCW's
 */
int sd_TCW_realloc(
struct sd_cmd *cmd)
{
        struct sd_adap_info *ap;
        char    word,last_word,oword,oshift;

        ap = cmd->ap;
	/*
	 * save original tcw_word and original shift
	 */
	oword = ap->tcw_word;
	oshift = ap->shift;
        /*
         * Free previously allocated TCW's
         */
        word = cmd->tcw_first_word;
	ap->tcw_word = word;
	ap->shift = sd_getfree(cmd->tcw_mask_first);
        last_word = cmd->tcw_first_word + cmd->tcw_words - 1;
	ASSERT(((~ap->tcw_free_list[word] & cmd->tcw_mask_first) == 
		cmd->tcw_mask_first));

        ap->tcw_free_list[word++] |= cmd->tcw_mask_first;
        for (; word <= last_word; word++) {
                if (word == last_word) {
                        /*
                         * if this is the last word
                         */
			ASSERT(((~ap->tcw_free_list[word] & 
				cmd->tcw_mask_last) == cmd->tcw_mask_last));
                        ap->tcw_free_list[word] |= cmd->tcw_mask_last;
                } else {
			ASSERT((ap->tcw_free_list[word] == 0));
                        ap->tcw_free_list[word] = 0xFFFFFFFF;
		}
        }

        if (sd_TCW_alloc(cmd) == FALSE) {
                /*
                 * couldn't allocate new amount, so reset old tcw's and
                 * tell caller TCWs not avail
                 */
                word = cmd->tcw_first_word;
                ap->tcw_free_list[word++] &= ~cmd->tcw_mask_first;
                for (; word <= last_word; word++) {
                        if (word == last_word)
                                /*
                                 * if this is the last word
                                 */
                                ap->tcw_free_list[word] &= ~cmd->tcw_mask_last;
                        else
                                ap->tcw_free_list[word] = 0;
                }
		/*
		 * restore tcw_word and shift to original values
		 */
		ap->tcw_word = oword;
		ap->shift = oshift;
                return(FALSE);
        } else
                return(TRUE);
}  /* end sd_TCW_realloc */



/*
 * NAME: sd_TCW_dealloc
 *
 * FUNCTION: Frees TCWs previously allocated for a command
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no TCWs were allocated, return.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command Structure
 *
 * RETURNS:     Void.
 */

void sd_TCW_dealloc(
struct sd_cmd *cmd)
{
        struct  sd_adap_info *ap;
        char            word,last_word;

        /*
         * assume interrupts are disabled here
         */
        if (cmd->tcw_words == 0) {
                /*
                 * didn't have any allocated
                 */
                return;
        }
        /*
         * get adapter pointer
         */
        ap = cmd->ap;
        /*
         * release TCWs, mark TCWs as available
         */
        word = cmd->tcw_first_word;
        last_word = cmd->tcw_first_word + cmd->tcw_words - 1;
	/*
	 * Assert if we are freeing TCW's that may already be free
	 */
	ASSERT(((~ap->tcw_free_list[word] & cmd->tcw_mask_first) == 
		cmd->tcw_mask_first));
        ap->tcw_free_list[word++] |= cmd->tcw_mask_first;
        for (; word <= last_word; word++) {
                if (word == last_word) {
                        /*
                         * if this is the last word
                         */
			ASSERT(((~ap->tcw_free_list[word] & 
				cmd->tcw_mask_last) == cmd->tcw_mask_last));
                        ap->tcw_free_list[word] |= cmd->tcw_mask_last;
                } else {
			ASSERT((ap->tcw_free_list[word] == 0));
                        ap->tcw_free_list[word] = 0xFFFFFFFF;
		}
        }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,tcwfree, trc,(char)0,(uint)cmd->tcw_words, (uint)cmd->tcw_first_word, (uint)cmd->tcw_mask_first, (uint)cmd->tcw_mask_last,(uint)cmd->tcws_start);
        sd_trc(ap,tcwfree, trc,(char)2,(uint)ap->tcw_free_list[0], (uint)ap->tcw_free_list[1], (uint)ap->tcw_free_list[2], (uint)ap->tcw_free_list[3],(uint)ap->tcw_free_list[4]);
        sd_trc(ap,tcwfree, trc,(char)3,(uint)ap->tcw_free_list[5], (uint)ap->tcw_free_list[6], (uint)ap->tcw_free_list[7],(uint)ap->tcw_free_list[8],(uint)0);
#endif
#endif
        cmd->tcws_start = 0;            /* clear starting tcw */
        ap->adap_resources = TRUE;
}  /* end sd_TCW_dealloc */



/*
 * NAME: sd_STA_alloc
 *
 * FUNCTION: Allocates a Small Transfer Area for a command.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no STA can be allocated, then recovery
 *      is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:
 *           TRUE       - successful completion
 *           FALSE      - no free STAs
 */

int sd_STA_alloc(
struct sd_cmd  *cmd)
{
        struct sd_adap_info  *ap;
        char             i;

        if (cmd->b_length == 0) {
                /*
                 * no data transfer
                 */
                return(TRUE);
        }
        /*
         * assume interrupts are disabled here
         */
        /*
         * get adapter pointer for this command
         */
        ap = cmd->ap;
        for (i = 0; i < SD_NUM_STA; i++) {
                /*
                 * search for a free STA
                 */
                if (ap->STA[i].in_use == FALSE) {
                        ap->STA[i].in_use = TRUE;   /* mark as in use */
                        cmd->sta_index = (signed char)i; /* save its index */
                        return(TRUE);
                }
        } /* endfor */

        /*
         * if we get here, we didn't get any
         */
        return(FALSE);
}  /* end sd_STA_alloc */


/*
 * NAME: sd_STA_dealloc
 *
 * FUNCTION: Frees a Small Transfer Area
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If no STA was previously allocated, then return.
 *
 * (DATA STRUCTURES:)   sd_cmd  - Command structure
 *
 * RETURNS:     Void.
 */

void sd_STA_dealloc(
struct sd_cmd *cmd)
{

        /*
         * assume interrupts are disabled here
         */
        if (cmd->sta_index == (signed char)-1) {
                /*
                 * no STA to free
                 */
                return;
        }
        /*
         * release STAs
         */
        cmd->ap->STA[cmd->sta_index].in_use = FALSE; /* mark STA available */
        cmd->sta_index = (signed char)-1;       /* clear cmd sta index */

}  /* end sd_STA_dealloc */


/*
 * NAME: sd_set_adap_parms_disable
 *
 * FUNCTION: Builds command to set up adapter parameters
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If error occurs, then the proper errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:
 *           0          - successful completion
 *           EIO        - PIO error from sd_start
 */

int sd_set_adap_parms_disable(
struct sd_adap_info *ap,
char    from_open)
{
        uint    old_pri;
	int	ret_code;

        old_pri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));
	ret_code = sd_set_adap_parms(ap,from_open);
	
	unlock_enable(old_pri,&(ap->spin_lock));	
	return (ret_code);

}

/*
 * NAME: sd_set_adap_parms
 *
 * FUNCTION: Builds command to set up adapter parameters
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If error occurs, then the proper errno is returned,
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:
 *           0          - successful completion
 *           EIO        - PIO error from sd_start
 */

int sd_set_adap_parms(
struct sd_adap_info *ap,
char    from_open)
{
        int     rc;
        struct  sd_cmd  *cmd;
        struct  sd_mbox *m;

        if (ap->special.status != SD_FREE) {
                if (ap->special.status & SD_ACTIVE) {
                        /*
                         * Shouldn't happen, quiesce adapter
                         */
                        sd_reset_quiesce(ap, (uchar)SD_QUIESCE_OP,
                                        (uchar)SD_ADAP_CMD);

                        return;
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        sd_d_q_cmd(&ap->special);
                }
        }
        /*
         * get the special command structure
         */
        cmd = &(ap->special);
        cmd->ap = ap;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,setparms, trc,(char)0,(uint)cmd, (uint)0 ,(uint)0, (uint)0,(uint)0);
#endif
#endif

        /*
         * build the copy of mailbox for set adapter parms command
         */
        m = (struct sd_mbox *)&cmd->mbox_copy;
        m->op_code = SD_SETPARMS_OP;            /* set op code */
        m->mb6.length = 4;                      /* set length to 4 parms */
        m->mb7.nul_address = 0;                 /* address is 0 */
        m->mb8.ad_parm.parm[0] = 0x01000001;    /* report ctlr rdy to host   */
        m->mb8.ad_parm.parm[1] = 0x02000001;    /* report DASD ready to host */
        m->mb8.ad_parm.parm[2] = 0x03000000;    /* no limit to simul writes */
        m->mb8.ad_parm.parm[3] = 0x04000800;    /* set write block size 2k   */
        m->mb8.ad_parm.parm[4] = 0;
        m->mb28.ad_parm.parm5 = 0;
        /*
         * remaining mailbox area unused
         * complete the rest of the command structure
         */
        cmd->type = SD_ADAP_CMD;                /* this is an adapter command */
        cmd->cmd_info = SD_SPECIAL;             /* this is a special command */
        cmd->b_length = 0;                      /* no data transfer involved */
        cmd->timeout = 3;                       /* set timeout to 3 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* put command on adap q */
        ap->adap_result = 0;
        sd_start(ap);       /* try and start this command */
        if (from_open) {
                /*
                 * if called from open then sleep until completion, else
                 * we were called from the interrupt level after an adapter
                 * reset, so return
                 */
                e_sleep_thread((int *)&ap->adap_event, &(ap->spin_lock),
			       LOCK_HANDLER);
        }

        return(ap->adap_result);

}  /* end sd_set_adap_parms */

/*
 * NAME: sd_cmd_timer
 *
 * FUNCTION: Watchdog timer routine for commands issued off of the error queue
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If sd_start fails, the adapter is reset.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      watchdog        - Watchdog timer structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void    sd_cmd_timer(
struct watchdog *w)
{
        struct sd_watchdog *sdw;
        struct sd_cmd *cmd;
        struct sd_adap_info *ap;
        uint    old_pri;

        /*
         * recast to get sd_watchdog pointer
         */
        sdw = (struct sd_watchdog *)w;

        /*
         * get command pointer
         */
        cmd = (struct sd_cmd *)sdw->pointer;

        old_pri = disable_lock(cmd->ap->dds.intr_priority,
			       &(cmd->ap->spin_lock));

        ap = cmd->ap;


#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,cmdtimer, entry,(char)0,(uint)ap, (uint)cmd, (uint)cmd->cmd_info, (uint)cmd->type,(uint)0);
#endif
#endif
        /*
         * Setup to eventually Log this error
         */
        cmd->status |= SD_LOG_ERROR;
        if (!cmd->uec)
                /*
                 * If uec not already set...don't want to overwrite a media
                 * error uec.
                 */
                cmd->uec = 0x0503;
        /*
         * set flag to force start to look for himself
         */
        ap->adap_resources = TRUE;
        /*
         * set status of command to timed out
         */
        cmd->status |= SD_TIMEDOUT;
        /*
         * Determine what to do
         */
        switch (cmd->type) {
                case SD_ADAP_CMD:
                        if (cmd->cmd_info == SD_RST_QSC) {
                                /*
                                 * if this was a reset quiesce
                                 * then call fail command
                                 */
                                sd_fail_cmd(cmd, (char)FALSE);
                        } else {
                                /*
                                 * quiesce the adapter
                                 */
                                sd_reset_quiesce(ap, (uchar)SD_QUIESCE_OP,
                                        (uchar)SD_ADAP_CMD);
                        }
                        break;
                case SD_CTRL_CMD:
                        if (cmd->cmd_info == SD_RST_QSC) {
                                /*
                                 * if this was a reset quiesce
                                 * then call fail command
                                 */
                                sd_fail_cmd(cmd, (char)FALSE);
                        } else {
                                /*
                                 * quiesce the controller
                                 */
				if (cmd->cmd_info == SD_REQSNS)
                                        /*
                                         * If it was a request sense that
                                         * timed out, clear the pending flag.
                                         * Notice, the request sense is still
                                         * active, and will be retried.
                                         */
                                        cmd->cp->status &= ~SD_REQ_SNS_PENDING;
                                sd_reset_quiesce((struct sd_adap_info *)cmd->cp,
                                   (uchar)SD_QUIESCE_OP, (uchar)SD_CTRL_CMD);
                        }
                        break;
                case SD_DASD_CMD:
                        if (cmd->cmd_info == SD_RST_QSC)
                                /*
                                 * if this was a reset quiesce
                                 * then call fail command
                                 */
                                sd_fail_cmd(cmd, (char)FALSE);
                        else {
                                if (cmd->cmd_info == SD_REQSNS)
                                        /*
                                         * If it was a request sense that
                                         * timed out, clear the pending flag.
                                         * Notice, the request sense is still
                                         * active, and will be retried.
                                         */
                                        cmd->dp->status &= ~SD_REQ_SNS_PENDING;
                                /*
                                 * quiesce dasd
                                 */
                                sd_reset_quiesce((struct sd_adap_info *)cmd->dp,
                                   (uchar)SD_QUIESCE_OP, (uchar)SD_DASD_CMD);
                        }
                        break;
        }

        /*
         * Try to keep us chugging
         */
        sd_start(ap);

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,cmdtimer, exit,(char)0,(uint)ap, (uint)cmd, (uint)0, (uint)0,(uint)0);
#endif
#endif
        unlock_enable(old_pri,&(ap->spin_lock));
        return;
}



/*
 * NAME: sd_dma_cleanup
 *
 * FUNCTION: Performs DMA cleanup on Mailbox and Data DMA transfers
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If system DMA error occurs the system DMA return code
 *      is returnd, and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *                      buf             - Buf structure
 *
 * RETURNS:  Return code from d_complete or xmemout
 */

int  sd_dma_cleanup(
struct  sd_cmd *cmd,
uchar   dma_err)
{
        struct sd_adap_info *ap;
        int     i,
                drc = 0;
        uint    dma_addr,
                temp_addr;
        struct buf *curr;

        ap = cmd->ap;
        /*
         * Cleanup the Mailbox DMA
         */
        (void)d_complete((int)ap->dma_channel,(int)(SD_DMA_TYPE |
                DMA_NOHIDE), (char *)cmd->mb, (size_t)SD_MB_SIZE,
                &ap->xmem_buf, (char *)cmd->mb_dma_addr);
        /*
         * get the dma address of any data transfer, if any
         */
        dma_addr = cmd->mb->mb8.fields.dma_addr;

        if (cmd->cmd_info == SD_NORMAL_PATH) {
                /*
                 * This command came through normal path
                 * (bufs associated with it)
                 */
                /*
                 * this was possibly a coalesced command
                 */
                curr = cmd->bp;
                temp_addr = dma_addr;
                while ( curr != NULL ) {
                        /*
                         * Walk thru buf structures
                         * and cleanup dma
                         */
                        drc |= d_complete((int)ap->dma_channel,
                           SD_DMA_TYPE | ((curr->b_flags & B_READ) ?
                           DMA_READ : 0) | ((curr->b_flags & B_NOHIDE)
                           ? DMA_WRITE_ONLY : 0) , curr->b_un.b_addr,
                           (curr->b_bcount - curr->b_resid), &curr->b_xmemd,
                           (char *)temp_addr);

                        ASSERT(drc == DMA_SUCC);
                        /*
                         * increment the dma_addr
                         */
                        temp_addr += curr->b_bcount;
                        /*
                         * point to next buf struct in chain
                         */
                        curr = (struct buf *)curr->b_work;
                }
        } else {
                /*
                 * This command is from somewhere other than normal path
                 */
                if (cmd->b_length) {
                        /*
                         * if data transfer involved
                         */
                        if (cmd->sta_index == (signed char)-1) {
                                /*
                                 * if not a small transfer
                                 */
                                drc = d_complete((int)ap->dma_channel,
                                        cmd->dma_flags, cmd->b_addr,
                                        cmd->b_length, cmd->xmem_buf,
                                        (char *)dma_addr);

                                ASSERT(drc == DMA_SUCC);
                        } else {
                                /*
                                 * handle small transfer completion.
                                 * issue the dma complete on the data
                                 * transfer. DMA_WRITE_ONLY prevents the
                                 * unhide of the page
                                 */
                                drc = d_complete((int) ap->dma_channel,
                                   (SD_DMA_TYPE | DMA_NOHIDE), ap->STA[0].stap,
                                   (size_t) SD_STA_ALLOC_SIZE, &ap->xmem_buf,
                                   (char *) SD_DMA_ADDR(ap->dds.tcw_start_addr,
                                   ap->sta_tcw_start));

                                ASSERT(drc == DMA_SUCC);

                                if (drc) {
                                   /*
                                    * if any DMA error try to clean
                                    * up various errors by doing a
                                    * re-execute of d_master on the
                                    * small xfer area
                                    */
                                   dma_addr = SD_DMA_ADDR(
                                        ap->dds.tcw_start_addr,
                                        ap->sta_tcw_start);
                                   d_master(ap->dma_channel, (SD_DMA_TYPE |
                                        DMA_NOHIDE), ap->STA[0].stap,
                                        (size_t)SD_STA_ALLOC_SIZE,
                                        &ap->xmem_buf, (char *) dma_addr);

                                   /*
                                    * execute d_complete on the STA to
                                    * allow access to it
                                    */
                                   (void) d_complete(ap->dma_channel,
                                        (SD_DMA_TYPE | DMA_NOHIDE),
                                        ap->STA[0].stap,
                                        (size_t)SD_STA_ALLOC_SIZE,
                                        &ap->xmem_buf, (char *) dma_addr);
                                 } else {
                                   /*
                                    * path for no DMA error, if transfer was
                                    * a read, and not called via DMA error
                                    * cleanup path, then finish transfer by
                                    * copying data to caller
                                    */
                                   if ((cmd->dma_flags == DMA_READ) &&
                                      (!dma_err)) {
                                        /*
                                         * must copy read data to caller's
                                         * buffer
                                         */
                                        if (cmd->xmem_buf->aspace_id ==
                                           XMEM_GLOBAL) {
                                           /*
                                            * copy data from kernel STA to
                                            * caller's area
                                            */
                                           for (i = 0; i < cmd->b_length; i++)
                                             *(cmd->b_addr + i) =
                                             *(ap->STA[cmd->sta_index].stap+i);
                                        } else {
                                           /*
                                            * copy data to user space
                                            */
                                           drc = xmemout(
                                                ap->STA[cmd->sta_index].stap,
                                                cmd->b_addr, cmd->b_length,
                                                cmd->xmem_buf);

                                            ASSERT(drc == XMEM_SUCC);
                                        }
                                     }
                                }
                        }
                }
        }
        return(drc);
}

/*
 * NAME: sd_request_sense
 *
 * FUNCTION: Builds a SCSI Request Sense Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_ctrl_info    - Controller info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_request_sense(
struct sd_dasd_info *dp,
struct sd_ctrl_info *cp,
char    type)
{

        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        /*
         * Allocate and initialize a request sense type
         * cmd for this operation
         */
        if (type == SD_DASD_CMD) {
	    cmd = &(dp->reqsns);
	} else {
	    cmd = &(cp->reqsns);
	}

        if (cmd->status != SD_FREE) {
	    if (cmd->status & SD_ACTIVE) {
		/*
		 * Only happens if command timed out and
		 * reset is active to recover.
		 */
		return;
	    } else {
		/*
		 * make sure command is dequeued
		 */
		sd_d_q_cmd(cmd);
	    }
        }
	
        if (type == SD_DASD_CMD) {
                cmd->xmem_buf = &(dp->xmem_buf);   /* set cross mem descrip */
                cmd->b_addr = (caddr_t)dp->sense_buf; /* set transfer address */
                cmd->ap = dp->ap;               /* set adapter pointer */
                cmd->cp = dp->cp;
                dp->status |= SD_REQ_SNS_PENDING;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_dptrc(dp,reqsns, trc,(char)0,(uint)cmd);
#endif
#endif
        } else {
                cmd->xmem_buf = &(cp->xmem_buf);   /* set cross mem descrip */
                cmd->b_addr = (caddr_t)cp->sense_buf;/* set transfer address */
                cmd->ap = cp->ap;               /* set adapter pointer */
		cp->status |= SD_REQ_SNS_PENDING;
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,reqsns, trc,(char)0,(uint)cmd, (uint)cmd->b_addr ,(uint)type, (uint)0,(uint)0);
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->cp = cp;                   /* set controller pointer */
        cmd->bp = NULL;                 /* clear buf pointer */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = 128;             /* set transfer length */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->dma_flags = DMA_READ;      /* set to read */
        cmd->type = type;               /* set command type */
        cmd->cmd_info = SD_REQSNS;      /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;   /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_NONE;  /* no queue control, no extension  */
        /*
         * set the appropriate device address
         */
        if (type == SD_DASD_CMD)
                m->mb7.dev_address = SD_LUNTAR(dp->cp->dds.target_id,
                        dp->dds.lun_id,SD_LUNDEV);
        else
                m->mb7.dev_address = SD_LUNTAR(cp->dds.target_id,0x00,
                        SD_TARDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_REQUEST_SENSE;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = 128;
        scsi->scsi_bytes[3] = 0x00;

        /*
         * Clear out the sense data buffer
         */
        if (type == SD_DASD_CMD)
                bzero(dp->sense_buf, 256);
        else
                bzero(cp->sense_buf, 256);
        cmd->timeout = 20;              /* set timeout to 20 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
}

/*
 * NAME: sd_start_unit_disable
 *
 * FUNCTION: Builds a SCSI Start Unit Command for the appropriate device
 *	     by first taking the MP spin lock and then calling sd_start_unit
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */
int sd_start_unit_disable(
struct sd_dasd_info *dp,
char    start_flag)
{
	int	ret_code;
	int	old_pri;

        /*
         * Disable interrupts
         */
        old_pri = disable_lock(dp->ap->dds.intr_priority,
			       &(dp->ap->spin_lock));
	ret_code = sd_start_unit(dp,start_flag);

	unlock_enable(old_pri,&(dp->ap->spin_lock));	
	
	return (ret_code);

}

/*
 * NAME: sd_start_unit
 *
 * FUNCTION: Builds a SCSI Start Unit Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

int sd_start_unit(
struct sd_dasd_info *dp,
char    start_flag)
{

        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;
        uint    old_pri;



#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,startunit, entry,(char)0,(uint)dp, (uint)start_flag ,(uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,startunit, entry,(char)0,(uint)start_flag);
#endif
#endif
        cmd = &(dp->restart);
        if (cmd->status != SD_FREE) {
                /*
                 * Command not free, find out where it is
                 */
                if (cmd->status & SD_ACTIVE) {
                        /*
                         * Shouldn't happen, get out and rely on timer
                         */

                        return(-1);
                } else {
                        /*
                         * make sure command is dequeued
                         */
                        sd_d_q_cmd(cmd);
                }
        }
        /*
         * Allocate and initialize a Start unit/Test Unit Ready type
         * cmd for this operation
         */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,startunit, trc,(char)1,(uint)dp, (uint)cmd , (uint)0,(uint) 0,(uint)0);
        sd_dptrc(dp,startunit, trc,(char)1,(uint)cmd );
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->xmem_buf = NULL;           /* clear cross mem descrip */
        cmd->b_length = 0;              /* clear transfer length */
        cmd->rba = 0;                   /* clear rba */
        cmd->b_addr = NULL;             /* clear transfer address */
        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = SD_START_UNIT;  /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered queue control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_START_STOP_UNIT;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        if (start_flag) {
                scsi->scsi_bytes[2] = 0x01;
        } else {
                scsi->scsi_bytes[2] = 0x00;
        }
        scsi->scsi_bytes[3] = 0x00;

        cmd->timeout = 120;              /* set timeout to 2 minutes */
        if(dp->restart_count > SD_MAX_RESTARTS) {
                /*
                 * if we have tried this too many times without success,
                 * give up and fail the command
                 */
                dp->restart_count = 0;
                sd_fail_cmd(cmd, (char)FALSE);
                sd_start(dp->ap); /* call start to avoid a stale-mate */
        } else if (dp->restart_count == SD_TRY_RESET) {
                /*
                 * if this is our next to last attempt then try a reset
                 * of the DASD before we attempt another start unit
                 */
                sd_q_cmd(cmd,(char)SD_STACK);         /* stack this command */
		/*
		 * increment restart count, to avoid infinite resets through
		 * this path
		 */
		dp->restart_count++;
                sd_reset_quiesce((struct sd_adap_info *)dp, (char)SD_RESET_OP,
                        (char)SD_DASD_CMD);
        } else
                sd_q_cmd(cmd,(char)SD_STACK);         /* stack this command */

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,startunit, exit,(char)0,(uint)dp, (uint)cmd , (uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,startunit, exit,(char)0,(uint)cmd);
#endif
#endif

        return(0);
}

/*
 * NAME: sd_test_unit_ready
 *
 * FUNCTION: Builds a SCSI Test Unit Ready Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     good or EBUSY if no concurrent command structure available.
 */

void sd_test_unit_ready(
struct sd_dasd_info *dp,
uchar  cinfo)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        /*
         * Allocate and initialize the appropriate command type
         * cmd for this operation
         */
	
	switch (cinfo)
	{
	  case SD_TEST_UNIT_READY:
	    cmd = &(dp->restart);
	    break;
	  case SD_TEST:
	    cmd = &(dp->concurrent);
	    break;
	  default:
	    ASSERT(FALSE);
	    break;
	}
	
	   
        ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,testunit, trc,(char)0,(uint)dp, (uint)cmd , (uint)0,(uint) 0,(uint)0);
        sd_dptrc(dp,testunit, trc,(char)0,(uint)cmd );
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = cinfo;          /* set command info to given value*/

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered queue control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_TEST_UNIT_READY;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = 0x00;
        scsi->scsi_bytes[3] = 0x00;

        cmd->timeout = 10;               /* set timeout to 10 secs */
	if (cinfo == SD_TEST_UNIT_READY)
	    sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
	else
	    sd_q_cmd(cmd,(char)SD_QUEUE);           /* queue this command */
}


/*
 * NAME: sd_reserve
 *
 * FUNCTION: Builds a SCSI Reserve Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     void
 */

void sd_reserve(
struct sd_dasd_info *dp,
uchar cinfo)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;


        if ((cinfo == SD_LOCK) || (!dp->no_reserve)) {
                /*
                 * if this was not a no reserve on open, then
                 * build reserve command
                 */

		/*
		 * Allocate and initialize the appropriate command type
		 * cmd for this operation
		 */
		
		switch (cinfo)
		{
		  case SD_RESERVE:
		    cmd = &(dp->restart);
		    break;
		  case SD_LOCK:
		    dp->no_reserve = FALSE;
		    cmd = &(dp->concurrent);
		    break;
		  default:
		    ASSERT(FALSE);
		    break;
		}
	

                ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(dp->ap,reserve, trc,(char)0,(uint)dp, (uint)cmd , (uint)0, (uint)0,(uint)0);
                sd_dptrc(dp,reserve, trc,(char)0,(uint)cmd );
#endif
#endif
                cmd->nextcmd = NULL;            /* clear next pointer */
                cmd->dp = dp;                   /* set dasd pointer */
                cmd->ap = dp->ap;               /* set dasd pointer */
                cmd->cp = dp->cp;               /* set controller pointer */
                cmd->xmem_buf = NULL;           /* clear cross mem descrip */
                cmd->b_length = 0;              /* clear transfer length */
                cmd->rba = 0;                   /* clear rba */
                cmd->b_addr = NULL;             /* clear transfer address */
                cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
                cmd->tcws_start = 0;            /* clear starting tcw */
                cmd->sta_index = (signed char)-1;/* init small xfer area index*/
                cmd->status_validity = 0;       /* clear status validity */
                cmd->erp = 0;                   /* clear error recov proc */
                cmd->type = SD_DASD_CMD;        /* set command type */
                cmd->cmd_info = cinfo;          /* set command info as given */

                /*
                 * build copy of eventual mailbox
                 */
                m = &(cmd->mbox_copy);
                m->op_code = SD_SEND_SCSI_OP;         /* Send SCSI Command */
                /*
                 * unordered queue control, no extension
                 */
        	m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
                /*
                 * set device address generated by this dasd's lun and his
                 * controllers target address
                 */
                m->mb7.dev_address =
                        SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
                                SD_LUNDEV);
                /*
                 * Initialize SCSI cmd for operation
                 */
                scsi = &(m->mb8.fields.scsi_cmd);
                scsi->scsi_op_code = SCSI_RESERVE_UNIT;
                scsi->lun = 0x00;
                scsi->scsi_bytes[0] = 0x00;
                scsi->scsi_bytes[1] = 0x00;
                scsi->scsi_bytes[2] = 0x00;
                scsi->scsi_bytes[3] = 0x00;

                cmd->timeout = 30;              /* set timeout to 30 secs */
		if (cinfo == SD_RESERVE)
		    sd_q_cmd(cmd,(char)SD_STACK);     /* stack this command */
		else
		    sd_q_cmd(cmd,(char)SD_QUEUE);     /* queue this command */

        } else {
                /*
                 * else don't reserve, but continue with reset cycle...
                 */
		dp->m_sense_status = SD_SENSE_CHANGEABLE;
                sd_mode_sense(dp);
        }
}



/*
 * NAME: sd_fence
 *
 * FUNCTION: Builds Pseudo-SCSI Fence Commands for the appropriate device
 *           This routine can be called to build two distinct types of fence
 *           command :
 *           SD_FENCE_POS_CHECK - This is simply a no-op fence command to 
 *                                allow the driver to determine the fence
 *                                host position.
 *           SD_FENCE           - This command actually changees the fence
 *                                active in the subsystem by means of a 
 *                                mask_swap type command.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_fence(
struct sd_dasd_info *dp,
uchar type)
{
    struct sd_cmd *cmd;
    struct sd_mbox *m;
    struct sc_cmd *scsi;
    
    
    cmd = &(dp->restart);
    ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
    sd_trc(dp->ap,fence, trc,(char)0,(uint)dp, (uint)cmd , (uint)type, (uint)0,(uint)0);
    sd_dptrc(dp,fence, trc,(char)0,(uint)cmd );
#endif
#endif
    cmd->nextcmd = NULL;                        /* clear next pointer       */
    cmd->dp = dp;                               /* set dasd pointer         */
    cmd->ap = dp->ap;                           /* set dasd pointer         */
    cmd->cp = dp->cp;                           /* set controller pointer   */
    cmd->xmem_buf = &(dp->xmem_buf);            /* set cross mem descrip    */
    cmd->xmem_buf->aspace_id = XMEM_GLOBAL;     /* kernel buffer            */
    cmd->b_length = 0x04;                       /* length of fence data     */
    cmd->b_addr = (caddr_t)dp->sense_buf;       /* set transfer address     */
    cmd->rba = 0;                               /* clear rba                */
    cmd->tcw_words = 0;                         /* clear number of tcw's alloc*/
    cmd->tcws_start = 0;                        /* clear starting tcw       */
    cmd->sta_index = (signed char)-1;           /* init small xfer area index */
    cmd->status_validity = 0;                   /* clear status validity    */
    cmd->erp = 0;                               /* clear error recov proc   */
    cmd->dma_flags = DMA_READ;                  /* set to read              */
    cmd->type = SD_DASD_CMD;                    /* set command type         */
    cmd->cmd_info = type;                       /* set command info         */
    
    /*
     * build copy of eventual mailbox
     */
    m = &(cmd->mbox_copy);
    m->op_code = SD_SEND_SCSI_OP;               /* Send SCSI Command */
    /*
     * unordered queue control, no extension
     */
    m->mb6.qc_scsiext = SD_Q_ORDERED;           /* ordered q control */
    /*
     * set device address generated by this dasd's lun and his
     * controllers target address
     */
    m->mb7.dev_address =
	SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
		  SD_LUNDEV);
    /*
     * Initialize SCSI cmd for operation
     */
    scsi = &(m->mb8.fields.scsi_cmd);
    scsi->scsi_op_code = SD_FENCE_OP_CODE;
    scsi->lun = SD_FENCE_MASK_SWAP;
    
    switch (type) 
    {
      case SD_FENCE:
	scsi->scsi_bytes[0] = (uchar)((dp->fence_mask) >> 8);
	scsi->scsi_bytes[1] = (uchar)((dp->fence_mask) & 0xFF);
	scsi->scsi_bytes[2] = (uchar)((dp->fence_data) >> 8);
	scsi->scsi_bytes[3] = (uchar)((dp->fence_data) & 0xFF);
	break;
      case SD_FENCE_POS_CHECK:
	scsi->scsi_bytes[0] = 0x00;
	scsi->scsi_bytes[1] = 0x00;
	scsi->scsi_bytes[2] = 0x00;
	scsi->scsi_bytes[3] = 0x00;
	break;
      default:
	ASSERT(FALSE);
    }
    scsi->scsi_bytes[4] = 0x00;
    scsi->scsi_bytes[5] = 0x00;
    scsi->scsi_bytes[6] = (uchar)sizeof(struct sd_fence_info);
    scsi->scsi_bytes[7] = 0x00;
    scsi->scsi_bytes[8] = 0x00;
    
    cmd->timeout = 30;                        /* set timeout to 30 secs */
    sd_q_cmd(cmd,(char)SD_STACK);             /* stack this command */
    return;
}


/*
 * NAME: sd_mode_sense
 *
 * FUNCTION: Builds a SCSI Mode Sense Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *       and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_mode_sense(
struct sd_dasd_info *dp)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        /*
         * Allocate and initialize a Mode Sense type
         * cmd for this operation
         */

        cmd = &(dp->restart);
        ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,modesense, trc,(char)0,(uint)dp, (uint)cmd , (uint)0,(uint) 0,(uint)0);
        sd_dptrc(dp,modesense, trc,(char)0,(uint)cmd );
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->xmem_buf = &(dp->xmem_buf);/* set cross mem descrip */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = 0xFF;           /* set length to full mode sense */
        cmd->b_addr = (caddr_t)dp->sense_buf;  /*   set transfer address */
        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->dma_flags = DMA_READ;      /* set to read */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = SD_MODE_SENSE;  /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_MODE_SENSE;
        scsi->lun = 0x00;
	/*
	 * Set Page Code byte to request all supported pages....set Page
 	 * Control bits appropriately depending if we want changeable or
	 * current mode data
	 */
        scsi->scsi_bytes[0] = (0x3F | 
		((dp->m_sense_status == SD_SENSE_CHANGEABLE) ? 
		SD_SENSE_CHANGEABLE : SD_SENSE_CURRENT));
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = (uchar) cmd->b_length;
        scsi->scsi_bytes[3] = 0x00;

        /*
         * Clear out the sense data buffer
         */
        bzero(dp->sense_buf, 256);

        cmd->timeout = 10;              /* set timeout to 10 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
}

/*
 * NAME: sd_mode_select
 *
 * FUNCTION: Builds a SCSI Mode Select Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_mode_select(
struct sd_dasd_info *dp)
{

        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        /*
         * Allocate and initialize a Mode Select type
         * cmd for this operation
         */

        cmd = &(dp->restart);
        ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,modeselect, trc,(char)0,(uint)dp, (uint)cmd , (uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,modeselect, trc,(char)0,(uint)cmd );
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->xmem_buf = &(dp->xmem_buf); /* clear cross mem descrip */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = dp->cd.sense_length; /* set transfer length */
        cmd->b_addr = (caddr_t)dp->sense_buf;
        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->dma_flags = 0;             /* set to write */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = SD_MODE_SELECT; /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_MODE_SELECT;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = dp->cd.sense_length;
        scsi->scsi_bytes[3] = 0x00;

        cmd->timeout = 20;              /* set timeout to 20 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
}


/*
 *
 * NAME: sd_format_mode_data
 *
 * FUNCTION:  This function parses a buffer of mode data into a standard 
 *            control structure to prepare the mode data for comparison    
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is  called at the interrupt level and cannot page fault.
 *
 * (DATA STRUCTURES:)  sd_dasd_dds     DASD's dds information
 *
 * INPUTS:
 *                    mode_data  - Pointer to mode data
 *                    mf         - Pointer to mode format structure
 *
 * (RECOVERY OPERATION:)  None.
 *
 * RETURNS:	Void.
 */

void sd_format_mode_data(
char *mode_data,
struct sd_mode_format *mf,
int	sense_length)
{
	char	i=0,page=0;
	short	bd_length,offset,p_length;

	for (i=0;i<SD_MAX_MODE_PAGES;i++)	
		/* 
		 * initialize all page indices to -1 
		 */
		mf->page_index[i] = (signed char)-1;

	mf->sense_length = sense_length;
	/* 
	 * get length of block descriptor 
	 */
	bd_length = mode_data[SD_BLK_DESC_LN_INDEX];
	if (bd_length == 8) {
		/*
		 * if == 8, we understand this
		 */
		mf->block_length = ((mode_data[9] << 16) | (mode_data[10]<< 8) |
				  mode_data[11]) & 0x00FFFFFF; 
	} else {
		/*
		 * else we don't understand this
		 */
		mf->block_length = 0;	
	}
	/* 
	 * compute offset to first page (i.e. offset to block descriptor +
	 * length of block descriptor area )
	 */
	offset = SD_BLK_DESC_LN_INDEX+bd_length+1;   
	while (offset < mf->sense_length) {
		/*
		 * For the remainder of the sense data...
		 * Store the index into the data buffer for each page
		 */
		page = (mode_data[offset] & SD_MAX_MODE_PAGES);	
		mf->page_index[page] = offset;
		p_length = mode_data[++offset];
		for (i=0;i<=p_length;i++)
			/*
			 * step over the data bytes for this page
			 */
			++offset;
	}
	return;
}
		

/*
 *
 * NAME: sd_mode_data_compare
 *
 * FUNCTION:  Parses a devices changable mode parameters, current mode 
 * 	      parameters, and the using systems desired mode parameters to
 * 	      determine if a mode select is necessary.
 *
 * EXECUTION ENVIRONMENT: This routine is called at the interrupt level and it
 *            can not page fault.
 *
 * DATA STRUCTURES:	sd_dasd_info	-	DASD information structure
 *			sd_mode_format	-	Mode data control information
 *
 * (RECOVERY OPERATION:)   None.
 *
 * RETURNS:	0 - No mode select is necessary (or wouldn't do any good)
 *		1 - Perform mode select with altered current data
 *
 */

int sd_mode_data_compare(
struct sd_dasd_info *dp)
{

	int 	select = 0;
	char	page,i,ch_index,dd_index,df_index,made_change;
	uchar	diff,changeable,def;
	short	ch_length, dd_length, df_length;

	/*
	 * First check block length, if non-zero
	 */
	if (!((dp->dd.block_length == 0) || (dp->cd.block_length == 0)))
		/*
		 * if both the desired data and the current data contain
		 * a defined block length
		 */
		if (dp->dd.block_length != dp->cd.block_length) {
			/*
			 * if they aren't the same, then set flag to do select
			 * and alter the current data block length to reflect
			 * the desired block length
			 */
			select = 1;
			dp->sense_buf[9] = (dp->dd.block_length >> 16) & 0x00ff;
			dp->sense_buf[10] = (dp->dd.block_length >> 8) & 0x00ff;
			dp->sense_buf[11] = dp->dd.block_length & 0x00ff;
		}

	dp->sense_buf[0] = 0;	/* set reserved byte 0 of header to 0 */
	/*
	 * Now check each changeable page
	 */
	for (page=0;page<SD_MAX_MODE_PAGES;page++) {
	    made_change = FALSE;
	    if (dp->ch.page_index[page] != (signed char) -1) {
		/*
		 * if this page is supported
		 */
		ch_index = dp->ch.page_index[page];
		ch_length = dp->ch_data[ch_index+1];
		/* 
		 * mask off reserved bits 6 and 7 of the page code
		 */
		dp->sense_buf[ch_index] &= SD_MAX_MODE_PAGES; 
		if (dp->dd.page_index[page] != (signed char) -1) {
			/*
			 * if this page is in our data base pages, 
			 * then we can and may potentially change this page 
			 * of data
			 */
			dd_index = dp->dd.page_index[page];
			dd_length = dp->dds.mode_data[ch_index+1];
			if (dd_length < ch_length) 
				/*
				 * if our data base has fewer bytes for this
				 * page than the device supports, only look at
				 * those bytes
				 */
				ch_length = dd_length;
			for (i=2;i < ch_length+2; i++) {
				/*
				 * for each changeable byte of this page
				 */
                                def = 0;
                                if (dp->df.page_index[page] != 
					(signed char) -1) {
                                    /*
                                     * if there is default data specified
                                     * for this page
                                     */
                                    df_index = dp->df.page_index[page];
                                    df_length = dp->dds.
					mode_default_data[df_index +1];
                                    if (i < (df_length+2))
                                          /*
                                           * if there is default data specified
                                           * for this byte
                                           */
                                          def = dp->dds.
						mode_default_data[df_index+i];
                                }
				/*
				 * diff = a bitmask of bits we would like to
				 * change. (i.e. current_data XOR desired_data)
				 */
				diff = dp->sense_buf[ch_index + i] ^ 
				       dp->dds.mode_data[dd_index + i];
				/*
				 * changeable = a bitmask of bits we would like
				 * to change and can change. (i.e. difference
				 * between current and desired ANDed with 
				 * changeable bits)
				 */
				changeable = diff & dp->ch_data[ch_index + i];
                                /*
                                 * Now, make sure we don't clobber any bits
                                 * that the data base has specified that we
                                 * should use device default parameters
                                 */
                                changeable &= ~def;

				if (changeable) {
					/*
					 * means we would like to make a change
					 */
					/*
					 * Clear and Set desired bits
					 * (i.e. current data XOR changeable)
					 */
					dp->sense_buf[ch_index +i] ^=
						changeable;
					select = 1;
                                    	made_change = TRUE;
				} /* if changeable */
			} /* for each byte of this page */
		} /* if this page in data base */	
                if (!made_change) {
                        /*
                         * we didn't change anything in this
                         * page, so shift it out of the current and
                         * changeable data
                         */
                        ch_index = dp->ch.page_index[page];
                        ch_length = dp->ch_data[ch_index+1];
                         for (i=ch_index;i<dp->ch.sense_length;i++)
                                dp->ch_data[i] = dp->ch_data[i+ch_length+2];
                         for (i=ch_index;i< dp->cd.sense_length;i++)
                                dp->sense_buf[i] = dp->sense_buf[i+ch_length+2];
                         sd_format_mode_data(dp->ch_data, &dp->ch,
				(int)(dp->ch.sense_length - (ch_length + 2)));
                         sd_format_mode_data(dp->sense_buf, &dp->cd, 
				(int)(dp->cd.sense_length - (ch_length + 2)));
                }
	    } /* if this page supported */					
	} /* for each possible page */
	return(select);
}


/*
 * NAME: sd_inquiry
 *
 * FUNCTION: Builds a SCSI Inquiry Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_inquiry(
struct sd_dasd_info *dp)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        /*
         * Allocate and initialize an inquiry type
         * cmd for this operation
         */

        cmd = &(dp->restart);
        ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,inquiry, trc,(char)0,(uint)dp, (uint)cmd , (uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,inquiry, trc,(char)0,(uint)cmd);
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->xmem_buf = &(dp->xmem_buf);/* set cross mem descrip */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = 255;            /* set length to inquiry length */
        cmd->b_addr = (caddr_t)dp->sense_buf;  /*   set transfer address */
        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->dma_flags = DMA_READ;      /* set to read */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = SD_INQUIRY;     /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;              /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_INQUIRY;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = 0xFF;
        scsi->scsi_bytes[3] = 0x00;

        /*
         * Clear out the sense data buffer
         */
        bzero(dp->sense_buf, 256);

        cmd->timeout = 10;              /* set timeout to 10 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
}

/*
 * NAME: sd_read_cap_disable
 *
 * FUNCTION: Builds a SCSI Read Capacity Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_read_cap_disable(
struct sd_dasd_info *dp,
char    just_cylinder)
{

        uint    old_pri;
        /*
         * Disable interrupts
         */
        old_pri = disable_lock(dp->ap->dds.intr_priority,
			       &(dp->ap->spin_lock));

	sd_read_cap(dp,just_cylinder);
        unlock_enable(old_pri,&(dp->ap->spin_lock));
	return;
}
/*
 * NAME: sd_read_cap
 *
 * FUNCTION: Builds a SCSI Read Capacity Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_read_cap(
struct sd_dasd_info *dp,
char    just_cylinder)
{

        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;


        /*
         * Allocate and initialize a Start unit/Test Unit Ready type
         * cmd for this operation
         */

        if (just_cylinder)
                cmd = &(dp->special);
        else
                cmd = &(dp->restart);
        ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,readcap, trc,(char)0,(uint)dp, (uint)cmd , (uint)just_cylinder,(uint) 0,(uint)0);
        sd_dptrc(dp,readcap, trc,(char)0,(uint)cmd);
#endif
#endif
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->dp = dp;                   /* set dasd pointer */
        cmd->ap = dp->ap;               /* set dasd pointer */
        cmd->cp = dp->cp;               /* set controller pointer */
        cmd->xmem_buf = &(dp->xmem_buf); /* clear cross mem descrip */
        cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
        cmd->b_length = 0x08;           /* set transfer length */
        if (just_cylinder)
                /*
                 * if we just want a cylinder's capacity, then set
                 * buffer address to cylinder capacity structure
                 */
                cmd->b_addr = (caddr_t) &(dp->cyl_capacity);
        else
                /*
                 * else set buffer address to disk capacity structure
                 */
                cmd->b_addr = (caddr_t) &(dp->disk_capacity);

        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->dma_flags = DMA_READ;      /* set to Read */
        cmd->type = SD_DASD_CMD;        /* set command type */
        cmd->cmd_info = SD_READ_CAPACITY; /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        m->op_code = SD_SEND_SCSI_OP;       /* Send SCSI Command */
        m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
        /*
         * set device address generated by this dasd's lun and his
         * controllers target address
         */
        m->mb7.dev_address =
                SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,SD_LUNDEV);
        /*
         * Initialize SCSI cmd for operation
         */
        scsi = &(m->mb8.fields.scsi_cmd);
        scsi->scsi_op_code = SCSI_READ_CAPACITY;
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] = 0x00;
        scsi->scsi_bytes[1] = 0x00;
        scsi->scsi_bytes[2] = 0x00;
        scsi->scsi_bytes[3] = 0x00;
        scsi->scsi_bytes[4] = 0x00;
        scsi->scsi_bytes[5] = 0x00;
        if (just_cylinder)
                /*
                 * if we just want a cylinder's capacity, then set
                 * PMI bit
                 */
                scsi->scsi_bytes[6] = 0x01;
        else
                /*
                 * else get capacity of entire disk
                 */
                scsi->scsi_bytes[6] = 0x00;
        scsi->scsi_bytes[7] = 0x00;

        cmd->timeout = 30;              /* set timeout to 30 secs */
        sd_q_cmd(cmd,(char)SD_STACK);           /* stack this command */
	return;
}

/*
 * NAME: sd_release_disable
 *
 * FUNCTION: Builds a SCSI Release Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     void
 */

void sd_release_disable(
struct sd_dasd_info *dp,
uchar cinfo)
{

        uint    old_pri;

	/*
	 * Disable interrupts
	 */
	old_pri = disable_lock(dp->ap->dds.intr_priority,
			       &(dp->ap->spin_lock));
	sd_release(dp,cinfo);
	unlock_enable(old_pri,&(dp->ap->spin_lock));
}
/*
 * NAME: sd_release
 *
 * FUNCTION: Builds a SCSI Release Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     void
 */

void sd_release(
struct sd_dasd_info *dp,
uchar cinfo)
{
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;

        if ((!dp->retain_reservation) ||
	    (cinfo == SD_UNLOCK)) {
                /*
                 * if (an unlock command) or
		 * (called from close and we are not
                 * supposed to retain the reservation)
                 * then build release
                 */

		/*
		 * Allocate and initialize the appropriate command type
		 * cmd for this operation
		 */
		
		switch (cinfo)
		{
		  case SD_RELEASE:
		    cmd = &(dp->special);
		    break;
		  case SD_UNLOCK:
		    dp->no_reserve = TRUE;
		    cmd = &(dp->concurrent);
		    break;
		  default:
		    ASSERT(FALSE);
		    break;
		}
				
		    
                ASSERT(cmd->status == SD_FREE);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(dp->ap,release, trc,(char)0,(uint)dp, (uint)cmd , (uint)0,(uint) 0,(uint)0);
                sd_dptrc(dp,release, trc,(char)0,(uint)cmd);
#endif
#endif
                cmd->nextcmd = NULL;            /* clear next pointer */
                cmd->dp = dp;                   /* set dasd pointer */
                cmd->ap = dp->ap;               /* set dasd pointer */
                cmd->cp = dp->cp;               /* set controller pointer */
                cmd->xmem_buf = NULL;           /* clear cross mem descrip */
                cmd->b_length = 0;              /* clear transfer length */
                cmd->b_addr = NULL;             /* clear transfer address */
                cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
                cmd->tcws_start = 0;            /* clear starting tcw */
                cmd->sta_index = (signed char)-1;/* init small xfer area index*/
                cmd->status_validity = 0;       /* clear status validity */
                cmd->erp = 0;                   /* clear error recov proc */
                cmd->type = SD_DASD_CMD;        /* set command type */
                cmd->cmd_info = cinfo;          /* set command info as given */

                /*
                 * build copy of eventual mailbox
                 */
                m = &(cmd->mbox_copy);
                m->op_code = 0x80;              /* Send SCSI Command */
        	m->mb6.qc_scsiext = SD_Q_UNORDERED; /* unordered q control */
                /*
                 * set device address generated by this dasd's lun and his
                 * controllers target address
                 */
                m->mb7.dev_address =
                        SD_LUNTAR(dp->cp->dds.target_id,dp->dds.lun_id,
                                SD_LUNDEV);
                /*
                 * Initialize SCSI cmd for operation
                 */
                scsi = &(m->mb8.fields.scsi_cmd);
                scsi->scsi_op_code = SCSI_RELEASE_UNIT;
                scsi->lun = 0x00;
                scsi->scsi_bytes[0] = 0x00;
                scsi->scsi_bytes[1] = 0x00;
                scsi->scsi_bytes[2] = 0x00;
                scsi->scsi_bytes[3] = 0x00;

                cmd->timeout = 20;              /* set timeout to 20 secs */
                sd_q_cmd(cmd, (char)SD_QUEUE);   /* queue up this command */

        } else {
                /*
                 * didn't need a release, wakeup sleeping beauty
                 */
                dp->dasd_result = 0;
                dp->seq_not_done = FALSE;
                e_wakeup((int *)&(dp->dasd_event));
        }
}



/*
 * NAME: sd_reset_quiesce_disable
 *
 * FUNCTION: Builds a Reset/Quiesce Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_reset_quiesce_disable(
struct sd_adap_info *ap,
uchar   op,
uchar   type)
{
	uint	old_pri;
        struct sd_dasd_info *dp;
        struct sd_ctrl_info *cp;
	struct sd_adap_info *ap2;

        /*
         * Disable interrupts
         */

	switch( type) {
	    case    SD_ADAP_CMD:
		ap2 = ap;

		break;
	    case    SD_CTRL_CMD:
		cp = (struct sd_ctrl_info *) ap;
		ap2 = cp->ap;

		break;
	    case    SD_DASD_CMD:	
		dp = (struct sd_dasd_info *) ap;
		ap2 = dp->ap;

		break;
	    default:
		ASSERT(FALSE);
        }
	old_pri = disable_lock(ap2->dds.intr_priority,
			       &(ap2->spin_lock));
	sd_reset_quiesce(ap,op,type);

	unlock_enable(old_pri,&(ap2->spin_lock));
	return;
}
/*
 * NAME: sd_reset_quiesce
 *
 * FUNCTION: Builds a Reset/Quiesce Command for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_reset_quiesce(
struct sd_adap_info *ap,
uchar   op,
uchar   type)
{
        struct sd_dasd_info *dp;
        struct sd_ctrl_info *cp;
        struct sd_cmd *cmd;
        struct sd_mbox *m;
        struct sc_cmd *scsi;
        char    reset_type = 0;
        uint    base;


        switch( type) {
                case    SD_ADAP_CMD:
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,reset_quiesce, entry,(char)0,(uint)type, (uint)op, (uint)ap, (uint)ap->status,(uint)0);
#endif
#endif
                        if (op == SD_RESET_OP) {
                                /*
                                 * Adapter reset
                                 */
                                if (ap->status & SD_RESET_PENDING) {
                                        /*
                                         * no need, already in progress
                                         */

                                        return;
                                }
                                ap->reset_result = SD_RESET_A;
                                ap->status |= SD_RESET_PENDING;
                                sd_write_POS(ap,SD_POS2,
                                        (uchar)SD_ADAP_RESET_BIT);
                                ap->IPL_tmr_cnt = 0;
                                sd_wait_reset(ap->reset_timer);
                                return;
                        } else {
                                /*
                                 * Quiesce adapter
                                 */
                        	if (ap->reset_count > SD_MAX_RESET_RETRIES) {
	                                /*
	                                 * Out of Quiesce Tries...Give up, 
					 * and fail this adapter. Reset 
					 * the adapter.
	                                 */
	                                sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
	                                        (uchar)SD_ADAP_CMD);
	                                ap->reset_count = 0;
	                                sd_flush_adap(ap);

	                                return;
	                        }
                                if (ap->status & SD_RESET_PENDING) {
                                        /*
                                         * no need, higher ERP already
                                         * in progress
                                         */

                                        return;
                                }
                                cmd = &(ap->quiesce);
                                if (cmd->status != SD_FREE)
                                        /*
                                         * Adapter Quiesce Command not free,
                                         * either in progress or sitting on
                                         * a queue
                                         */
                                        if (cmd->status & SD_ACTIVE) {
                                                /*
                                                 * Must be in progress
                                                 */

                                                return;
                                        } else {
                                                /*
                                                 * must be on queue, remove
                                                 * and build again, this way
                                                 * we know it will be stacked
                                                 * on head of queue
                                                 */
                                                sd_d_q_cmd(cmd);
                                        }

                                cmd->ap = ap;
                                cmd->dev_address = 0;
                                reset_type = SD_QUIESCE_ADAP_MB;
                                ap->status |= SD_QUIESCE_PENDING;
                        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,reset_quiesce, exit,(char)0,(uint)type, (uint)op, (uint)ap->status,(uint)cmd,(uint)0);
#endif
#endif
                        break;
                case    SD_CTRL_CMD:
                        /*
                         * either a reset or quiesce controller
                         * recast pointer as controller info pointer
                         */
                        cp = (struct sd_ctrl_info *) ap;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(cp->ap,reset_quiesce, entry,(char)1,(uint)type, (uint)op, (uint)cp, (uint)cp->status,(uint)0);
#endif
#endif
                        if (cp->reset_count > SD_MAX_RESET_RETRIES) {
                                /*
                                 * Out of Reset Tries...Give up, and fail this
                                 * controller. Quiesce the adapter to flush
                                 * any active commands for this dasd
                                 */
                                sd_reset_quiesce(cp->ap, (uchar)SD_QUIESCE_OP,
                                        (uchar)SD_ADAP_CMD);
                                cp->reset_count = 0;
                                sd_flush_ctrl(cp);


                                return;
                        }
                        /*
                         * if already quiescing or resetting this controller,
                         * don't do it again unless it is an escalation
                         */
                        if (op == SD_RESET_OP) {
                                cp->status |= SD_RESET_PENDING;
                                reset_type = SD_RESET_CTRL_MB;
                                cmd = &(cp->reset);
                        }  else  {
                                if ((cp->status & SD_RESET_PENDING) ||
                                     (cp->ap->status & SD_QUIESCE_PENDING )) {
                                        /*
                                         * if this controller has a reset
                                         * pending, or his adapter has a
                                         * quiesce pending
                                         * no need, higher ERP already
                                         * in progress
                                         */

                                        return;
                                }
                                cp->status |= SD_QUIESCE_PENDING;
                                reset_type = SD_QUIESCE_CTRL_MB;
                                cmd = &(cp->quiesce);
                        }
                        if (cmd->status != SD_FREE)
                                /*
                                 * Controller Reset or Quiesce command not
                                 * free, either in progress or sitting on
                                 * a queue
                                 */
                                if (cmd->status & SD_ACTIVE) {
                                        /*
                                         * Already in progress
                                         */

                                        return;
                                } else {
                                        /*
                                         * must be on queue, remove
                                         * and build again, this way
                                         * we know it will be stacked
                                         * on head of queue
                                         */
                                        sd_d_q_cmd(cmd);
                                }
                        cmd->cp = cp;
                        cmd->ap = cp->ap;
                        cmd->dev_address = SD_LUNTAR(cp->dds.target_id,0,
                                SD_TARDEV);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(cp->ap,reset_quiesce, exit,(char)1,(uint)type, (uint)op, (uint)cp->status,(uint)cmd,(uint)0);
#endif
#endif
                        break;
                case    SD_DASD_CMD:
                        /*
                         * either a reset or quiesce DASD
                         * recast pointer as dasd info pointer
                         */
                        dp = (struct sd_dasd_info *) ap;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(dp->ap,reset_quiesce,entry,(char) 2,(uint)type, (uint)op, (uint)dp, (uint)dp->status,(uint)0);
                        sd_dptrc(dp,reset_quiesce,entry,(char) 2,(uint)type);
#endif
#endif
                        if (dp->reset_count > SD_MAX_RESET_RETRIES) {
                                /*
                                 * Out of Reset Tries...Give up, and fail this
                                 * DASD, Quiesce adapter to flush any
                                 * active commands for this dasd
                                 */
                                sd_reset_quiesce(dp->ap, (uchar)SD_QUIESCE_OP,
                                        (uchar)SD_ADAP_CMD);
                                sd_fail_dasd(dp);

                                return;
                        }
                        /*
                         * if already quiescing or resetting this DASD,
                         * don't do it again unless it is an escalation
                         */
                        if (op == SD_RESET_DASD) {
                                /*
                                 * absolute reset
                                 */
                                cmd = &(dp->abs_reset);
                                dp->status |= SD_RESET_PENDING;
                        } else if (op == SD_RESET_OP) {
                                if (dp->cp->status & SD_RESET_PENDING) {
                                        /*
                                         * This DASD's controller has reset
                                         * pending, so
                                         * no need, higher ERP already
                                         * in progress
                                         */

                                        return;
                                }
                                dp->status |= SD_RESET_PENDING;
                                reset_type = SD_RESET_DASD_MB;
                                cmd = &(dp->reset);
                        }  else  {
                                if ((dp->status & SD_RESET_PENDING) ||
                                    (dp->cp->status & ( SD_RESET_PENDING |
                                    SD_QUIESCE_PENDING )) ||
                                    (dp->cp->ap->status & SD_QUIESCE_PENDING)){
                                        /*
                                         * if this dasd has a reset pending or
                                         * his controller has a reset/quiesce
                                         * pending, or his adapter has a
                                         * quiesce pending
                                         * no need, higher ERP already
                                         * in progress
                                         */

                                        return;
                                }
                                dp->status |= SD_QUIESCE_PENDING;
                                reset_type = SD_QUIESCE_DASD_MB;
                                cmd = &(dp->quiesce);
                        }
                        if (cmd->status != SD_FREE)
                                /*
                                 * DASD Reset or Quiesce command not
                                 * free, either in progress or sitting on
                                 * a queue
                                 */
                                if (cmd->status & SD_ACTIVE) {
                                        /*
                                         * Already in progress
                                         */

                                        return;
                                } else {
                                        /*
                                         * must be on queue, remove
                                         * and build again, this way
                                         * we know it will be stacked
                                         * on head of queue
                                         */
                                        sd_d_q_cmd(cmd);
                                }
                        cmd->dp = dp;
                        cmd->cp = dp->cp;
                        cmd->ap = dp->ap;
                        cmd->dev_address = SD_LUNTAR(dp->cp->dds.target_id,
                                dp->dds.lun_id,SD_LUNDEV);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(dp->ap,reset_quiesce, exit,(char)2,(uint)type, (uint)op, (uint)dp->status,(uint)cmd,(uint)0);
                        sd_dptrc(dp,reset_quiesce, exit,(char)2,(uint)type);
#endif
#endif
                        break;
                default:
                        ASSERT(FALSE);
        }
        cmd->nextcmd = NULL;            /* clear next pointer */
        cmd->xmem_buf = NULL;           /* clear cross mem descrip */
        cmd->b_length = 0;              /* clear transfer length */
        cmd->b_addr = NULL;             /* clear transfer address */
        cmd->tcw_words = 0;             /* clear number of tcw's alloc*/
        cmd->tcws_start = 0;            /* clear starting tcw */
        cmd->sta_index = (signed char)-1;/* init small xfer area index*/
        cmd->status_validity = 0;       /* clear status validity */
        cmd->erp = 0;                   /* clear error recov proc */
        cmd->type = type;               /* set command type */
        cmd->cmd_info = SD_RST_QSC;     /* set command info */

        /*
         * build copy of eventual mailbox
         */
        m = &(cmd->mbox_copy);
        if (op == SD_RESET_DASD) {
                /*
                 * if this is an Absolute Reset on the DASD, then
                 * build the Send Diagnostic Reset DASD SCSI command
                 */
                m->op_code = SD_SEND_SCSI_OP;   /* Send SCSI Command */
                m->mb6.qc_scsiext = SD_Q_UNORDERED;  /* no q control, no ext */
                /*
                 * set device address generated by this dasd's lun and his
                 * controllers target address
                 */
                m->mb7.dev_address = cmd->dev_address;

                /*
                 * Initialize SCSI cmd for operation
                 */
                scsi = &(m->mb8.fields.scsi_cmd);
                scsi->scsi_op_code = SCSI_SEND_DIAGNOSTIC;
                scsi->lun = 0x00;
                scsi->scsi_bytes[0] = 0x00;
                scsi->scsi_bytes[1] = 0x00;
                scsi->scsi_bytes[2] = 0x04;
                scsi->scsi_bytes[3] = 0x00;

                /*
                 * Initialized DMA'ed parameters
                 */
                dp->def_list.header = (int)0x81000000;
                cmd->b_length = 0x04;
                cmd->dma_flags = 0;
                /*
                 * set cross mem descrip
                 */
                cmd->xmem_buf = (struct xmem *)&(dp->xmem_buf);
                cmd->xmem_buf->aspace_id = XMEM_GLOBAL;
                cmd->b_addr = (char *)&(dp->def_list);

                cmd->timeout = 125;             /* set timeout to 125 secs */
        } else {
                /*
                 * else build reset or quiesce mailbox
                 */
                m->op_code = SD_RSTQSC_OP;              /* reset op code*/
                m->mb6.reset_type = reset_type;        /* set reset type*/
                /*
                 * set device address generated by this dasd's lun and his
                 * controllers target address
                 */
                m->mb7.dev_address = cmd->dev_address;
                cmd->timeout = 20;              /* set timeout to 20 secs */
        }
        if (reset_type == SD_QUIESCE_ADAP_MB) {
                /*
                 * if this is an adapter quiesce, don't stack the command.
                 * Store the command pointer, and call sd_halt_adap to make
                 * sure the adapter is quiet. sd_halt_adap will then call
                 * sd_fail_adap to clean up mapped commands and then
                 * sd_restart_adap and will place the stored command on the
                 * queue and call start
                 */
                if (!(ap->halt_timer->flags & T_ACTIVE)) {
			/*
			 * If timer not already active, then force the
			 * original call to halt adapter. Otherwise, the
			 * timer will eventually pop and call halt adapter
			 * for us.
			 */
	                ap->halt_timer->func_data = (ulong) cmd;
       		        /*
       		         * Write the pointer to the first mailbox to be issued
       		         * to the Mailbox Pointer Register. This releases an 
			 * interlock in the adapter microcode that is intended 
			 * to cover the asynchronous nature of the interface. 
			 * Unchecked PIO write is safe here since following 
			 * tag=0 never results in DMA.
       		         */
       		        base =(uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
       		        (void)SD_PUTL(base + SD_MBPT,
       		        	SD_MB_ADDR((uint)ap->base_MB_dma_addr, 1));
       		        /*
       		         * Write 0 to last tag register (unchecked PIO write)
       		         * This isn't critical path
       		         */
       		        SD_PUTC((base + SD_LTAG), 0x00);
       		        BUSIO_DET(base);
			ap->IPL_tmr_cnt = 0;
       	         	sd_halt_adap(ap->halt_timer);
		}
        } else {
                /*
                 * stack this command
                 */
                sd_q_cmd(cmd, (char)SD_STACK);
		switch (cmd->type) {
			case SD_ADAP_CMD:
				/*
				 * start delay timer to delay before issueing
				 */
				sd_delay(cmd->ap, (char)SD_ADAP_CMD, (uint)500);
				break;
			case SD_CTRL_CMD:
				/*
				 * start delay timer to delay before issueing
				 */
				sd_delay((struct sd_adap_info *)cmd->cp, 
					(char)SD_CTRL_CMD, (uint)500);
				break;
			case SD_DASD_CMD:
				/*
				 * start delay timer to delay before issueing
				 */
				sd_delay((struct sd_adap_info *)cmd->dp, 
					(char)SD_DASD_CMD, (uint)500);
				break;
		}
	}

}

/*
 * NAME: sd_flush_adap
 *
 * FUNCTION: Terminates all active and pending commands for an adapter.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level, and
 *      can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void sd_flush_adap(
struct sd_adap_info *ap)
{
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        struct sd_cmd *cmd;
        int     i,j;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,flushadap, entry,(char)0,(uint)ap, (uint)ap->status,(uint)ap->errhead, (uint)ap->ioctlhead,(uint)0);
#endif
#endif
        /*
         * call fail adap to clear all active commands out of the command
         * map.
         */
        sd_fail_adap(ap);

        while (ap->errhead != NULL) {
                /*
                 * while there are commands on this adap's
                 * error queue
                 */
                cmd = ap->errhead;
                cmd->b_error = EIO;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }
        while (ap->ioctlhead != NULL) {
                /*
                 * while there are commands on this adap's
                 * ioctl queue
                 */
                cmd = ap->ioctlhead;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }
        while (ap->ctrl_errhead != NULL) {
                /*
                 * while there are commands on this adap's
                 * controller error queue
                 */
                cmd = ap->ctrl_errhead;
                cmd->b_error = EIO;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }
        while (ap->ctrl_ioctlhead != NULL) {
                /*
                 * while there are commands on this adap's
                 * ioctl queue
                 */
                cmd = ap->ctrl_ioctlhead;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }
        for (i = 0; i < SD_NUM_CTRLS; i++) {
                /*
                 * For all of this adapters controllers
                 */
                cp = ap->ctrllist[i];
                if (cp != NULL) {
                        for (j = 0; j < SD_NUM_DASD; j++) {
                                /*
                                 * For all of this controller's DASD
                                 */
                                dp = cp->dasdlist[j];
                                if (dp != NULL)
                                        sd_fail_dasd(dp);
                        }
                }
        }
        /*
         * Clear this adapter's reset count and status.  This way, if new
         * commands come in they will at least go through the error recovery
         * we just tried before they too are flushed.  This provides a window
         * for diagnostics to try and kick some commands off.
         */
        ap->reset_count = 0;
        ap->status = 0;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,flushadap, exit,(char)0,(uint)ap, (uint)ap->status,(uint)0,(uint)0,(uint)0);
#endif
#endif
}

/*
 * NAME: sd_flush_ctrl
 *
 * FUNCTION: Terminates all active and pending commands for a controller.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level, and
 *      can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_ctrl_info    - Controller info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */
void sd_flush_ctrl(
struct sd_ctrl_info *cp)
{
        struct sd_dasd_info *dp;
        struct sd_cmd *cmd;
        int     j;


        cmd = cp->ap->ctrl_errhead;
        while (cmd != NULL) {
                /*
                 * while there are controller commands on this adap's
                 * controller error queue, and they are for this controller
                 */
                if (cmd->cp == cp) {
                        cmd->b_error = EIO;
                        sd_d_q_cmd(cmd);
                        sd_fail_cmd(cmd, (char)TRUE);
                }
                cmd = cmd->nextcmd;
        }
        cmd = cp->ap->ctrl_ioctlhead;
        while (cmd != NULL) {
                /*
                 * while there are controller commands on this adap's
                 * ioctl queue
                 */
                if (cmd->cp == cp) {
                        sd_d_q_cmd(cmd);
                        sd_fail_cmd(cmd, (char)TRUE);
                }
                cmd = cmd->nextcmd;
        }
        for (j = 0; j < SD_NUM_DASD; j++) {
                /*
                 * For all of this controller's DASD
                 */
                dp = cp->dasdlist[j];
                if (dp != NULL)
                        sd_fail_dasd(dp);
        }
        /*
         * Clear this controller's reset count and status.  This way, if new
         * commands come in they will at least go through the error recovery
         * we just tried before they too are flushed.  This provides a window
         * for diagnostics to try and kick some commands off.
         */
        cp->reset_count = 0;
        cp->status = 0;

}

/*
 * NAME: sd_fail_adap_disable
 *
 * FUNCTION: Free's resources for all active commands for an adapter, and
 *      places them on the appropriate queue to be retried.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_fail_adap_disable(
struct sd_adap_info *ap)
{
        uint    old_pri;

        /*
         * Disable interrupts
         */
        old_pri = disable_lock(ap->dds.intr_priority,
			       &(ap->spin_lock));
	sd_fail_adap(ap);
        unlock_enable(old_pri,&(ap->spin_lock));
}
/*
 * NAME: sd_fail_adap
 *
 * FUNCTION: Free's resources for all active commands for an adapter, and
 *      places them on the appropriate queue to be retried.
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *
 * RETURNS:     Void.
 */

void sd_fail_adap(
struct sd_adap_info *ap)
{
        int     i;
        struct sd_cmd *cmd;
        uchar   search;
        char    queue_type,
                scan;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,failadap,entry,(char)0,(uint)ap, (uint)ap->status,(uint)0,(uint)0,(uint)0);
#endif
#endif
        for (scan = 1; scan < 5 ; scan++) {
                /*
                 * Scan the map 4 times, first handling request sense commands
                 * then reassign commands (Notice: these are processed first
                 * to preserve their chain dependence on the "next" command,
                 * which in both cases has to be the head of the error queue)
                 * On the third pass, we scan for any reset/quiesce type
                 * and finally on the fourth pass, we queue up remaining cmds.
                 * No DASD should ever have BOTH a request sense and reassign
                 * ACTIVE.
                 */
                if (scan == 1) {
                        /*
                         * if first scan, set queue type to stack, and
                         * set search to Request Sense
                         */
                        queue_type = SD_STACK;
                        search = SD_REQSNS;
                } else if (scan == 2) {
                        /*
                         * if second scan, set queue type to stack, and
                         * set search to Reassign
                         */
                        queue_type = SD_STACK;
                        search = SD_REASSIGN;
                } else if (scan == 3) {
                        /*
                         * if third scan, set queue type to stack, and
                         * set search to Reset/Quiesce
                         */
                        queue_type = SD_STACK;
                        search = SD_RST_QSC;
                } else
                        /*
                         * else, set queue type to queue
                         */
                        queue_type = SD_QUEUE;
		
                for (i = 1; i < SD_NUM_MB; i++) {
                        /*
                         * for all entries of the command map
                         */
                        cmd = ap->cmd_map[i];
                        if ((cmd != NULL) &&
                            ((cmd->cmd_info == search) || (scan == 4))) {
                                /*
                                 * if there is a command, and it is the type
                                 * we are looking for, or its the last scan
                                 */
                                /*
                                 * Stop appropriate timer
                                 */
                                if (cmd->type == SD_DASD_CMD)
                                        w_stop(&cmd->dp->cmd_timer.watch);
                                else if (cmd->type == SD_CTRL_CMD)
                                         w_stop(&cmd->cp->cmd_timer.watch);
                                else
                                         w_stop(&cmd->ap->cmd_timer.watch);
                                /*
                                 * DMA cleanup on Mailbox and any data transfer
                                 */
                                sd_dma_cleanup(cmd,(uchar)1);
                                /*
                                 * save a copy of our mailbox in cmd jacket
                                 */
                                bcopy ((char *)cmd->mb,
                                        (char *)(&(cmd->mbox_copy)),SD_MB_SIZE);
                                /*
                                 * free this commands mailbox, Notice: this
                                 * should take this command out of the cmd map.
                                 */
                                sd_free_MB(cmd, (char)SD_USED);
                                /*
                                 * Free any TCW's or STA's
                                 */
                                sd_TCW_dealloc(cmd);
                                sd_STA_dealloc(cmd);

                                cmd->status |= SD_RETRY;
				if (cmd->ap->status & SD_DOWNLOAD_PENDING) {
					/*
					 * If adapter download pending, do 
					 * do not count this as a retry
					 */
					  cmd->retry_count--;
				}

                                cmd->retries += 2;
                                if (cmd->retries > 20)
                                        /*
                                         * This is to provide an upper bound on
                                         * retry attempts.  In theory, it is
                                         * possible for one command to cause
                                         * us to reset or quiesce the adapter
                                         * each time it is issued. The problem
                                         * is that we never really know which
                                         * command caused it.  This will weed
                                         * out these commands and put a cap
                                         * on infinite error recovery. So,
                                         * having said that, if this command
                                         * has been retried too many
                                         * times, fail this command.
                                         */
                                        cmd->retries = 0;
                                if (cmd->status & SD_TIMEDOUT) {
                                        /*
                                         * if this command
                                         * previously timed out
                                         */
                                        if ((cmd->cmd_info == SD_IOCTL) ||
                                           (cmd->cmd_info == SD_QRYDEV) ||
                                           (cmd->cmd_info == SD_RST_QSC))
                                                /*
                                                 * if ioctl, query, or reset
                                                 * quiesce, just free
                                                 */
                                                sd_free_cmd(cmd);
                                        else {
                                                cmd->retries -= 2;
                                                /*
                                                 * Queue this command back up
                                                 */
                                                sd_q_cmd(cmd,(char)queue_type);
                                        }
                                } else
                                        /*
                                         * Queue this command back up
                                         */
                                        sd_q_cmd(cmd,(char)queue_type);
                        }
                }
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,failadap, exit,(char)0,(uint)ap, (uint)ap->status, (uint)0, (uint)0,(uint)0);
#endif
#endif

}


/*
 * NAME: sd_fail_dasd
 *
 * FUNCTION: Fails all pending requests and commands for a DASD. This is called
 *      by sd_process_complete after device verification failure.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      buf             - Buf structure
 *
 * RETURNS:     Void.
 */

void sd_fail_dasd(
struct sd_dasd_info *dp)
{
        struct buf *bp_h,
                   *bp_v,
                   *next;
        struct sd_cmd *cmd;
	int	i;
	struct conc_cmd *ccptr;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,faildasd, entry,(char)0,(uint)dp, (uint)dp->status, (uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,faildasd, entry,(char)0,(uint)dp->status);
#endif
#endif
        while (dp->errhead != NULL) {
                /*
                 * while there are commands on this dasd's
                 * error queue
                 */
                cmd = dp->errhead;
		if (dp->dasd_result)		
		    cmd->b_error = dp->dasd_result;
		else
		    cmd->b_error = EIO;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }
	while (dp->conc_cmd_list != NULL) {
	    /* 
	     * While there are concurrent commands
	     * on this dasd's concurrent command queue
	     */
	    ccptr = dp->conc_cmd_list;
	    if (dp->dasd_result)
		ccptr->error = dp->dasd_result;
	    else
		ccptr->error = EIO;
	    dp->conc_cmd_list=ccptr->next;
	    ccptr->next=NULL;
	    /* 
	     * return command to caller..if he is still registered.
	     */
	    sd_return_conc_cmd(dp,ccptr);
	}
        while (dp->ioctlhead != NULL) {
                /*
                 * while there are commands on this dasd's
                 * ioctl queue
                 */
                cmd = dp->ioctlhead;
                sd_d_q_cmd(cmd);
                sd_fail_cmd(cmd, (char)TRUE);
        }

        bp_h = dp->low_cyl;
        while( bp_h != NULL) {
                /*
                 * while there are bufs in this dasd's elevator
                 */
                bp_v = bp_h;
                bp_h = (struct buf *)bp_h->av_forw;     /* point right */
                while(bp_v != NULL) {
                        /*
                         * For each buf on this cylinder
                         */
                        next = (struct buf *)bp_v->b_work;
			if (dp->dasd_result)
			    bp_v->b_error = dp->dasd_result;
			else
			    bp_v->b_error = EIO;
                        bp_v->b_flags |= B_ERROR;
                        bp_v->b_resid = bp_v->b_bcount;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(dp->ap,sdiodone, trc,(char)0,(uint)bp_v, (uint)bp_v->b_error, (uint)bp_v->b_flags, (uint)bp_v->b_resid,(uint)0);
                        sd_dptrc(dp,sdiodone, trc,(char)0,(uint)bp_v);
#endif
#endif
                        iodone(bp_v);
                        bp_v = next;
                }
        }
        dp->currbuf = dp->low_cyl = NULL;
        dp->checked_cmd = NULL;
        dp->reset_count = 0;
        dp->restart_count = 0;
        dp->sick = TRUE;
		
        /*
         * Leave a Start Unit on the queue for this DASD, so if he is
         * accessed again, he will go through the verify process
         */
        (void)sd_start_unit(dp, (char)TRUE);
        /*
         * NOTICE: Removing DASD from start chain, so above start unit won't
         * get started unless this DASD is accessed again
         */
        sd_del_chain(dp, (char)TRUE);
        /*
         * Notice:  Only pending requests for this DASD were flushed, any
         * active commands will either complete (oh well) or attempt to
         * be re-queued after an error or timeout.  
         */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(dp->ap,faildasd, exit,(char)0,(uint)dp, (uint)dp->status, (uint)0, (uint)0,(uint)0);
        sd_dptrc(dp,faildasd, exit,(char)0,(uint)dp->status);
#endif
#endif
}
/*
 * NAME: sd_halt_adap_disable
 *
 * FUNCTION: System timer routine to determine when a Last Tag Register reset
 *      has completed, to then issue an adapter quiesce
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) If the sd_restart_adapter fails, or if sd_start of
 *      the quiesce adapter command fails, or if the Last Tag reset does not
 *      complete within the time allowed then reset the adapter
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *                      trb             - System timer structure
 *
 * RETURNS:     Void.
 */

void sd_halt_adap_disable(
struct trb *t)
{
	struct sd_cmd *cmd;
        int     old_pri;
	struct sd_adap_info *ap;

        /*
         * get adapter pointer
         */
        cmd = (struct sd_cmd *)(t->func_data);

	ap = cmd->ap;

        /* disable my irpt level*/
        old_pri = disable_lock(ap->dds.intr_priority, &(ap->spin_lock));

        sd_halt_adap(t);

        unlock_enable(old_pri,&(ap->spin_lock));
}

/*
 * NAME: sd_halt_adap
 *
 * FUNCTION: System timer routine to determine when a Last Tag Register reset
 *      has completed, to then issue an adapter quiesce
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault. This routine assumes that the
 *	caller is disable_locked when calling this routine.
 *
 * (RECOVERY OPERATION:) If the sd_restart_adapter fails, or if sd_start of
 *      the quiesce adapter command fails, or if the Last Tag reset does not
 *      complete within the time allowed then reset the adapter
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sd_cmd          - Command structure
 *                      trb             - System timer structure
 *
 * RETURNS:     Void.
 */

void sd_halt_adap(
struct trb *t)
{
        struct sd_adap_info *ap;
        struct sd_cmd *cmd;
        uint     base;
        uchar   ctag;

        /*
         * get adapter pointer
         */
        cmd = (struct sd_cmd *)(t->func_data);
        ap = cmd->ap;
        /*
         * Read Last Tag Register and Current Tag Register
         */
        base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);
        if (SD_GETC(base + SD_CTAG, &ctag)) {
                /*
                 * if PIO error, the adapter was reset by pio_recov, so free
                 * the quiesce we would have queued.
                 */
                sd_free_cmd(cmd);
                BUSIO_DET(base);
                return;
        }
        BUSIO_DET(base);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,haltadap, entry,(char)0,(uint)ap, (uint)ap->status, (uint)ctag, (uint)ap->IPL_tmr_cnt,(uint)0);
#endif
#endif
        if (ctag == 0xFF) {
                /*
                 * the last tag reset has completed
                 */
                /*
                 * make sure our command map is empty, recycle
                 * any commands in the map
                 */
                sd_fail_adap(ap);
                /*
                 * reinit the mailbox chain
                 */
                if (sd_restart_adap(ap)) {
                        /*
                         * Must have been PIO error, adapter was reset by
                         * pio_recov, so free the quiesce
                         */
                        sd_free_cmd(cmd);
                } else {
                        /*
                         * else stack up the adapter quiesce command
                         */
                        sd_q_cmd(cmd, (char)SD_STACK);
			/*
			 * start delay timer to delay before issueing cmd
			 */
			sd_delay(cmd->ap, (char)cmd->type, (uint)500);
                }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,haltadap, exit,(char)0,(uint)ap, (uint)ap->status, (uint)ctag, (uint)ap->IPL_tmr_cnt,(uint)0);
#endif
#endif
                return;
        } else {
                /*
                 * last tag reset is still going !
                 */
                if (ap->IPL_tmr_cnt >= SD_MAX_IPL_RETRIES) {
                        /*
                         * timed-out waiting for last tag reset
                         */
                        sd_log_adap_err(ap,(ushort)0xF503,0);
                        sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                (uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,haltadap, exit,(char)1,(uint)ap, (uint)ap->status, (uint)ctag, (uint)ap->IPL_tmr_cnt,(uint)0);
#endif
#endif
                        return;
                }
                /*
                 * set timer to come back and check ctag and ltag again.
                 */
                ap->halt_timer->timeout.it_value.tv_sec = 1;
                ap->halt_timer->timeout.it_value.tv_nsec = 0;
                if (!(ap->halt_timer->flags & T_ACTIVE))
                        /*
                         * Make sure the timer is not active before starting
                         * it, if it is, this will eliminate one of the
                         * threads that caused this in the first place
                         */
                        tstart(ap->halt_timer);
                ap->IPL_tmr_cnt++;    /* inc the timer counter */
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,haltadap, exit,(char)2,(uint)ap, (uint)ap->status, (uint)ctag, (uint)ap->IPL_tmr_cnt,(uint)0);
#endif
#endif
                return;
        }
}
/*
 * NAME: sd_verify_disable
 *
 * FUNCTION: Starts Verification Procedures for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *
 * RETURNS:     Void.
 */

void sd_verify_disable(
struct sd_adap_info *ap,
char    type)
{
        int     old_pri;
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
	struct sd_adap_info *ap2;


	/* disable my irpt level*/

        switch( type) {
                case    SD_ADAP_CMD:
		ap2 = ap;
		break;

                case    SD_CTRL_CMD:
		cp = (struct sd_ctrl_info *) ap;
		ap2 = cp->ap;


		break;
		
                case    SD_DASD_CMD:
		dp = (struct sd_dasd_info *)ap;
		ap2 = dp->ap;

		break;
		
	}

	old_pri = disable_lock(ap2->dds.intr_priority, 
			       &(ap2->spin_lock));
	sd_verify(ap,type);

	unlock_enable(old_pri,&(ap2->spin_lock));
	return;
}

/*
 * NAME: sd_verify
 *
 * FUNCTION: Starts Verification Procedures for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *
 * RETURNS:     Void.
 */

void sd_verify(
struct sd_adap_info *ap,
char    type)
{
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        int     i;

        switch( type) {
                case    SD_ADAP_CMD:

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,verify, entry,(char)0,(uint)ap, (uint)ap->status, (uint)type, (uint)0,(uint)0);
#endif
#endif
                        /*
                         * Need to verify Adapter Microcode level and all
                         * devices on this adapter
                         */
                        if (ap->status &
                                (SD_RESET_PENDING | SD_QUIESCE_PENDING))
                                /*
                                 * if adapter has reset or quiesce pending
                                 */
                                break;
                        /*
                         * Queue up config and download events to daemon
                         */
                        if (!ap->diag_mode)
                                /*
                                 * if not diagnostic mode, queue up config
                                 * and download
                                 */
                                sd_async_event(ap, (uchar)0,
                                        (uchar)(SD_ADAPCFG|SD_ADAPDLMC),
					       SIGTRAP);
                        else
                                /*
                                 * else just a download
                                 */
                                sd_async_event(ap, (uchar)0,
                                        (uchar)(SD_ADAPDLMC),SIGTRAP);
                        for(i=0; i < SD_NUM_CTRLS ;i++) {
                                /*
                                 * for each possible controller on this adapter
                                 */
                                cp = ap->ctrllist[i];
                                if (cp != NULL)
                                        sd_verify((struct sd_adap_info *)cp,
                                                (uchar)SD_CTRL_CMD);
                        }
                        break;
                case    SD_CTRL_CMD:
                        /*
                         * Need to verify Controller Microcode level and all
                         * devices on this controller
                         */
                        cp = (struct sd_ctrl_info *) ap;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cp->ap,verify, entry,(char)1,(uint)ap, (uint)ap->status, (uint)type, (uint)0,(uint)0);
#endif
#endif
                        if (cp->status &
                                (SD_RESET_PENDING | SD_QUIESCE_PENDING))
                                /*
                                 * if controller in diagnostic mode, or has
                                 * a reset or quiesce pending, break out
                                 */
                                break;
                        /*
                         * Queue up config and download events to controller
                         */
                        if ((!cp->ap->diag_mode) && (!cp->diag_mode))
                                /*
                                 * if controller and his adapter not in
                                 * diagnostic mode, queue up config and
                                 * download
                                 */
                                sd_async_event(cp->ap,
                                  (uchar)(((cp->dds.target_id << 5) & 0xE0)
                                   | SD_TARDEV), (uchar)(SD_CONFIG|SD_DLMC),
					       SIGTRAP);
                        else
                                /*
                                 * else, just queue up download
                                 */
                                sd_async_event(cp->ap,
                                  (uchar)(((cp->dds.target_id << 5) & 0xE0)
                                   | SD_TARDEV), (uchar)(SD_DLMC),SIGTRAP);

                        for(i=0; i < SD_NUM_DASD ;i++) {
                                /*
                                 * for each possible dasd on this controller
                                 */
                                dp = cp->dasdlist[i];
                                if (dp != NULL)
                                        /*
                                         * if this dasd is configured, not
                                         * in diagnostic mode, but opened
                                         */
                                        sd_verify((struct sd_adap_info *)dp,
                                                (uchar)SD_DASD_CMD);
                        }
                        break;
                case    SD_DASD_CMD:
                        /*
                         * Need to verify this Dasd's Mode select parameters,
                         * etc.  Notice that no event is queued unless we
                         * determine that this device is not the one that was
                         * previously at this location (after the inquiry)
                         */
                        dp = (struct sd_dasd_info *)ap;
                        if ((dp->diag_mode) || (dp->cp->diag_mode) ||
                            (dp->ap->diag_mode) || (dp->status &
                            (SD_RESET_PENDING | SD_QUIESCE_PENDING)) ||
                            (!dp->opened))
                                /*
                                 * if dasd, his controller, or his adapter
                                 * in diagnostic mode, or has
                                 * a reset or quiesce pending, or isn't open,
                                 * break out
                                 */
                                break;

			if (dp->sick) {
				/*
				 * we have been through sd_fail_dasd, due to
				 * catastrophic error conditions, so don't
				 * try to verify this DASD.  This is to avoid
				 * the never ending sequence caused by a 
				 * subsequent adapter or controller reset that
				 * result in verification of the DASD, which
				 * fails and results in more resets, etc.
				 */
				dp->sick = FALSE; /* toggle; notice this only
						     catches 1 verify pass */
				break;
			}
	
                        dp->status |= SD_VERIFY_PENDING;
                        dp->status &= ~SD_SUSPEND;
                        (void)sd_start_unit(dp, (char)TRUE);
                        break;
        }

}

/*
 * NAME: sd_check_map
 *
 * FUNCTION: Checks the Active Command Map for Command entries for the
 *      specified device. This is used after completion of a Reset or Quiesce
 *      command to verify that all the tags were purged for that device.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *
 * RETURNS:     TRUE    - if there are Active Commands for the specified device
 *              FALSE   - if there are no Active Commands for this device
 */

int sd_check_map(
struct sd_adap_info *ap,
struct sd_ctrl_info *cp,
struct sd_dasd_info *dp,
char    type)
{
        char    i,
                found_cmd = FALSE;
        struct sd_cmd   *cmd;

        for (i = 1; ((i < SD_NUM_MB) && (!found_cmd)); i++) {
                /*
                 * for each entry in the command map
                 */
                cmd = ap->cmd_map[i];
                if (cmd != NULL) {
                        /*
                         * found an active command, now see if it is for
                         * our device
                         */
                        switch (type) {
                                /*
                                 * switching on the level at which we want
                                 * to check
                                 */
                                case    SD_ADAP_CMD:
                                        /*
                                         * if the level to check was adapter,
                                         * then if we find any command in the
                                         * map, we know its for this adapter
                                         */
                                        found_cmd = TRUE;
                                        break;
                                case    SD_CTRL_CMD:
                                        /*
                                         * level to check was controller
                                         */
                                        if ((cmd->type == SD_CTRL_CMD)  &&
                                           (cmd->cp == cp)) {
                                           /*
                                            * if this command was a controller
                                            * command and its this controller,
                                            * then set flag
                                            */
                                           found_cmd = TRUE;
                                           break;
                                        }
                                        if ((cmd->type == SD_DASD_CMD) &&
                                            (cmd->dp->cp == cp)) {
                                           /*
                                            * if this command was a dasd
                                            * command and its controller is
                                            * this controller,then set flag
                                            */
                                           found_cmd = TRUE;
                                        }
                                        break;
                                case    SD_DASD_CMD:
                                        /*
                                         * level to check was dasd
                                         */
                                        if ((cmd->type == SD_DASD_CMD) &&
                                            (cmd->dp == dp)) {
                                            /*
                                             * if this command was a dasd and
                                             * its this dasd, then set flag
                                             */
                                            found_cmd = TRUE;
                                        }
                                        break;
                        }
                }
        }
        return(found_cmd);
}

/*
 * NAME: sd_async_event
 *
 * FUNCTION: Queues up an Asynchronous Event to the RAS Daemon.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      sc_event        - Async Event structure
 *
 * RETURNS:     Void.
 */

void sd_async_event(
struct sd_adap_info *ap,
uchar   address,
uchar   event,
int     signal)
{

        struct sd_event *e,
	                *event_ptr;
	int    found_duplicate = FALSE;
	
        /*
         * Assume interrupts disabled here
         */
	/*
	 * Clear the config bit...We Don't support Dynamic Config Yet.
	 */
	event &= ~SD_CONFIG;
	if ((!event) || (!ap->daemon_open))
		/*
		 * if that leaves nothing to process, leave
		 */
		return;
	/*
	 * To avoid filling the event queue with duplicate entries we
	 * scan it here looking for a duplicate of the entry we are about to
	 * add.
	 */
	event_ptr = ap->event_head;
	while (( NULL != event_ptr) && !found_duplicate)
	{
	    if ((event_ptr->event == event) &&
		(event_ptr->tarlun == address))
		found_duplicate = TRUE;
	    else	
		event_ptr = event_ptr->next_event;
	}    
	
	if (!found_duplicate)
	{
	    /*
	     * get an event structure
	     */
	    e = sd_event_alloc(ap);
	    if (e) {
                /*
                 * initialize fields of event structure
                 */
                e->next_event = NULL;
                e->event = event;
                e->tarlun = address;
                if (ap->event_head == NULL) {
		        ASSERT ( NULL == ap->event_tail );
			/*
                         * event queue was empty, so queue this one and
                         * wakeup the daemon
                         */
                        ap->event_head = e;
                } else {
		        ASSERT ( NULL != ap->event_tail );
                        /*
                         * else the daemon is already busy, so just
                         * queue this one up
                         */
                        ap->event_tail->next_event = e;
                }
                        pidsig(ap->daemon_pid,signal);
                ap->event_tail = e;
	    }
	}
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,async, trc,(char)0,(uint)ap, (uint)e, (uint)event,(uint)address,(uint)found_duplicate);
#endif
#endif
}


/*
 *
 * NAME: sd_sleep
 *
 * FUNCTION: Used to wait on an ioctl event
 *
 * EXECUTION ENVIRONMENT:
 *      This routine is called in the process environment.  It
 *      can not page fault and is pinned
 *
 *
 *
 * (DATA STRUCTURES:)  struct sd_dasd_info  - dasd's information struct
 *                     struct sd_adap_info  - adapter's information struct
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     disable_lock                    unlock_enable
 *     e_sleep_thread
 *
 * RETURNS:      Void
 */

void sd_sleep(
struct sd_adap_info *ap,
uchar   *intrpt,
uint    *event)
{
        int    old_pri;   /* old interrupt level */



        old_pri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));
        if (*intrpt)
                /*
                 * if flag still set, go to sleep
                 */
                e_sleep_thread((int *)event, &(ap->spin_lock),
			       LOCK_HANDLER); 
        /* reenable interrupts */
        unlock_enable(old_pri,&(ap->spin_lock));    

        return;
}

/*
 *
 * NAME: sd_ioctl_timer
 *
 * FUNCTION: Ioctl timeout routine called by watch dog timer
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 * (RECOVERY OPERATION:)  If failure occurs no recovery action takes place
 *
 *
 * (DATA STRUCTURES:)  struct sd_adap_watchdog- timer with pointer to adapter
 *                     struct sd_adap_info   - adapter's information struct
 *
 * CALLED BY:
 *      watchdog timer
 *
 * INTERNAL PROCEDURES CALLED:
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *     e_wakeup
 *
 * RETURNS:      Void
 */

void sd_ioctl_timer(
struct watchdog *w)
{
        struct sd_watchdog *check;
        struct sd_cmd *cmd;
	struct sd_adap_info *ap;
	struct sd_ctrl_info *cp;
	struct sd_dasd_info *dp;
	int	             opri;


        /*
         * can use sd_adap_info if used fields
         * are in the same location in all infos.
         */

        check = (struct sd_watchdog *) w;

        /*
         * get timed out command pointer
         */
         cmd = (struct sd_cmd *)check->pointer;


	/* 
	 * disable interrupts and on MP machines acquire a spin lock.
	 */
	opri = disable_lock(cmd->ap->dds.intr_priority,
			    &(cmd->ap->spin_lock));  


#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,ioctltimer, entry,(char)0,(uint)cmd, (uint)cmd->type, (uint)0,(uint)0,(uint)0);
#endif
#endif
        /*
         * Mark this command as TIMED OUT
         */
        cmd->status |= SD_TIMEDOUT;
	ap = cmd->ap;

        if (cmd->type == SD_ADAP_CMD) {
                /*
                 * If this was an adapter ioctl, quiesce the adapter
                 */
                sd_reset_quiesce(ap, (uchar)SD_QUIESCE_OP, (uchar)SD_ADAP_CMD);
                /*
                 *  notify that a time out
                 *  has occured.
                 */
                ap->ioctl_timeout = 1;
                /*
                 * wake up ioctl call
                 */
                e_wakeup((int *)&ap->ioctl_event);
        } else if (cmd->type == SD_CTRL_CMD) {
                /*
                 * else if this was a controller command, then quiesce the
                 * controller
                 */
		cp = cmd->cp;
                sd_reset_quiesce((struct sd_adap_info *)cp, 
			(uchar)SD_QUIESCE_OP, (uchar)SD_CTRL_CMD);
                /*
                 *  notify that a time out
                 *  has occured.
                 */
                cp->ioctl_timeout = 1;
                /*
                 * wake up ioctl call
                 */
                e_wakeup((int *)&cp->ioctl_event);
        } else {
                /*
                 * else must have been dasd command, so quiesce the dasd
                 */
		dp = cmd->dp;
                sd_reset_quiesce((struct sd_adap_info *)dp, 
			(uchar)SD_QUIESCE_OP, (uchar)SD_DASD_CMD);
                /*
                 *  notify that a time out
                 *  has occured.
                 */
                dp->ioctl_timeout = 1;
                /*
                 * wake up ioctl call
                 */
                e_wakeup((int *)&dp->ioctl_event);

        }
        sd_start(ap);

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,ioctltimer, exit,(char)0,(uint)cmd->type, (uint)0, (uint)0,(uint)0,(uint)0);
#endif
#endif



	unlock_enable(opri,&(ap->spin_lock)); 


        return;
}


/*
 *
 * NAME: sd_add_chain
 *
 * FUNCTION: Add device to the appropriate start chains for sd_start
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine is called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 * (RECOVERY OPERATION:)  If failure occurs no recovery action takes place
 *
 *
 * (DATA STRUCTURES:)  struct sd_dasd_info  - dasd's information struct
 *                     struct sd_adap_info  - adapterr's information struct
 *
 * INPUTS
 *    numflag  -  = 1 if only adapter being added.
 *                = 2 if both adapter and dasd being added.
 *
 * CALLED BY:
 *      sd_ioctl_wait
 *
 * INTERNAL PROCEDURES CALLED:
 *     None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 * RETURNS:      Void
 */

void sd_add_chain(
struct sd_dasd_info *dp)
{
        int                   i;             /* general counter */
        int                   found_flg;     /* 1 if found */
        struct sd_adap_info *ap;

        /*
         * Assume interrupts disabled
         */
        ap = dp->ap;
        if (dp->start_chain != TRUE) {
                /*
                 * if dasd is not already in the
                 * dasd-to-be started chain.
                 * then make this one the new
                 * head of the start chain
                 */
                 dp->nextdp = ap->dphead;
                 ap->dphead = dp;
                 /*
                  * set flag that in start chain
                  */
                 dp->start_chain = TRUE;
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                sd_trc(dp->ap,indasdchain, trc,(char)0,(uint)dp, (uint)dp->nextdp,(uint)ap->dphead, (uint)ap->nextdp,(uint)ap->starting_dp);
                sd_dptrc(dp,indasdchain, trc,(char)0,(uint)dp->nextdp);
#endif
#endif
        }
        return;
}

/*
 * NAME: sd_del_chain
 *
 * FUNCTION: Removes the appropriate DASD and/or Adapter from the start chain
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *
 * RETURNS:     Void.
 */

void    sd_del_chain(
struct sd_dasd_info *dp,
char    force)
{
        struct  sd_adap_info *ap;
        struct  sd_dasd_info *prevdp,
                             *currdp;
        char    done = FALSE;

        /*
         * check to remove dasd from start chain
         */
        ap = dp->ap;
        if ( force || ((dp->currbuf == NULL) && (dp->ioctlhead == NULL) &&
                (dp->errhead == NULL) && (dp->conc_cmd_list == NULL))) {
                /*
                 * if this is a forced removal, or
                 * if nothing outstanding for this dasd
                 * remove it from his adapters start chain
                 */
                if (dp == ap->nextdp)
                        /*
                         * if this was going to be the next dasd
                         * looked at for this adapter, then walk
                         * the adapters next dp pointer.  Notice
                         * that if the next dp is NULL, then our
                         * start routine will reset to dp head
                         */
                        ap->nextdp = dp->nextdp;

                prevdp = NULL;
                currdp = ap->dphead;
                done = FALSE;
                while ((currdp != NULL) && (!done)) {
                        /*
                         * while not at end of list and we haven't
                         * removed it.  Notice that if it is not
                         * in the list, we don't do anything
                         */
                        if ( currdp != dp) {
                                /*
                                 * if this is not it, then walk
                                 * to the next one
                                 */
                                prevdp = currdp;
                                currdp = currdp->nextdp;
                        } else {
                                /*
                                 * else this is it
                                 */
                                if (prevdp == NULL)
                                        /*
                                         * if it was the head, then
                                         * set head to the next one
                                         */
                                        ap->dphead = currdp->nextdp;
                                else
                                        /*
                                         * else, it wasn't the head,
                                         * so just set the previous
                                         * one's next to the next
                                         */
                                        prevdp->nextdp = currdp->nextdp;
                                done = TRUE; /* flag that we're done */
                                dp->start_chain = FALSE;
                                if (dp == ap->starting_dp) {
                                        /*
                                         * if the one we are removing was
                                         * our starting mark, then update
                                         * starting mark to new one
                                         */
                                        if (dp->nextdp != NULL)
                                                 ap->starting_dp = dp->nextdp;
                                        else
                                                 ap->starting_dp = ap->dphead;
                                }
				/*
				 * Clean up his link.  NOTICE: This fixed a
				 * MAJOR bug caused by start being preempted
				 * during an interrupt window, and when we 
				 * came back the current disk had been removed
				 * from the list, but we set ap->nextdp to this
				 * DASD's next pointer.  Results, endless loop
				 * in start cause we never wrap to starting dp.
				 * Setting next to NULL causes a preempted 
				 * start to set ap->nextap to ap->dphead.
				 */
				dp->nextdp = NULL;
				if (ap->dphead == NULL) 
					/*
					 * We just emptied the list
					 */
					ap->nextdp = NULL;
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                                sd_trc(ap,outdasdchain, trc,(char)0,(uint)dp, (uint)dp->nextdp,(uint)ap->dphead,(uint)ap->nextdp,(uint)ap->starting_dp);
                                sd_dptrc(dp,outdasdchain, trc,(char)0,(uint)0);
#endif
#endif
                        } /* else found it */
                } /* while not end of list and not done */
        } /* if nothing outstanding for this dasd */
        return;
}


/*
 * NAME: sd_build_cmd
 *
 * FUNCTION: Builds a SCSI Read/Write/Extended Cmd for the appropriate device
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command structure
 *                      sd_mbox         - Mailbox structure
 *                      sc_cmd          - SCSI command structure
 *
 * RETURNS:     Void.
 */

void sd_build_cmd(
struct sd_cmd *cmd,
uint    flags,
uint    dma_addr)
{

        uint    blk_cnt;
        struct sc_cmd *scsi;

        /*
         * assume interrupts disabled, and mailbox already allocated
         */
        /*
         * set opcode to Send SCSI command
         */
        cmd->mb->op_code = SD_SEND_SCSI_OP;
        /*
         * set queue control to Unordered,
         * and scsi extension to split read/write enabled
	 * if flag field specifies ok to enable split read/write.
         */
       cmd->mb->mb6.qc_scsiext = ((flags & B_SPLIT) ? 
	   (SD_SPLIT_WRITE | SD_SPLIT_READ) : SD_NO_EXT ) | SD_Q_UNORDERED ;

        /*
         * set the device address
         */
        cmd->mb->mb7.dev_address = SD_LUNTAR(cmd->dp->cp->dds.target_id,
                                          cmd->dp->dds.lun_id, SD_LUNDEV);
        cmd->mb->mb8.fields.dma_addr = dma_addr;
        cmd->mb->mb8.fields.dma_length = cmd->b_length;
        scsi = &(cmd->mb->mb8.fields.scsi_cmd);
        blk_cnt = cmd->b_length / SD_BPS;
        if (cmd->bp->b_flags & B_READ) {
                /*
                 * set up for read
                 */
                cmd->dma_flags = DMA_READ;
                /*
                 * Build an extended read SCSI command
                 */
                scsi->scsi_op_code = SCSI_READ_EXTENDED;
        } else {
                /*
                 * set up for write
                 */
                cmd->dma_flags = 0;
                if (cmd->bp->b_options & WRITEV)
                        /*
                         * Initialize SCSI cmd for Write with verify
                         */
                        scsi->scsi_op_code = SCSI_WRITE_AND_VERIFY;
                else
                        /*
                         * Build SCSI extended write
                         */
                        scsi->scsi_op_code = SCSI_WRITE_EXTENDED;
        }
        scsi->lun = 0x00;
        scsi->scsi_bytes[0] =((cmd->rba >> 24) & 0xff);
        scsi->scsi_bytes[1] =((cmd->rba >> 16) & 0xff);
        scsi->scsi_bytes[2] =((cmd->rba >> 8) & 0xff);
        scsi->scsi_bytes[3] = (cmd->rba & 0xff);
        scsi->scsi_bytes[4] = 0x00;
        scsi->scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
        scsi->scsi_bytes[6] = (blk_cnt & 0xff);
        scsi->scsi_bytes[7] = 0x00;
        return;
}


/*
 * NAME: sd_pio_recov
 *
 * FUNCTION: Performs Error Recovery and/or Retries following a PIO error
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) If error reading POS 5, then reset adapter and return
 *      original exception.  If persistent PIO error, then return exception
 *      and recovery is up to the caller.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      pio_except      - PIO exception information
 *
 * RETURNS:     0               - Successful Completion
 *              exception       - PIO exception.
 */

int     sd_pio_recov(
struct sd_adap_info *ap,
int     exception,
uchar   op,
void    *ioaddr,
long    parm,
uchar   vol)
{

        int     retry_count;            /* retry count  */
        uchar   pos5;
        struct  pio_except      except;
        short   uec = 0x0000;

        /*
         * trap if not an io exception
         */
        ASSERT(exception == EXCEPT_IO);

        /*
         * Read POS register 5
         */
        pos5 = sd_read_POS(ap, SD_POS5);
        ap->elog.validity |= SD_POS5_VALID;
        ap->elog.pos_reg_5 = pos5;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,piorecov, entry,(char)0,(uint)ap, (uint)exception, (uint)pos5,(uint)ioaddr,(uint)0);
#endif
#endif
        if ((pos5 == 0xFF) || (pos5 & SD_ADAP_TYPE_1)) {
                /*
                 * Adapter Type 1 error, or couldn't even read POS5,
                 * Need total restart
                 */
                sd_log_adap_err(ap, (ushort)0x04FF,0);
                sd_reset_quiesce(ap, (uchar)SD_RESET_OP, (uchar)SD_ADAP_CMD);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,piorecov, exit,(char)0,(uint)pos5, (uint)0, (uint)0,(uint)0,(uint)0);
#endif
#endif
                return(exception);
        } else if (pos5 & SD_W_DATA_PARITY_ERR) {
                /*
                 * Write Data Parity Error
                 */
                retry_count = 1;
                uec = 0x0480;
        } else if (pos5 & (SD_INVALID_ACCESS | SD_INVALID_STATUS)) {
                /*
                 * Write Data Parity Error, Invalid access, or
                 * Invalid status, retry one time
                 */
                retry_count = 1;
                uec = 0x0481;
        } else {
                /*
                 * Must have been reported by IOCC,
                 * Request additional exception information
                 */
                getexcept((struct except *)&except);
                ap->elog.validity |= SD_CHANNEL_STATUS_VALID;
                ap->elog.chan_status_reg = except.pio_csr;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,piorecov, trc,(char)0,(uint)op, (uint)parm, (uint)vol,(uint)except.pio_csr,(uint)0);
#endif
#endif
                if (vol) {
                        /*
                         * if the data we were reading was volatile,
                         * i.e. Completion Register or Alert Register,
                         * parse the Channel Status Register to determine
                         * if we actually touched the register or not
                         */
                        switch (except.pio_csr & PIO_EXCP_MASK) {
                                case    PIO_EXCP_INV_OP:
                                case    PIO_EXCP_LIMIT:
                                case    PIO_EXCP_PROT:
                                case    PIO_EXCP_PFAULT:
                                case    PIO_EXCP_TPRTY:
                                case    PIO_EXCP_STECC:
                                case    PIO_EXCP_STADR:
                                        /*
                                         * Unrecoverable...
                                         * These should Never Happen
                                         */
                                        uec = 0x0422;
                                        retry_count = 1;
                                        ASSERT(FALSE);
                                        break;
                                case    PIO_EXCP_DPRTY:
                                        /*
                                         * Data parity error occurred on the
                                         * operation, so we know we touched
                                         * the register, so change address to
                                         * address of Backup Register, and
                                         * retry.
                                         */
                                        uec = 0x0502;
                                        retry_count = 3;
                                        ioaddr = (void *)
                                                (((ulong)ioaddr & 0xFFFFFF00) |
                                                 (ulong)(SD_BCKP & 0x000000FF));
                                        break;
                                case    PIO_EXCP_CHNCHK:
                                        /*
                                         * Somebody else drove channel check
                                         * or we would have noticed in our
                                         * POS5 check that we did it
                                         */
                                case    PIO_EXCP_NORSP:
                                        /*
                                         * Card did not respond to address on
                                         * I/O bus
                                         *
                                         * In either case, we didn't touch
                                         * the register
                                         */
                                        uec = 0x0503;
                                        retry_count = 3;
                                        break;
                                case    PIO_EXCP_IOCC:
                                        /*
                                         * IOCC detected internal error,
                                         * Don't know if we touched the
                                         * register or not, fail the op,
                                         * if we did touch it and cleared
                                         * the condition, then we'll rely
                                         * on timers to clean us up, if
                                         * we didn't touch it, then there
                                         * should still be an interrupt
                                         * pending for an automatic retry
                                         */
                                        uec = 0x0502;
                                        retry_count = 0;
                                        break;
                                default:
                                        /*
                                         * We don't know, fail the op and
                                         * rely on timers again
                                         */
                                        uec = 0x0422;
                                        retry_count = 0;
                                        break;
                        }
                } else {
                        /*
                         * Wasn't volatile, so retry
                         */
                        retry_count = 3;
			/*
		 	 * set uec to 0x0502 
		 	 */
			uec = 0x0502;
		}
        }

        while(TRUE) {

                if (retry_count <= 0) {
                        /*
                         * if exceeded retries, log PERM error , reset
                         * adapter, and return the exception
                         */
                        sd_log_adap_err(ap,(ushort)uec,0);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,piorecov, exit,(char)1,(uint)retry_count, (uint)0, (uint)0,(uint)0,(uint)0);
#endif
#endif
                        sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                (uchar)SD_ADAP_CMD);
                        return(exception);
                } else {
                        /*
                         * else still have retries,
                         * decrement retry count
                         */
                        retry_count--;
                }

                /*
                 * retry the pio function, return if successful
                 */
                switch (op) {
                        case PUTC:
                                exception = BUS_PUTCX((char *)ioaddr,
                                        (char)parm);
                                break;
                        case PUTS:
                                exception = BUS_PUTSX((short *)ioaddr,
                                        (short)parm);
                                break;
                        case PUTL:
                                exception = BUS_PUTLX((long *)ioaddr,
                                        (long)parm);
                                break;
                        case GETC:
                                exception = BUS_GETCX((char *)ioaddr,
                                        (char *)parm);
                                break;
                        case GETS:
                                exception = BUS_GETSX((short *)ioaddr,
                                        (short *)parm);
                                break;
                        case GETL:
                                exception = BUS_GETLX((long *)ioaddr,
                                        (long *)parm);
                                break;
                        default:
                                ASSERT(0);
                }

                if (exception == 0) {
                        /*
                         * Recovered, so log recovered adapter error
                         */
                        uec |= 0xF000;
                        sd_log_adap_err(ap,(ushort)uec,0);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,piorecov, exit,(char)2,(uint)0, (uint)0, (uint)0,(uint)0,(uint)0);
#endif
#endif
                        return(0);
                }

        }

}


/*
 * NAME: sd_pio
 *
 * FUNCTION: Performs a PIO operation
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *
 * RETURNS:     0               - Successful Completion
 *              Results of sd_pio_recov, if error
 */

int sd_pio(
struct sd_adap_info *ap,
void    *ioaddr,
char    op,
long    data)
{
        long local_data,
             parm;
        int rc;
        char    read = FALSE;

        /*
         * Notice : No volatile PIO operations will come through
         * this routine, they will be through the performance critical
         * macros
         *
         * Assume the calling routine performed the BUSIO_ATT and will
         * perform the BUSIO_DET
         */
        parm = (long) data;
        switch(op) {
                /*
                 * Perform PIO
                 */
                case    GETC:
                        /*
                         * Read character, parm = address of data
                         */
                        rc = BUS_GETCX((char *)ioaddr, (char *)parm);
                        break;
                case    PUTC:
                        /*
                         * Write character, parm = copy of data
                         */
                        rc = BUS_PUTCX((char *)ioaddr, (char)parm);
                        break;
                case    GETS:
                        /*
                         * Read short, parm = address of data
                         */
                        rc = BUS_GETSX((short *)ioaddr, (short *)parm);
                        break;
                case    PUTS:
                        /*
                         * Write short, parm = copy of data
                         */
                        rc = BUS_PUTSX((short *)ioaddr, (short)parm);
                        break;
                case    GETL:
                        /*
                         * Read long, parm = address of data
                         */
                        rc = BUS_GETLX((long *)ioaddr, (long *)parm);
                        break;
                case    PUTL:
                        /*
                         * Write long, parm = copy of data
                         */
                        rc = BUS_PUTLX((long *)ioaddr, (long)parm);
                        break;
        }

        /*
         * call retry routine if the PIO failed
         */
        if (rc) {
                rc = sd_pio_recov(ap, rc, op, ioaddr, (long)parm,
                        (uchar)NON_VOLATILE);
                if (rc)
                        return(rc);
        }

        return(0);
}


/*
 * NAME: sd_epow
 *
 * FUNCTION: Early Power Off Warning exception handler.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level,
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter info structure
 *                      intr            - System Interrupt structure
 *
 * RETURNS:     INTR_SUCC       - Successful completion of exception.
 */

int     sd_epow(
struct  intr    *is)
{
        int     i,
                opri,
	        ap_opri;
        uint    base;
        struct sd_adap_info *ap;

        opri = disable_lock(INTEPOW,&(sd_epow_lock));
        if (is->flags & EPOW_SUSPEND) {
                /*
                 * if we are to suspend operation, then quiesce and
                 * set suspend flag for each open adapter in the system
                 */
                for (i = 0; i < SD_ADAP_TBL_SIZE; i++) {
                        ap = sd_adap_table[i];
                        while ( ap != NULL ) {
                                if (ap->opened) {
                                        /*
                                         * Write 0 to last tag register to
                                         * stop the adapter from processing
                                         * anything
                                         */
                                        base = (uint)BUSIO_ATT(ap->dds.bus_id,
                                                ap->dds.base_addr);
                                        SD_PUTC((base + SD_LTAG), 0x00);
                                        BUSIO_DET(base);
                                        ap->status |= SD_SUSPEND;
                                }
                                ap = ap->hash_next;
                        }
                }
        } else {
                if (is->flags & EPOW_RESUME) {
                        /*
                         * else if we are to resume operation, then clear
                         * suspend flag for each open adapter in the system
                         * and start him
                         */
                        for (i = 0; i < SD_ADAP_TBL_SIZE; i++) {
                                ap = sd_adap_table[i];
                                while ( ap != NULL ) {
                                        if (ap->opened) {
                                                /*
                                                 * Resume operation by
                                                 * quiescing the adapter
                                                 */
                                                ap->status &= ~SD_SUSPEND;

						ap_opri = disable_lock(
							ap->dds.intr_priority, 
							&(ap->spin_lock));

                                                sd_reset_quiesce(ap,
                                                        (uchar)SD_QUIESCE_OP,
                                                        (uchar)SD_ADAP_CMD);
						 unlock_enable(ap_opri,
							   &(ap->spin_lock));
                                        }
                                        ap = ap->hash_next;
                                }
                        }
                }
        }
        unlock_enable(opri,&(sd_epow_lock));
        return(INTR_SUCC);
}



#ifdef DEBUG

/*
 * NAME: sd_trc_disable
 *
 * FUNCTION: Builds an internal Trace Entry for Debug
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void sd_trc_disable(
struct sd_adap_info *ap,
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{
        uint    old_pri;

        old_pri = disable_lock(INTCLASS2,&(ap->spin_lock));
	sd_trc(ap,desc,type,count,word1,word2,word3,word4,word5);
        unlock_enable(old_pri,&(ap->spin_lock));
	return;
}

/*
 * NAME: sd_trc
 *
 * FUNCTION: Builds an internal Trace Entry for Debug
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void sd_trc(
struct sd_adap_info *ap,
char    *desc,
char    *type,
char    count,
uint    word1,
uint    word2,
uint    word3,
uint    word4,
uint    word5)
{

        bcopy(desc,ap->ap_trace[ap->ap_trcindex].desc, SD_TRC_STR_LENGTH);
        bcopy(type,ap->ap_trace[ap->ap_trcindex].type, 3);
        ap->ap_trace[ap->ap_trcindex].count = count;
        ap->ap_trace[ap->ap_trcindex].word1 = word1;
        ap->ap_trace[ap->ap_trcindex].word2 = word2;
        ap->ap_trace[ap->ap_trcindex].word3 = word3;
        ap->ap_trace[ap->ap_trcindex].word4 = word4;
        ap->ap_trace[ap->ap_trcindex].word5 = word5;
        if(++(ap->ap_trcindex) >= TRCLNGTH)
                ap->ap_trcindex = 0;
        bcopy(topoftrc,ap->ap_trace[ap->ap_trcindex].desc, SD_TRC_STR_LENGTH);
        ap->ap_trctop = (struct sd_trace *)&(ap->ap_trace[ap->ap_trcindex]);
	return;

}
/*
 * NAME: sd_dptrc_disable
 *
 * FUNCTION: Builds an internal Trace Entry for Debug of Specific DASD
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void sd_dptrc_disable(
struct sd_dasd_info *dp,
char    *desc,
char    *type,
char    count,
uint    word1)
{
        uint    old_pri;

        old_pri = disable_lock(INTCLASS2,&(dp->ap->spin_lock));
	sd_dptrc(dp,desc,type,count,word1);
        unlock_enable(old_pri,&(dp->ap->spin_lock));

}
/*
 * NAME: sd_dptrc
 *
 * FUNCTION: Builds an internal Trace Entry for Debug of Specific DASD
 *
 * EXECUTION ENVIRONMENT: This routine is called on both the process and
 *      interrupt levels, and can not page fault.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:) None.
 *
 * RETURNS:     Void.
 */

void sd_dptrc(
struct sd_dasd_info *dp,
char    *desc,
char    *type,
char    count,
uint    word1)
{

        bcopy(desc,dp->dp_trace[dp->dp_trcindex].desc, SD_TRC_STR_LENGTH);
        bcopy(type,dp->dp_trace[dp->dp_trcindex].type, 3);
        dp->dp_trace[dp->dp_trcindex].count = count;
        dp->dp_trace[dp->dp_trcindex].word1 = word1;
        if(++dp->dp_trcindex >= 100)
                dp->dp_trcindex = 0;
        bcopy(topoftrc,dp->dp_trace[dp->dp_trcindex].desc, SD_TRC_STR_LENGTH);
        dp->dp_trctop = (struct dp_trace *)(&dp->dp_trace[dp->dp_trcindex]);
}
#endif

/*
 *
 * NAME:        sd_adap_cdt
 *
 * FUNCTION:    Registers serial dasd adapters for the compenent dump
 *              table. This routine is called at dump time.  The component
 *              dump table has been allocated at config time, so this
 *              routine fills in the table and returns the address.
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine must not be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 *
 * (DATA STRUCTURES:)  struct cdt          - component dump table
 *                     struct sd_adap_info - adapter info structure
 *
 * INPUTS:
 *              arg - 1 indicates the start of a dump
 *                    2 indicates the end of a dump
 *
 * INTERNAL PROCEDURES CALLED:
 *
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *     sizeof                           bcopy
 *
 * (RECOVERY OPERATION:)
 *
 *     None
 *
 * RETURNS:
 *
 *      This routine returns the address of the component
 *       dump table.
 */

struct cdt *sd_adap_cdt_func(int arg)
{
        struct sd_adap_info     *ap;             /* adapter info structure  */
        struct cdt_entry        *entry_ptr;      /* pointer to entry point  */
        int                     entry_index = 0; /* entry point             */
        int                     i;               /* general counter         */


        if (arg == 1) {

                /*
                 * Entry point for adapter hash table
                 */
                entry_ptr = &sd_adap_cdt->cdt_entry[entry_index++];
                bcopy("ap_tble",entry_ptr->d_name,8);
                entry_ptr->d_len =
                        sizeof(int) * SD_ADAP_TBL_SIZE;
                entry_ptr->d_ptr = (caddr_t)&sd_adap_table[0];
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_open_adaps: number
                 * of open adapters
                 */
                entry_ptr = &sd_adap_cdt->cdt_entry[entry_index++];
                bcopy("open_ap",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(uchar);
                entry_ptr->d_ptr = (caddr_t) &sd_open_adaps;
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_inited_adaps: number of initialized
                 * adapters
                 */

                entry_ptr = &sd_adap_cdt->cdt_entry[entry_index++];
                bcopy("init_ap",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(char);
                entry_ptr->d_ptr = (caddr_t) &sd_inited_adaps;
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_global_lock
                 */
                entry_ptr = &sd_adap_cdt->cdt_entry[entry_index++];
                bcopy("lock",entry_ptr->d_name,5);
                entry_ptr->d_len = sizeof(lock_t);
                entry_ptr->d_ptr = (caddr_t)&sd_global_lock;
                entry_ptr->d_segval = 0;


                entry_index = SD_NUM_ADAP_ENTRIES - 1;

                /*
                 * Create an entry point for the sd_adap_info
                 * of all configured adapters
                 */
                for (i = 0;i < SD_ADAP_TBL_SIZE;i++) {

                        /*
                         * Using hash table sd_adap_table
                         * where collisions are handled using a linked list
                         * of pointers for the the same index
                         */
                        ap = sd_adap_table[i];
                        /*
                         * Locate all collided adapter
                         * with this same index and create an
                         * entry point for them
                         */

                        while (ap != NULL) {
                                entry_ptr =
                                        &sd_adap_cdt->cdt_entry[++entry_index];
                                bcopy(ap->dds.resource_name,
                                      entry_ptr->d_name,8);
                                entry_ptr->d_ptr = (caddr_t) ap;
                                entry_ptr->d_len = sizeof(struct sd_adap_info);
                                entry_ptr->d_segval = 0;
                                ap = ap->hash_next;
                        }
                } /* for */
        }
        return(sd_adap_cdt);
}
/*
 *
 * NAME:        sd_ctrl_cdt
 *
 * FUNCTION:    Registers serial dasd controllers for the compenent dump
 *              table. This routine is called at dump time.  The component
 *              dump table has been allocated at config time, so this
 *              routine fills in the table and returns the address.
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine must not be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 *
 * (DATA STRUCTURES:)  struct cdt          - component dump table
 *                     struct sd_ctrl_info - controller info structure
 *
 * INPUTS:
 *              arg - 1 indicates the start of a dump
 *                    2 indicates the end of a dump
 *
 * INTERNAL PROCEDURES CALLED:
 *
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *     sizeof                           bcopy
 *
 * (RECOVERY OPERATION:)
 *
 *     None
 *
 * RETURNS:
 *
 *      This routine returns the address of the component
 *       dump table.
 */

struct cdt *sd_ctrl_cdt_func(int arg)
{
        struct sd_ctrl_info     *cp;             /*controller info structure*/
        struct cdt_entry        *entry_ptr;      /* pointer to entry point  */
        int                     entry_index = 0; /* entry point             */
        int                     i;               /* general counter         */


        if (arg == 1) {
                /*
                 * Entry point for sd_ctrl_table: controllers hash table
                 */
                entry_ptr = &sd_ctrl_cdt->cdt_entry[entry_index++];
                bcopy("cp_tble",entry_ptr->d_name,8);
                entry_ptr->d_len =
                        sizeof(int) * SD_CTRL_TBL_SIZE;
                entry_ptr->d_ptr = (caddr_t)&sd_ctrl_table[0];
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_inited_ctrls: number of initialized
                 * controllers
                 */
                entry_ptr = &sd_ctrl_cdt->cdt_entry[entry_index++];
                bcopy("init_cp",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(char);
                entry_ptr->d_ptr = (caddr_t) &sd_inited_ctrls;
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_open_ctrls: number of open
                 * controllers
                 */
                entry_ptr = &sd_ctrl_cdt->cdt_entry[entry_index++];
                bcopy("open_cp",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(uchar);
                entry_ptr->d_ptr = (caddr_t) &sd_open_ctrls;
                entry_ptr->d_segval = 0;

                entry_index = SD_NUM_CTRL_ENTRIES - 1;

                entry_ptr--;            /* decrement so the below loop works */
                /*
                 * Create an entry point for the sd_ctrl_info
                 * of all configured contrllers
                 */
                for (i = 0;i < SD_CTRL_TBL_SIZE;i++) {
                        /*
                         * Using hash table sd_ctrl_table
                         * where collisions are handled using a linked list
                         * of pointers for the the same index
                         */
                        cp = sd_ctrl_table[i];
                        /*
                         * Locate all collided controllers
                         * with this same index and create an
                         * entry point for them
                         */

                        while (cp != NULL) {
                                entry_ptr =
                                        &sd_ctrl_cdt->cdt_entry[++entry_index];
                                bcopy(cp->dds.resource_name,
                                      entry_ptr->d_name,8);
                                entry_ptr->d_ptr = (caddr_t) cp;
                                entry_ptr->d_len = sizeof(struct sd_ctrl_info);
                                entry_ptr->d_segval = 0;
                                cp = cp->hash_next;
                        }
                } /* for */
        }
        return(sd_ctrl_cdt);
}
/*
 *
 * NAME:        sd_dasd_cdt
 *
 * FUNCTION:    Registers serial dasd  for the compenent dump
 *              table. This routine is called at dump time.  The component
 *              dump table has been allocated at config time, so this
 *              routine fills in the table and returns the address.
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This routine must not be called in the interrupt environment.  It
 *      can not page fault and is pinned
 *
 *
 * (DATA STRUCTURES:)  struct cdt          - component dump table
 *                     struct sd_dasd_info - dasd info structure
 *
 * INPUTS:
 *              arg - 1 indicates the start of a dump
 *                    2 indicates the end of a dump
 *
 * INTERNAL PROCEDURES CALLED:
 *
 *      None
 *
 * EXTERNAL PROCEDURES CALLED:
 *
 *     sizeof                           bcopy
 *
 * (RECOVERY OPERATION:)
 *
 *     None
 *
 * RETURNS:
 *
 *      This routine returns the address of the component
 *       dump table.
 */

struct cdt *sd_dasd_cdt_func(int arg)
{
        struct sd_dasd_info     *dp;             /* dasd info structure     */
        struct cdt_entry        *entry_ptr;      /* pointer to entry point  */
        int                     entry_index = 0; /* entry point             */
        int                     i;               /* general counter         */



        if (arg == 1) {
                /*
                 * Entry point for sd_dasd_table: dasd hash table
                 */
                entry_ptr = &sd_dasd_cdt->cdt_entry[entry_index++];
                bcopy("dp_tble",entry_ptr->d_name,8);
                entry_ptr->d_len =
                        sizeof(int) * SD_DASD_TBL_SIZE;
                entry_ptr->d_ptr = (caddr_t)&sd_dasd_table[0];
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_inited_dasd: number of initialized
                 * dasd
                 */
                entry_ptr = &sd_dasd_cdt->cdt_entry[entry_index++];
                bcopy("init_dp",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(char);
                entry_ptr->d_ptr = (caddr_t)&sd_inited_dasd;
                entry_ptr->d_segval = 0;

                /*
                 * Entry point for sd_open_dasd: number
                 * of open dasd
                 */
                entry_ptr = &sd_dasd_cdt->cdt_entry[entry_index++];
                bcopy("open_dp",entry_ptr->d_name,8);
                entry_ptr->d_len = sizeof(uchar);
                entry_ptr->d_ptr = (caddr_t)&sd_open_dasd;
                entry_ptr->d_segval = 0;

                entry_index = SD_NUM_DASD_ENTRIES - 1;

                /*
                 * Create an entry point for the sd_dasd_info
                 * of all configured DASD
                 */
                for (i = 0;i < SD_DASD_TBL_SIZE;i++) {
                        /*
                         * Using hash table sd_dasd_table
                         * where collisions are handled using a linked list
                         * of pointers for the the same index
                         */
                        dp = sd_dasd_table[i];
                        /*
                         * Locate all collided DASD
                         * with this same index and create an
                         * entry point for them
                         */
                        while (dp != NULL) {
                                entry_ptr =
                                        &sd_dasd_cdt->cdt_entry[++entry_index];
                                bcopy(dp->dds.resource_name,
                                      entry_ptr->d_name,8);
                                entry_ptr->d_ptr = (caddr_t) dp;
                                entry_ptr->d_len = sizeof(struct sd_dasd_info);
                                entry_ptr->d_segval = 0;
                                dp = dp->hash_next;
                        }
                } /* for */
        }
        return(sd_dasd_cdt);
}


/*
 * NAME: sd_walk_event
 *
 * FUNCTION: Walks the Event Head pointer to the next Event to process
 *
 * EXECUTION ENVIRONMENT: This routine is called on the process level by
 *      sd_daemon, but disables interrupts and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter Information Structure
 *
 * RETURNS:     Void.
 */

void sd_walk_event(
struct sd_adap_info *ap)
{
        uint    opri;

        /*
         * Disable interrupts and walk the event head pointer
         */
        opri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));
        ap->event_head = ap->event_head->next_event;
	/* 
	 * If we dequeued the last event, tidy up the tail pointer
	 * too
	 */
	if (NULL == ap->event_head)
	    ap->event_tail = NULL;
        unlock_enable(opri,&(ap->spin_lock));
}

/*
 * NAME: sd_log_error
 *
 * FUNCTION: Logs errors against the Serial DASD Subsystem
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_cmd          -  Command structure
 *                      sd_dasd_info    - DASD Information structure
 *                      sc_error_log_df - SCSI Error Log Data
 *
 * RETURNS:     Void.
 */
void sd_log_error(
struct sd_cmd *cmd)
{
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        struct sd_adap_info *ap;
        struct sd_logbook *lb;
        struct sc_error_log_df *log;
        char *name;
        struct timestruc_t ctime;
        uint    errid;
        ulong   last_log;
        uchar    etype;
	char	ii;
	int     ctrl_flag= 0;          /* if logging against controller  */


        /*
         * get the current time
         */
        curtime(&ctime);

        ap = cmd->ap;
        switch (cmd->uec) {
                case 0x0101:
                case 0x0111:
                case 0x0121:
                case 0x0131:
                case 0x0141:
                case 0x0391:
                case 0x03A3:
                case 0x03A4:
                case 0x03A5:
                case 0x03A6:
                case 0x03A7:
                case 0x03C2:
                case 0x03F0:
	        case 0x0800:
	        case 0x0801:
	        case 0x0802:
	        case 0x0805:
	        case 0x0806:

                        /*
                         * D A S D  HARDWARE ERROR
                         */
                        if ((cmd->type != SD_DASD_CMD) || (cmd->dp->diag_mode)
                           || (cmd->dp->cp->diag_mode) || (cmd->ap->diag_mode))
                                /*
                                 * make sure this was a DASD command , and
                                 * that he nor his parents are in diagnostic
                                 * mode before we try and log an error
                                 * against a DASD
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR) {
                                /*
                                 * if this was a recovered error, log as
                                 * recovered
                                 */
                                errid = ERRID_DISK_ERR4;
                                cmd->uec |= 0xF000;
                                etype = E_TYPE_D4;
                        } else {
                                /*
                                 * else, log as physical volume hardware
                                 * error
                                 */
                                errid = ERRID_DISK_ERR2;
                                etype = E_TYPE_D2;
                        }
                        dp = cmd->dp;
                        log = &(dp->elog);
                        lb = dp->logbook;
                        log->reserved2 = dp->dds.segment_cnt;
                        log->reserved3 = dp->dds.byte_count;
                        name = dp->dds.resource_name;
                        break;
	        case 0x0000:
                case 0x0281:
                case 0x0282:
                case 0x0283:
                case 0x0284:
                case 0x0285:
                case 0x0286:
                case 0x0288:
                case 0x0290:
                case 0x0291:
                case 0x03A0:
                case 0x03A1:
                case 0x0803:
                        /*
                         * D A S D  MEDIA ERROR
                         */
                        if ((cmd->type != SD_DASD_CMD) || (cmd->dp->diag_mode)
                           || (cmd->dp->cp->diag_mode) || (cmd->ap->diag_mode))
                                /*
                                 * make sure this was a DASD command , and
                                 * that he nor his parents are in diagnostic
                                 * mode before we try and log an error
                                 * against a DASD
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR) {
                                /*
                                 * if this was a recovered error, log as
                                 * recovered
                                 */
                                errid = ERRID_DISK_ERR4;
				cmd->uec |= 0xF000;
                        } else
                                /*
                                 * else, log as physical volume media
                                 * error
                                 */
                                errid = ERRID_DISK_ERR1;
                        /*
                         * NOTICE: We don't threshold MEDIA Errors
                         */
                        etype = FORCED_LOG;
                        dp = cmd->dp;
                        log = &(dp->elog);
                        lb = dp->logbook;
                        log->reserved2 = dp->dds.segment_cnt;
                        log->reserved3 = dp->dds.byte_count;
                        name = dp->dds.resource_name;
                        break;
                case 0x01C1:
                case 0x01C8:
                case 0x01FE:
                case 0x01FF:
                case 0x02FF:
                case 0x0382:
                case 0x03FD:
                case 0x03FE:
                case 0x03FF:
		case 0x0506:
                case 0x0804:
                case 0x08FE:
                        /*
                         * Software or Microcode Errors
                         */
                        if ((cmd->type == SD_DASD_CMD) && (!cmd->dp->diag_mode)
                          && (!cmd->dp->cp->diag_mode) && 
			    (!cmd->ap->diag_mode)) {
                                /*
                                 * If DASD command and not in diagnostic mode,
                                 * and controller and adapter not in diag mode,
                                 * get his controllers ptr
                                 */
                                cp = cmd->dp->cp;
				ctrl_flag = 1;
			}
                        else if ((cmd->type == SD_CTRL_CMD) &&
                                (!cmd->cp->diag_mode) && 
				 (!cmd->ap->diag_mode)) {
                                /*
                                 * else If CTRL command, and he's not in
                                 * diag mode and his adapter not in diag mode,
                                 * get his ptr
                                 */
                                cp = cmd->cp;
				ctrl_flag = 1;
				
			}
                        else if ((cmd->type == SD_ADAP_CMD) && 
				 (!cmd->ap->diag_mode)) {
				/*
                                 * else If ADAP command, and he's not in
                                 * diag mode
				 */
				ctrl_flag = 0;
				ap = cmd->ap;
				errid = ERRID_SDM_ERR1;
				etype = E_TYPE_M1;
				log =(struct sc_error_log_df *)&(ap->elog);
				lb = ap->logbook;
				name = ap->dds.resource_name;
			}
			else 
                                /*
                                 * Can't log error against controller
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR)
                                /*
                                 * if this was a recovered error
                                 */
                                cmd->uec |= 0xF000;
			if (ctrl_flag) {
				cp = cmd->cp;
				errid = ERRID_SDM_ERR1;
				etype = E_TYPE_M1;
				log = &(cp->elog);
				lb = cp->logbook;
				name = cp->dds.resource_name;
			}
                        break;
                case 0x0321:
                case 0x0322:
                case 0x0324:
                case 0x0351:
                case 0x0353:
                        /*
                         * Controller/DASD Link Errors
                         */
                        if ((cmd->type == SD_DASD_CMD) && (!cmd->dp->diag_mode)
                          && (!cmd->dp->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * If DASD command and not in diagnostic mode,
                                 * and controller and adapter not in diag mode,
                                 * get his controllers ptr
                                 */
                                cp = cmd->dp->cp;
                        else if ((cmd->type == SD_CTRL_CMD) &&
                                (!cmd->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * else If CTRL command, and he's not in
                                 * diag mode and his adapter not in diag mode,
                                 * get his ptr
                                 */
                                cp = cmd->cp;
                        else
                                /*
                                 * Can't log error against controller
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR)
                                /*
                                 * if this was a recovered error
                                 */
                                cmd->uec |= 0xF000;
                        errid = ERRID_SDC_ERR1;
                        etype = E_TYPE_C1;
                        log = &(cp->elog);
                        lb = cp->logbook;
                        name = cp->dds.resource_name;
                        break;

		case 0x0405:
                case 0x0408:
                case 0x040A:
                case 0x040B:
                case 0x0411:
                case 0x0417:
                case 0x0421:

			/*
			 * errors detected by adapter to be logged against
			 * controller. Need to set up cmd->req_sns_bytes[]
			 * with psuedo sense.
			 */

			bzero(&cmd->req_sns_bytes, SD_REQ_SNS_LENGTH);
			cmd->req_sns_bytes[22] = cmd->alert_tag;
			cmd->req_sns_bytes[23] = cmd->dev_address;
			cmd->req_sns_bytes[24] = cmd->adapter_status;
			cmd->req_sns_bytes[25] = cmd->controller_status;
			
			/*
			 * Fall through to deal with as per other controller
			 * errors
			 */

                case 0x0301:
                case 0x0323:
                case 0x0380:
                case 0x0381:
                case 0x03C3:
                case 0x0500:
                case 0x0501:
		case 0x0505:
			
                        /*
                         * Controller Hardware Errors
                         */
                        if ((cmd->type == SD_DASD_CMD) && (!cmd->dp->diag_mode)
                          && (!cmd->dp->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * If DASD command and not in diagnostic mode,
                                 * and controller and adapter not in diag mode,
                                 * get his controllers ptr
                                 */
                                cp = cmd->dp->cp;
                        else if ((cmd->type == SD_CTRL_CMD) &&
                                (!cmd->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * else If CTRL command, and he's not in
                                 * diag mode and his adapter not in diag mode,
                                 * get his ptr
                                 */
                                cp = cmd->cp;
                        else
                                /*
                                 * Can't log error against controller
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR) {
                                cmd->uec |= 0xF000;
                                errid = ERRID_SDC_ERR3;
                                etype = E_TYPE_C3;
                        } else {
                                errid = ERRID_SDC_ERR2;
                                etype = E_TYPE_C2;
                        }
                        log = &(cp->elog);
                        lb = cp->logbook;
                        name = cp->dds.resource_name;
                        break;
                case 0xF380:
                case 0xF381:
                case 0xF382:
                case 0xF3FF:
                case 0xF405:
                case 0xF408:
                case 0xF40A:
                case 0xF40B:
                case 0xF411:
                case 0xF417:
                case 0xF421:
                        /*
                         * Recovered Controller Hardware Errors
                         */
                        if ((cmd->type == SD_DASD_CMD) && (!cmd->dp->diag_mode)
                          && (!cmd->dp->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * If DASD command and not in diagnostic mode,
                                 * and controller and adapter not in diag mode,
                                 * get his controllers ptr
                                 */
                                cp = cmd->dp->cp;
                        else if ((cmd->type == SD_CTRL_CMD) &&
                                (!cmd->cp->diag_mode) && (!cmd->ap->diag_mode))
                                /*
                                 * else If CTRL command, and he's not in
                                 * diag mode and his adapter not in diag mode,
                                 * get his ptr
                                 */
                                cp = cmd->cp;
                        else
                                /*
                                 * Can't log error against controller
                                 */
                                return;
                        errid = ERRID_SDC_ERR3;
                        etype = E_TYPE_C3;
                        log = &(cp->elog);
                        lb = cp->logbook;
                        name = cp->dds.resource_name;
                        break;
                case 0x0401:
                case 0x0402:
                case 0x0403:
                case 0x0404:
                case 0x0407:
                case 0x040C:
                case 0x040D:
                case 0x040E:
                case 0x040F:
                case 0x0410:
                case 0x0413:
                case 0x0414:
                case 0x0415:
                case 0x0416:
                case 0x0418:
                case 0x0419:
                case 0x041D:
                case 0x0422:
                case 0x0424:
                case 0x0425:
                case 0x0480:
                case 0x0481:
                case 0x0484:
                case 0x048A:
                case 0x04FE:
                case 0x04FF:
                case 0x0502:
                case 0x0503:
                case 0x0504:
                        /*
                         * Adapter Hardware Errors
                         */
                        if (ap->diag_mode)
                                /*
                                 * if adapter in diagnostic mode, don't log
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR) {
                                cmd->uec |= 0xF000;
                                errid = ERRID_SDA_ERR2;
                                etype = E_TYPE_A2;
                        } else {
                                errid = ERRID_SDA_ERR1;
                                etype = E_TYPE_A1;
                        }
                        lb = ap->logbook;
                        break;
                case 0xF401:
                case 0xF402:
                case 0xF403:
                case 0xF404:
                case 0xF407:
                case 0xF40C:
                case 0xF40D:
                case 0xF40E:
                case 0xF40F:
                case 0xF410:
                case 0xF413:
                case 0xF414:
                case 0xF415:
                case 0xF416:
                case 0xF418:
                case 0xF419:
                case 0xF41D:
                case 0xF422:
                case 0xF424:
                case 0xF425:
                case 0xF480:
                case 0xF484:
                case 0xF48A:
                case 0xF4FF:
                case 0xF501:
                case 0xF502:
                case 0xF503:
                case 0xF504:
		case 0xF505:	
                        /*
                         * Recovered Adapter Hardware Errors
                         */
                        if (ap->diag_mode)
                                /*
                                 * if adapter in diagnostic mode, don't log
                                 */
                                return;
                        errid = ERRID_SDA_ERR2;
                        etype = E_TYPE_A2;
                        lb = ap->logbook;
                        break;
                default:
                        /*
                         * Catch the rest
                         */
                        cmd->uec = 0x04FE;
#ifdef SD_ERROR_PATH
			ASSERT(cmd->uec != 0x04FE);
#endif
                        if (ap->diag_mode)
                                /*
                                 * if adapter in diagnostic mode, don't log
                                 */
                                return;
                        if (cmd->status & SD_RECOV_ERROR) {
                                /*
                                 * log as temporary unknown system error
                                 */
                                cmd->uec |= 0xF000;
                                errid = ERRID_SDA_ERR4;
                                etype = E_TYPE_A4;
                        } else {
                                /*
                                 * log as permanent unknown system error
                                 */
                                errid = ERRID_SDA_ERR3;
                                etype = E_TYPE_A3;
                        }
                        lb = ap->logbook;
                        break;
        }

        if (etype != FORCED_LOG) {
                last_log = lb[etype].last_log;
        	if ((last_log <= ctime.tv_sec) &&
	           ((ctime.tv_sec - last_log) < SD_ERR_THRESHOLD)) {
			if (sd_log_limit(lb, etype, cmd->uec))
		                /*
		                 * If we've logged this error within the
		                 * last threshold period, its not a forced 
				 * log, and this UEC has been logged the 
				 * limit, don't log again
		                 */
		                return;
		} else 
			/*
			 * else, this error type has not been logged within
			 * the threshold, so clear the UEC history for this
			 * error type
			 */
			for (ii=0; ii < SD_NUM_UEC_HISTORY; ii++)
			    lb[etype].uecs[ii] = 0;
                /*
                 * update log book statistics
                 */
                lb[etype].last_log = ctime.tv_sec;
		sd_shift_uecs(lb, etype, cmd->uec);
        }

        switch (errid) {
           case ERRID_SDA_ERR1:
           case ERRID_SDA_ERR2:
           case ERRID_SDA_ERR3:
           case ERRID_SDA_ERR4:
                /*
                 * Use Adapter Error Log Structure
                 * Notice, that sense data for adapter should already be
                 * filled in
                 */
                ap->elog.error_id = errid;
                ap->elog.unit_err_code = cmd->uec;
                ap->elog.validity = cmd->elog_validity;
                ap->elog.sys_dma_rc = cmd->elog_sys_dma_rc;
                ap->elog.alert_reg = (cmd->alert_tag << 24) | 
				     (cmd->dev_address << 16) | 
				     (cmd->adapter_status << 8) |
				     (cmd->controller_status);
                bcopy(ap->dds.resource_name, ap->elog.resource_name,
                                ERR_NAMESIZE);
                errsave(&(ap->elog), sizeof(struct sd_adap_error_df));
                /*
                 * Zero for next time
                 */
                bzero(&(ap->elog), sizeof(struct sd_adap_error_df));
                break;
           default:
                /*
                 * build detail data
                 */
		if (cmd->type == SD_ADAP_CMD) {
			/*
			 * Use Adapter Error Log Structure
			 * Notice, that sense data for adapter should already be
			 * filled in
			 */
			ap->elog.error_id = errid;
			ap->elog.unit_err_code = cmd->uec;
			ap->elog.validity = cmd->elog_validity;
			ap->elog.sys_dma_rc = cmd->elog_sys_dma_rc;
			ap->elog.alert_reg = (cmd->alert_tag << 24) | 
				     (cmd->dev_address << 16) | 
				     (cmd->adapter_status << 8) |
				     (cmd->controller_status);
			bcopy(ap->dds.resource_name, ap->elog.resource_name,
			      ERR_NAMESIZE);
			errsave(&(ap->elog), sizeof(struct sd_adap_error_df));
			/*
			 * Zero for next time
			 */
			bzero(&(ap->elog), sizeof(struct sd_adap_error_df));

		}
		else {
			log->error_id = errid;
			cmd->req_sns_bytes[20] = (char)((cmd->uec >> 8) & 0x00FF);
			cmd->req_sns_bytes[21] = (char)(cmd->uec & 0x00FF);
			bcopy(name, log->resource_name, ERR_NAMESIZE);
			bcopy(cmd->req_sns_bytes, log->req_sense_data, 
			      SD_REQ_SNS_LENGTH);
			bcopy(&(cmd->mbox_copy.mb8.fields.scsi_cmd),
			      &(log->scsi_command.scsi_cmd), 
			      sizeof(struct sc_cmd));
			log->scsi_command.resvd = 0;
			log->scsi_command.flags = 0;
			if (cmd->cp != NULL)
				log->scsi_command.scsi_id = cmd->cp->dds.target_id;
			else
				ASSERT(FALSE);
			if (cmd->mbox_copy.op_code == SD_SEND_SCSI_OP ) {
				sd_get_scsi_length(log,
						   cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_op_code);
			}
			log->status_validity = cmd->status_validity;
			log->scsi_status = cmd->scsi_status;
			log->general_card_status = cmd->adapter_status;
			errsave(log, sizeof(struct sc_error_log_df));
			bzero(log, sizeof(struct sc_error_log_df));
		}
                break;
        }
}

/* 
 *
 * NAME: sd_get_scsi_length
 *                  
 * FUNCTION:   Determine the length of the scsi command 
 *
 * EXECUTION ENVIRONMENT: 
 *
 *      This routine must be called in the interrupt environment.  It     
 *      can not page fault and is pinned.
 *
 *
 * (DATA STRUCTURES:)  sc_error_log_df - SCSI Error Log Data
 *
 *
 * CALLED BY:
 *
 *      sd_log_error
 *
 * INTERNAL PROCEDURES CALLED:
 * 
 *      None
 *                              
 * EXTERNAL PROCEDURES CALLED:
 *       None
 *
 * (RECOVERY OPERATION:)  None
 *
 *
 * RETURNS:     
 *       	Void
 */                                                                         
void sd_get_scsi_length(
			struct sc_error_log_df *log,
			uchar scsi_op_code)
{
	switch(scsi_op_code) {
	      case 0x00:     		/* Test Unit Ready 	*/
	      case 0x03:     		/* Request Sense 	*/
	      case 0x04:     		/* Format Unit  	*/
	      case 0x07:      		/* Reassign Blocks	*/
	      case 0x08:      		/* Read			*/
	      case 0x0a:      		/* Write		*/
	      case 0x12:      		/* Inquiry		*/
	      case 0x15:      		/* Mode Select		*/
	      case 0x16:     		/* Reserve		*/
	      case 0x17:      		/* Release		*/
	      case 0x1a:     		/* Mode Sense		*/ 
	      case 0x1b:     		/* Start/Stop Unit	*/ 
	      case 0x1c:      		/* Receive Diagnostic	*/
	      case 0x1d:     		/* Send Diagnostics	*/
		log->scsi_command.scsi_length = 0x06;
		break;

	      case 0x25:     		/* Read Capacity	*/
	      case 0x28:      		/* Read (10)		*/
	      case 0x2a:      		/* Write(10)		*/
	      case 0x2e:      		/* Write & Verify	*/
	      case 0x2f:      		/* Verify		*/
	      case 0x3b:      		/* Write Buffer		*/
	      case 0x41:     		/* Write Same		*/
		log->scsi_command.scsi_length = 0x0a; /* length of 10 */
		break;
	      default:
		log->scsi_command.scsi_length = 0x0c; /* length of 12 */
	}
	return;
}
/*
 * NAME: sd_log_dasd_err
 *
 * FUNCTION: Logs DASD specific errors against a Serial DASD Subsystem
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - Controller Information Structure
 *                      sc_error_log_df - SCSI Error Log Data
 *
 * RETURNS:     Void.
 */
void sd_log_dasd_err(
struct sd_dasd_info *dp,
ushort  uec)
{
        struct sd_ctrl_info *cp;
        struct sd_logbook *lb;
        uint    errid;
        struct timestruc_t ctime;
        ulong   last_log;
        uchar   etype;
	char	ii;

        /*
         * get the current time
         */
        curtime(&ctime);

        cp = dp->cp;
        if ((dp->diag_mode) || (cp->diag_mode) || (dp->ap->diag_mode))
                /*
                 * Can't log error against dasd
                 */
                return;

        switch (uec) {
                case 0x0101:
                case 0x0111:
                case 0x0121:
                case 0x0131:
                case 0x0141:
                case 0x0391:
                case 0x03A3:
                case 0x03A4:
                case 0x03A5:
                case 0x03A6:
                case 0x03A7:
                case 0x03C2:
                case 0x03F0:
	        case 0x0800:
	        case 0x0801:
	        case 0x0802:
	        case 0x0805:
	        case 0x0806:
                        /*
                         * D A S D  HARDWARE ERROR
                         */
                        /*
                         * else, log as physical volume hardware
                         * error
                         */
                        errid = ERRID_DISK_ERR2;
                        etype = E_TYPE_D2;
                        lb = dp->logbook;
                        break;
	        case 0xF802:
                        /*
                         * D A S D  RECOVERED ERROR
                         */
                        /*
			 * if this was a recovered error, log as
			 * recovered
			 */
			errid = ERRID_DISK_ERR4;
			etype = E_TYPE_D4;
                        lb = dp->logbook;
                        break;
		case 0x0000:
                case 0x0281:
                case 0x0282:
                case 0x0283:
                case 0x0284:
                case 0x0285:
                case 0x0286:
                case 0x0288:
                case 0x0290:
                case 0x0291:
                case 0x03A0:
                case 0x03A1:
		case 0x0803:
                        /*
                         * D A S D  MEDIA ERROR
                         */
                        /*
                         * else, log as physical volume media
                         * error
                         */
                        errid = ERRID_DISK_ERR1;
                        etype = FORCED_LOG;
                        lb = dp->logbook;
                        break;
                case 0x0321:
                case 0x0322:
                case 0x0324:
                case 0x0351:
                case 0x0353:
                        /*
                         * Controller/DASD Link Errors
                         */
                        errid = ERRID_SDC_ERR1;
                        etype = E_TYPE_C1;
                        lb = cp->logbook;
                        break;
                case 0x0301:
                case 0x0323:
                case 0x0380:
                case 0x0381:
                case 0x03C3:
                case 0x0500:
                case 0x0501:
                case 0x0505:
                        /*
                         * Controller Hardware Errors
                         */
                        errid = ERRID_SDC_ERR2;
                        etype = E_TYPE_C2;
                        lb = cp->logbook;
                        break;
                case 0xF380:
                case 0xF381:
                case 0xF382:
                case 0xF3FF:
                        /*
                         * Recovered Controller Hardware Errors
                         */
                        errid = ERRID_SDC_ERR3;
                        etype = E_TYPE_C3;
                        lb = cp->logbook;
                        break;
                default:
                        /*
                         * All others, log as microcode/software error
                         */
                        errid = ERRID_SDM_ERR1;
                        etype = E_TYPE_M1;
                        lb = cp->logbook;
                        break;
        }
        if (etype != FORCED_LOG) {
                last_log = lb[etype].last_log;
        	if ((last_log <= ctime.tv_sec) &&
	           ((ctime.tv_sec - last_log) < SD_ERR_THRESHOLD)) {
			if (sd_log_limit(lb, etype, uec))
		                /*
		                 * If we've logged this error within the
		                 * last threshold period, its not a forced 
				 * log, and this UEC has been logged the 
				 * limit, don't log again
		                 */
		                return;
		} else 
			/*
			 * else, this error type has not been logged within
			 * the threshold, so clear the UEC history for this
			 * error type
			 */
			for (ii=0; ii < SD_NUM_UEC_HISTORY; ii++)
				lb[etype].uecs[ii] = 0;
                /*
                 * update log book statistics
                 */
                lb[etype].last_log = ctime.tv_sec;
		sd_shift_uecs(lb, etype, uec);
        }
        /*
         * Determine whether to log against DASD or controller
         */
        switch ( errid ) {
                case ERRID_SDM_ERR1:
                case ERRID_SDC_ERR1:
                case ERRID_SDC_ERR2:
                case ERRID_SDC_ERR3:
                        cp->elog.error_id = errid;
                        bcopy(cp->dds.resource_name, cp->elog.resource_name,
                                ERR_NAMESIZE);
			/*
			 * Notice: we use the DASD sense data
			 */
                        dp->sense_buf[20] = (char)((uec >> 8) & 0x00FF);
                        dp->sense_buf[21] = (char)(uec & 0x00FF);
                        bcopy(dp->sense_buf, cp->elog.req_sense_data, 128);
                        errsave(&(cp->elog), sizeof(struct sc_error_log_df));
                        bzero(&(cp->elog), sizeof(struct sc_error_log_df));
                        break;
                case ERRID_DISK_ERR1:
                case ERRID_DISK_ERR2:
                case ERRID_DISK_ERR4:
                        dp->elog.error_id = errid;
                        dp->elog.reserved2 = dp->dds.segment_cnt;
                        dp->elog.reserved3 = dp->dds.byte_count;
                        bcopy(dp->dds.resource_name, dp->elog.resource_name,
                                ERR_NAMESIZE);
                        dp->sense_buf[20] = (char)((uec >> 8) & 0x00FF);
                        dp->sense_buf[21] = (char)(uec & 0x00FF);
                        bcopy(dp->sense_buf, dp->elog.req_sense_data, 128);
                        errsave(&(dp->elog), sizeof(struct sc_error_log_df));
                        bzero(&(dp->elog), sizeof(struct sc_error_log_df));
                        break;
        }
}

/*
 * NAME: sd_log_ctrl_err
 *
 * FUNCTION: Logs Controller specific errors against a Serial DASD Subsystem
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_ctrl_info    - Controller Information Structure
 *                      sc_error_log_df - SCSI Error Log Data
 *
 * RETURNS:     Void.
 */
void sd_log_ctrl_err(
struct sd_ctrl_info *cp,
ushort  uec)
{
        uint    errid;
        struct timestruc_t ctime;
        struct sd_logbook *lb;
        ulong   last_log;
        uchar   etype;
	char	ii;

        /*
         * get the current time
         */
        curtime(&ctime);

        if ((cp->diag_mode) || (cp->ap->diag_mode))
                /*
                 * Can't log error against controller
                 */
                return;

        lb = cp->logbook;

        switch (uec) {
                case 0x0321:
                case 0x0322:
                case 0x0324:
                case 0x0351:
                case 0x0353:
                        /*
                         * Controller/DASD Link Errors
                         */
                        errid = ERRID_SDC_ERR1;
                        etype = E_TYPE_C1;
                        break;
                case 0x0301:
                case 0x0323:
                case 0x0380:
                case 0x0381:
                case 0x03C3:
                case 0x0500:
                case 0x0501:
                case 0x0505:
                        /*
                         * Controller Hardware Errors
                         */
                        errid = ERRID_SDC_ERR2;
                        etype = E_TYPE_C2;
                        break;
                case 0xF380:
                case 0xF381:
                case 0xF382:
                case 0xF3FF:
                        /*
                         * Recovered Controller Hardware Errors
                         */
                        errid = ERRID_SDC_ERR3;
                        etype = E_TYPE_C3;
                        break;
                default:
                        /*
                         * All others, log as microcode/software error
                         */
                        errid = ERRID_SDM_ERR1;
                        etype = E_TYPE_M1;
                        break;
        }
        last_log = lb[etype].last_log;
        if ((last_log <= ctime.tv_sec) &&
           ((ctime.tv_sec - last_log) < SD_ERR_THRESHOLD)) {
		if (sd_log_limit(lb, etype, uec))
	                /*
	                 * If we've logged this error within the
	                 * last threshold period, its not a forced log, and
	                 * this UEC has been logged the limit, don't log again
	                 */
	                return;
	} else 
		/*
		 * else, this error type has not been logged within
		 * the threshold, so clear the UEC history for this
		 * error type
		 */
		for (ii=0; ii < SD_NUM_UEC_HISTORY; ii++)
			lb[etype].uecs[ii] = 0;
        /*
         * update log book statistics
         */
        lb[etype].last_log = ctime.tv_sec;
	sd_shift_uecs(lb, etype, uec);
        /*
         * Use SCSI Error Log Structure
         */
        cp->elog.error_id = errid;
        bcopy(cp->dds.resource_name, cp->elog.resource_name, ERR_NAMESIZE);
        cp->sense_buf[20] = (char)((uec >> 8) & 0x00FF);
        cp->sense_buf[21] = (char)(uec & 0x00FF);
        bcopy(cp->sense_buf, cp->elog.req_sense_data, 128);
        errsave(&(cp->elog), sizeof(struct sc_error_log_df));
        bzero(&(cp->elog), sizeof(struct sc_error_log_df));
}

/*
 * NAME: sd_log_adap_err
 *
 * FUNCTION: Logs errors reported as an adapter status code
 *           against a Serial DASD Subsystem
 *           Note that these may be caused by a controller problem
 *           hence some of them are logged agaist the controller
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter Information Structure
 *                      sd_adap_error_df - Adapter Error Log Data
 *
 * RETURNS:     Void.
 */
void sd_log_adap_err(
struct sd_adap_info *ap,
ushort  uec,
uchar address)
{
        uint    errid;
        ulong   last_log;
        uchar   etype;
        struct timestruc_t ctime;
        struct sd_logbook *lb;
	char	ii;
	struct sd_ctrl_info *cp;
	
        /*
         * get the current time
         */
        curtime(&ctime);

        if (ap->diag_mode)
                /*
                 * Don't log while in diagnostic mode
                 */
                return;

        switch (uec) {
                case 0x0401:
                case 0x0402:
                case 0x0403:
                case 0x0404:
                case 0x0407:
                case 0x040C:
                case 0x040D:
                case 0x040E:
                case 0x040F:
                case 0x0410:
                case 0x0413:
                case 0x0414:
                case 0x0415:
                case 0x0416:
                case 0x0418:
                case 0x0419:
                case 0x041D:
                case 0x0422:
                case 0x0424:
                case 0x0425:
                case 0x0480:
                case 0x0481:
                case 0x0484:
                case 0x048A:
                case 0x04FE:
                case 0x04FF:
                case 0x0502:
                case 0x0503:
                case 0x0504:
                        /*
                         * Adapter Hardware Errors
                         */
	                lb = ap->logbook;
                        errid = ERRID_SDA_ERR1;
                        etype = E_TYPE_A1;
                        break;
                case 0xF401:
                case 0xF402:
                case 0xF403:
                case 0xF404:
                case 0xF407:
                case 0xF40C:
                case 0xF40D:
                case 0xF40E:
                case 0xF40F:
                case 0xF410:
                case 0xF413:
                case 0xF414:
                case 0xF415:
                case 0xF416:
                case 0xF418:
                case 0xF419:
                case 0xF41D:
                case 0xF422:
                case 0xF424:
                case 0xF425:
                case 0xF480:
                case 0xF484:
                case 0xF48A:
                case 0xF4FF:
                case 0xF501:
                case 0xF502:
                case 0xF503:
                case 0xF504:
                case 0xF505:
                        /*
                         * Recovered Adapter Hardware Errors
                         */
	                lb = ap->logbook;
                        errid = ERRID_SDA_ERR2;
                        etype = E_TYPE_A2;
                        break;
		case 0x0405:
		case 0x0408:
		case 0x040A:
		case 0x040B:
		case 0x0417:
		case 0x0421:
			/* 
			 * Controller Hardware errors
			 */
			cp = ap->ctrllist[SD_TARGET(address)];
			if (cp == NULL)
			    /*
			     * we do not know of this controller
			     */
			    return;
			if (cp->diag_mode)
			    /*
			     * Don't log while in diagnostic mode
			     */
			    return;
	                lb = cp->logbook;
                        errid = ERRID_SDC_ERR2;
                        etype = E_TYPE_C2;
                        break;
		case 0xF405:
		case 0xF408:
		case 0xF40A:
		case 0xF40B:
		case 0xF417:
		case 0xF421:
                        /*
                         * Recovered Controller Hardware Errors
                         */
			cp = ap->ctrllist[SD_TARGET(address)];
			if (cp == NULL)
			    /*
			     * we do not know of this controller
			     */
			    return;
			if (cp->diag_mode)
			    /*
			     * Don't log while in diagnostic mode
			     */
			    return;
	                lb = cp->logbook;
                        errid = ERRID_SDC_ERR3;
                        etype = E_TYPE_C3;
                        break;
		case 0xF411:
		case 0x0411:
			/*
			 * These should be logged against controller
			 * but address is not valid. Should always log in sd_log_error
			 * and never get here.
			 */
			ASSERT((uec & 0xFFF) != 0x0411);
			/*
			 * fall through
			 */
                default:
                        /*
                         * Catch the rest
                         */
                        /*
                         * log as permanent unknown system error
                         */
	                lb = ap->logbook;
                        uec = 0x04FE;
#ifdef SD_ERROR_PATH
			ASSERT(uec != 0x04FE);
#endif
                        errid = ERRID_SDA_ERR3;
                        etype = E_TYPE_A3;
                        break;
        }
        last_log = lb[etype].last_log;
        if ((last_log <= ctime.tv_sec) &&
           ((ctime.tv_sec - last_log) < SD_ERR_THRESHOLD)) {
		if (sd_log_limit(lb, etype, uec))
	                /*
	                 * If we've logged this error within the
	                 * last threshold period, its not a forced log, and
	                 * this UEC has been logged the limit, don't log again
	                 */
	                return;
	} else 
		/*
		 * else, this error type has not been logged within
		 * the threshold, so clear the UEC history for this
		 * error type
		 */
		for (ii=0; ii < SD_NUM_UEC_HISTORY; ii++)
			lb[etype].uecs[ii] = 0;
        /*
         * update log book statistics
         */
        lb[etype].last_log = ctime.tv_sec;
	sd_shift_uecs(lb, etype, uec);
	
	/*
	 * Determine whether to log against adapter or conroller
	 */
	switch (errid) 
	{
	  case ERRID_SDA_ERR1:
	  case ERRID_SDA_ERR2:
	  case ERRID_SDA_ERR3:
	      /*
	       * Use Adapter Error Log Structure
	       * Notice, that sense data for adapter should already be
	       * filled in
	       */
	      ap->elog.error_id = errid;
	      ap->elog.unit_err_code = uec;
	      bcopy(ap->dds.resource_name, ap->elog.resource_name, ERR_NAMESIZE);
	      errsave(&(ap->elog), sizeof(struct sd_adap_error_df));
	      /*
	       * Zero for next time
	       */
	      bzero(&(ap->elog), sizeof(struct sd_adap_error_df));
	      break;
	    case ERRID_SDM_ERR1:
	    case ERRID_SDC_ERR1:
	    case ERRID_SDC_ERR2:
	    case ERRID_SDC_ERR3:
	      /*
	       * Use controller Error Log Structure.
	       * Use adapter sense data.
	       * Notice, that sense data for adapter should already be
	       * filled in.
	       */
	      cp->elog.error_id = errid;
	      bcopy(cp->dds.resource_name, cp->elog.resource_name,
		    ERR_NAMESIZE);
	      cp->elog.req_sense_data[20] = (char)((uec >> 8) & 0x00FF);
	      cp->elog.req_sense_data[21] = (char)(uec & 0x00FF);
	      cp->elog.req_sense_data[22] =
		  (char)((ap->elog.alert_reg >> 24) & 0xFF);
	      cp->elog.req_sense_data[23] = 
		  (char)((ap->elog.alert_reg >> 16) & 0xFF);
	      cp->elog.req_sense_data[24] =
		  (char)((ap->elog.alert_reg >> 8) & 0xFF);
	      cp->elog.req_sense_data[25] = (char)(ap->elog.alert_reg & 0xFF);
	      errsave(&(cp->elog), sizeof(struct sc_error_log_df));
	      bzero(&(cp->elog), sizeof(struct sc_error_log_df));
	      break;
	}
}

/*
 * NAME: sd_log_limit
 *
 * FUNCTION: Determines if a UEC has been logged some number of times    
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_logbook    - Error Log History Information Structure
 *
 * RETURNS:     TRUE  - if a UEC has been logged defined number of times.
 *		FALSE - if the UEC has not yet been logged the defined number
 *			of times.
 */
int	sd_log_limit(
struct sd_logbook *lb,
uchar	etype,
ushort	uec)
{
	int	i,
		count = 0;
	

	for (i=0;i < SD_NUM_UEC_HISTORY;i++) 
		if (lb[etype].uecs[i] == uec)
			/*
			 * Count occurances of this UEC in the history record
			 */
			count++;
	if (count >= SD_LOG_LIMIT)
		return(TRUE);
	else
		return(FALSE);
}

/*
 * NAME: sd_shift_uecs
 *
 * FUNCTION: Updates history record of UEC's logged per error type       
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_logbook    - Error Log History Information Structure
 *
 * RETURNS: Void.
 */
void	sd_shift_uecs(
struct sd_logbook *lb,
uchar	etype,
ushort	uec)
{
	int	i;

	/*
	 * Walk through table shifting UEC's
	 */
	for (i=(SD_NUM_UEC_HISTORY - 2); i >= 0 ;i--) 
		lb[etype].uecs[i+1] = lb[etype].uecs[i];

	/*
	 * Save latest UEC
	 */
	lb[etype].uecs[0] = uec;
		
	return;
}

/*
 * NAME: sd_delay
 *
 * FUNCTION: Delays operation of some device in the Serial DASD Subsystem
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter Information Structure
 *                      sd_ctrl_info 	- Controller Information Structure
 *                      sd_dasd_info 	- DASD Information Structure
 *
 * RETURNS:     Void.
 */
void sd_delay(
struct sd_adap_info *ap,
char	type,
uint	mseconds)                      			/* milli seconds */
{
	struct sd_ctrl_info *cp;
	struct sd_dasd_info *dp;
	struct trb	*t;

	switch(type) {
		case SD_ADAP_CMD:
			/*
			 * Place this adapter in delay status
			 */
			ap->status |= SD_DELAY;
			t = ap->delay_timer;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(ap,sddelay, trc,(char)0,(uint)ap, (uint)type, (uint)mseconds,(uint)ap->status,(uint)0);
#endif
#endif
			break;
		case SD_CTRL_CMD:
			/*
			 * Place this controller in delay status
			 */
			cp = (struct sd_ctrl_info *)ap;
			cp->status |= SD_DELAY;
			t = cp->delay_timer;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(cp->ap,sddelay, trc,(char)1,(uint)cp, (uint)type, (uint)mseconds,(uint)cp->status,(uint)0);
#endif
#endif
			break;
		case SD_DASD_CMD:
			/*
			 * Place this dasd in delay status
			 */
			dp = (struct sd_dasd_info *)ap;
			dp->status |= SD_DELAY;
			t = dp->delay_timer;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                        sd_trc(dp->ap,sddelay, trc,(char)2,(uint)dp, (uint)type, (uint)mseconds,(uint)dp->status,(uint)0);
#endif
#endif
			break;
	}
	
	t->timeout.it_value.tv_sec = mseconds/1000;
	t->timeout.it_value.tv_nsec = (mseconds % 1000) * 1000000;
	if (!(t->flags & T_ACTIVE))
		/*
		 * if timer not already active, then
		 * start
		 */
                tstart(t);
	return;
}

/*
 * NAME: sd_delay_timer
 *
 * FUNCTION: System timer function to restore operation of a device after some
 *		delay.
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level
 *      and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - Adapter Information Structure
 *                      sd_ctrl_info 	- Controller Information Structure
 *                      sd_dasd_info 	- DASD Information Structure
 *
 * RETURNS:     Void.
 */
void sd_delay_timer(
struct trb *t)
{
        struct sd_adap_info *ap;
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        int     opri;

        /*
         * get pointer
         */
        ap = (struct sd_adap_info *)(t->func_data);
	


	if (ap->dds.dev_type == SD_CONTROLLER) {
		/*
		 * This is actually called for a controller
		 */
		cp = (struct sd_ctrl_info *)ap;

		/* 
		 * disable interrupts and on MP machines acquire a spin lock.
		 */
		opri = disable_lock(cp->ap->dds.intr_priority,
				    &(cp->ap->spin_lock));  



#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(cp->ap,delay_tm, trc,(char)0,(uint)ap, (uint)ap->status, (uint)ap->dds.dev_type,(uint)0,(uint)0);
#endif
#endif
		/*
		 * Clear the delay flag 
		 */

		cp->status &= ~SD_DELAY;
		ap = cp->ap;
	} else if (ap->dds.dev_type == SD_DASD) {
		/*
		 * This is actually called for a dasd
		 */
		dp = (struct sd_dasd_info *)ap;

		/* 
		 * disable interrupts and on MP machines acquire a spin lock.
		 */
		opri = disable_lock(dp->ap->dds.intr_priority,
				    &(dp->ap->spin_lock));  


#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(dp->ap,delay_tm, trc,(char)1,(uint)ap, (uint)ap->status, (uint)ap->dds.dev_type,(uint)0,(uint)0);
#endif
#endif
		/*
		 * Clear the delay flag 
		 */
		dp->status &= ~SD_DELAY;
		ap = dp->ap;
	} else {

		/* 
		 * disable interrupts and on MP machines acquire a spin lock.
		 */
		opri = disable_lock(ap->dds.intr_priority,
				    &(ap->spin_lock));  


#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(ap,delay_tm, trc,(char)1,(uint)ap, (uint)ap->status, (uint)ap->dds.dev_type,(uint)0,(uint)0);
#endif
#endif
		/*
		 * Clear the delay flag 
		 */
		ap->status &= ~SD_DELAY;
	}
	/*
	 * Now call start to keep things chugging
	 */
	sd_start(ap);

	unlock_enable(opri,&(ap->spin_lock));

	return;
}
					

    
    
    
    
    
	
    


