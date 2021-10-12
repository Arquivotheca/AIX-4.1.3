static char sccsid[] = "@(#)96  1.24.1.9  src/bos/kernext/disk/sd/sdintr.c, sysxdisk, bos411, 9428A410j 3/16/94 10:02:33";
/*
 * COMPONENT_NAME: (SYSXDISK) Serial Dasd Subsystem Device Driver
 *
 * FUNCTIONS: sd_intr(), sd_process_complete(), sd_process_buf(),
 *            sd_parse_err(), sd_parse_ready_async(), sd_alert_notag(), 
 *            sd_prepare_buf_retry(), sd_fail_buf_cmd(), sd_process_error()
 *            sd_process_scsi_error(), sd_process_sense(),
 *            sd_process_reset()
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

#include <sys/sd.h>
#include <sys/errno.h>
#include <sys/ddtrace.h>
#include <sys/trchkid.h> 
#include <sys/signal.h>
#include <sys/ddconc.h>

/*
 * Declare any global pinned variables
 */
struct  sd_adap_info *sd_adap_table[SD_ADAP_TBL_SIZE];
struct  sd_ctrl_info *sd_ctrl_table[SD_CTRL_TBL_SIZE];
struct  sd_dasd_info *sd_dasd_table[SD_DASD_TBL_SIZE];
struct  watchdog        activity_timer;
uchar   sd_open_adaps = 0;
uchar   sd_open_ctrls = 0; /* number of opened ctrls in system */
uchar   sd_open_dasd = 0; /* number of opened dasd in system */
char    sd_inited_adaps = 0; /* number of inited adapters in system */
char    sd_inited_ctrls = 0; /* number of inited ctrls in system */
char    sd_inited_dasd = 0; /* number of inited dasd in system */
lock_t  sd_global_lock = LOCK_AVAIL;    /* global lock word */
struct  intr    *sd_epow_ptr;



#ifdef DEBUG

uint *sd_trace;


/*
 ******  strings for Debug  *******
 */

char     *strategy      = "STRATEGY";
char     *insertq       = "INSERT_Q";
char     *dequeue       = "DEQUEUE ";
char     *start         = "START   ";
char     *coales        = "COALESCE";
char     *relocate      = "RELOCATE";
char     *startcmd      = "STARTCMD";
char     *interrupt     = "INTR    ";
char     *complete      = "COMPLETE";
char     *error         = "ERROR   ";
char     *sdiodone      = "IODONE  ";
char     *inadapchain   = "IN_A_CHN";
char     *indasdchain   = "IN_D_CHN";
char     *outdasdchain  = "OUT_DCHN";
char     *outadapchain  = "OUT_ACHN";
char     *lasttag       = "W_L_TAG ";
char     *alertnotag    = "ALERTNOT";
char     *bufretry      = "BUFRETRY";
char     *failbuf       = "FAIL_BUF";
char     *scsierror     = "SCSI_ERR";
char     *processbuf    = "BUFCMPLT";
char     *processreset  = "RESETCMP";
char     *waitreset     = "WAIT_RST";
char     *qcmd          = "Q_CMD   ";
char     *dqcmd         = "D_Q_CMD ";
char     *cmdalloc      = "CMDALLOC";
char     *cmdfree       = "FREE_CMD";
char     *cmdfail       = "FAIL_CMD";
char     *mballoc       = "MB_ALLOC";
char     *mbfree        = "FREE_MB ";
char     *cmdtimer      = "CMD_TMR ";
char     *ioctltimer    = "IOCTLTMR";
char     *flushadap     = "FLSHADAP";
char     *failadap      = "FAILADAP";
char     *faildasd      = "FAILDASD";
char     *reset_quiesce = "RST_QUI ";
char     *haltadap      = "HALTADAP";
char     *verify        = "VERIFY  ";
char     *async         = "ASYNCEVT";
char     *piorecov      = "PIORECOV";
char     *activity      = "ACTIVITY";
char     *qrytimer      = "QRY_TMER";
char     *qrydev        = "QUERYDEV";
char     *startunit     = "STRTUNIT";
char     *testunit      = "TESTUNIT";
char     *reqsns        = "REQSNS  ";
char     *reserve       = "RESERVE ";
char     *release       = "RELEASE ";
char     *fence         = "FENCE   ";
char     *modesense     = "M_SENSE ";
char     *modeselect    = "M_SELECT";
char     *readcap       = "READ_CAP";
char     *inquiry       = "INQUIRY ";
char     *setparms      = "SETPARMS";
char     *scsisense     = "SCSISENS";
char     *notcws   	= "NO_TCWS ";
char     *tcwalloc   	= "TCWALLOC";
char     *tcwfree   	= "TCW_FREE";
char     *sddelay   	= "DELAY   ";
char     *message       = "MESSAGE ";	
char     *delay_tm   	= "DELAY_TM";
char     *entry         = " IN";	/* Entry into routine                */
char     *exit          = " EX";	/* Exit from routine                 */
char     *trc           = " TR";	/* Trace point in routine            */
#endif



/*
 * NAME: sd_intr
 *
 * FUNCTION: Interrupt Handler for Serial DASD Subsytem Device Driver
 *
 * EXECUTION ENVIRONMENT: This routine is called by the system interrupt
 *      handler upon detecting a hardware interrupt at this level.  This
 *      routine runs on the interrupt level and can not page fault.
 *
 * (NOTES:) Possible operations : Determines if the adapter caused an interrupt,
 *      and if so processed that interrupt
 *
 * (RECOVERY OPERATION:) If a PIO error occurs while trying to read the
 *                       completion register, INTR_FAIL is returned since
 *                       it is not know whether it was the adapters interrupt.
 *                       If a PIO error occures while trying to read the
 *                       alert register, INTR_SUCC is returned since it was
 *                       this adapters interrupt, and recovery is left up to
 *                       system timers.  If a PIO error occurs while trying
 *                       to start the next command, then the adapter is reset.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - adapter info structure
 *                      sd_ctrl_info    - controller info structure
 *                      sd_dasd_info    - dasd info structure
 *                      sd_cmd          - command structure
 *                      intr            - interrupt structure
 *
 * RETURNS:
 *           INTR_SUCC         - successful completion of interrupt
 *           INTR_FAIL         - unsuccessful completion of interrupt
 */

int     sd_intr(
struct intr *is)
{
        struct sd_intr *sd_is;
        struct sd_adap_info *ap;
        struct sd_cmd   *cmd;
        union {
                uint    word;
                uchar   byte[4];
              }                 cmpl_reg;

        uchar			 byte,
				 tag;
        uint                    base;
        int                     prc = 0;
	int			opri;

        /*
         * recast to get pointer to our interrupt structure
         */
        sd_is = (struct sd_intr *)is;
        /*
         * get adapter pointer
         */
        ap = sd_is->ap;


	DDHKWD5(HKWD_DD_SERDASDD, DD_ENTRY_INTR, 0, ap->devno, 0, 0, 0, 0);

        if (!ap->opened) {
                /*
                 * if this adapter not open, then can't be my
                 * interrupt (interrupts enabled at open, disabled at close)
                 */
                DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_INTR, INTR_FAIL, ap->devno);
                return(INTR_FAIL);
	}

	opri = disable_lock(ap->dds.intr_priority,&(ap->spin_lock));
	
        base = (uint)BUSIO_ATT(ap->dds.bus_id,ap->dds.base_addr);

        /*
         * read the adapter COMPLETION REGISTER
         * Notice: volatile flag set
         */
        prc = BUS_GETLX((long *)(base + SD_CMPL), (long *)&cmpl_reg.word);
        if (prc) {
                prc = sd_pio_recov(ap, prc, (uchar)GETL,
                        (void *)(base + SD_CMPL), (long)&cmpl_reg.word,
                        (uchar)VOLATILE);
                if (prc) {
                        /*
                         * if PIO error, rely on timers to clean up
                         */
                        BUSIO_DET(base);
                	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_INTR, INTR_FAIL, 
				ap->devno);
			unlock_enable(opri,&(ap->spin_lock)); 

                        return( INTR_FAIL);
                }
        }
        if (!cmpl_reg.word) {
                /*
                 * no tags or anything, must not be our interrupt
                 */
                BUSIO_DET(base);
                DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_INTR, INTR_FAIL, ap->devno);
		unlock_enable(opri,&(ap->spin_lock)); 
                return( INTR_FAIL);
        }

        i_reset(&(ap->sdi.intr_st));    /* reset system IOCC intrpt logic */

#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,interrupt, entry, (char)0, (uint)ap, (uint)cmpl_reg.word ,(uint)0,(uint)0,(uint)0);
#endif
#endif
        byte = 0;
        do {
                /*
                 * for each byte of the completion register ...
                 */
                /*
                 * get the tag associated with this completion
                 */
                tag = (uchar) cmpl_reg.byte[byte];
                if (tag != SD_CMPL_ERROR) {
                        /*
                         * if No indication of error, then process good
                         * completion first.  Notice that having good
                         * completion processing first streamlines good path
                         * where performance is critical.  Performance is
                         * improved by avoiding cache misses that would occur
                         * if we had to branch further down in the code to
                         * process good completion
                         */
                        if ( tag == SD_NO_MORE_TAGS ) {
                                /*
                                 * Nothing more to process, so set byte to last
                                 * one causing us to fall out of the do loop
                                 */
                                byte = SD_CMPL_REG_SIZE;
                        } else {
                                /*
                                 * Else Valid tag with no error
                                 */
			   	if (tag > SD_MAX_TAG) {
                                        /*
                                         * Invalid tag
                                         */
                                        sd_alert_notag(ap,(uchar)SD_QUIESCE_A,
						(uchar)0);
					sd_log_adap_err(ap, (ushort)0xF419,0);
                                        continue;
                                }

                                /*
                                 * get pointer to cmd struct
                                 */
                                cmd = (struct sd_cmd *)ap->cmd_map[tag];
                                if ((cmd == NULL) ||
					(!(cmd->status & SD_ACTIVE))) {
                                        /*
                                         * if no command or command not active
                                         */
					if (!(ap->status &
                                                SD_QUIESCE_PENDING)) {
                                                /*
                                                 * As long as a quiesce is not
                                                 * already in progress, log
                                                 * this
                                                 */
						sd_alert_notag(ap,
							       (uchar)SD_QUIESCE_A,
							       (uchar)0);
						sd_log_adap_err(ap, 
							       (ushort)0xF419,0);
					}
                                        continue;
                                }
                                /*
                                 * process good completion of this command
                                 */
                                sd_process_complete(cmd, (char)FALSE);
                        } /* else valid tag */
                } else {
			/*
			 * Some form of error, so parse and handle...
			 */
			if (sd_parse_err(ap, base)) 
				/*
				 * Must have had PIO error reading alert,
				 * break out
				 */
				break;
		}
        } while (++byte < SD_CMPL_REG_SIZE);


        /*
         * keep us chugging
         */
        sd_start(ap);

        BUSIO_DET(base);
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,interrupt, exit, (char)0, (uint)ap, (uint)0,(uint)0, (uint)0,(uint)0);
#endif
#endif
	DDHKWD1(HKWD_DD_SERDASDD, DD_EXIT_INTR, INTR_SUCC, ap->devno);

	unlock_enable(opri,&(ap->spin_lock)); 

        return(INTR_SUCC);
}

/*
 * NAME: sd_process_complete
 *
 * FUNCTION: Processes good completion of a command
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level by
 *      sd_intr or sd_process_sense, and can not page fault.
 *
 * (NOTES:) Possible operations : For the completed command, this routine
 *      performs DMA cleanup on the Mailbox and any data transfers, frees
 *      the assigned mailbox, any other resources, and frees the command
 *
 * (RECOVERY OPERATION:) If the DMA cleanup fails, then sd_process_error is
 *      called to process this command as having failed.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command Structure
 *                      sd_dasd_info    - DASD info structure
 *                      sd_adap_info    - Adapter info structure
 *
 * RETURNS:     Void.
 */

void sd_process_complete(
struct sd_cmd *cmd,
char    skip)
{
        int     rc;
        struct  sd_dasd_info *dp;
        struct  sd_adap_info *ap;
        uint    serial_num1,
                serial_num2;
	uchar	address;
	ushort  fencepos;         /* holds returned fence position indicator*/
	struct  sd_fence_info *fence_buf;
	
        /*
         * Whatever the command was, it completed with no errors,
         * so process completion here ...
         */
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,complete, entry,(char)0, (uint)cmd, (uint)skip, 
	       (uint)cmd->cmd_info, (uint)cmd->status,(uint)0);
#endif
#endif

        ap = cmd->ap;

        if (!skip) {
                /*
                 * if not supposed to skip this part ....example:
                 * if a command was previously checked, we have already
                 * done this, now we're asked to complete the command
                 * as successful.
                 */

                /*
                 * DMA cleanup on Mailbox and any data transfer
                 */
                rc = sd_dma_cleanup(cmd,(uchar)0);
                if (rc) {
                        /* 
			 * Setup to LOG system DMA error
			 */
			cmd->uec = 0x040C;
			cmd->elog_validity |= SD_DMA_RC_VALID; 
			cmd->elog_sys_dma_rc = rc;
                        cmd->status |= SD_LOG_ERROR;
                        cmd->b_error = EIO;
                        cmd->retries = 3;
                        cmd->erp = SD_NONE;
                        sd_process_error(ap, cmd, (char)TRUE, (char)SD_QUEUE);
                        return;
                }

                /*
                 * Free any TCW's or STA's
                 */
                sd_TCW_dealloc(cmd);
                sd_STA_dealloc(cmd);

                /*
                 * free this commands mailbox
                 */
                sd_free_MB(cmd, (char)SD_USED);
        }

	if (cmd->status & SD_LOG_ERROR)
		/*
		 * if this command previously had an error, make sure 
		 * status reflects recovered error
		 */
		cmd->status |= SD_RECOV_ERROR;
        switch(cmd->cmd_info) {
                case SD_NORMAL_PATH:
                        /*
                         * This command came through normal path
                         * (bufs associated with it)
                         */
			if (cmd->status & SD_RETRY) {
				/*
				 * if this was a retried command, then stop
				 * the cmd timer if it was pointing to this cmd.
				 */
			    if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
				w_stop(&(cmd->dp->cmd_timer.watch));
			}
                        /*
                         * complete buf processing
                         */
                        sd_process_buf(cmd);
                        break;
                case SD_IOCTL:
			/*
			 * No Status -- Command Completed Successfully
			 */
			cmd->status_validity = 0;
			cmd->adapter_status = 0;
                        /*
                         * This command originated from our ioctl routine,
                         * so simply wake him up
                         */
                        if (cmd->status & SD_TIMEDOUT) {
                                /*
                                 * if this command has just now come back,
                                 * but the ioctl routine already timed it
                                 * out, simply free the command
                                 */
                                sd_free_cmd(cmd);
                        } else
                                /*
                                 * else wakeup the ioctl routine
                                 */
                                switch(cmd->type) {
                                        case SD_ADAP_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->ap->ioctl_timer.watch);
                                           cmd->ap->ioctl_intrpt = 0;
                                           e_wakeup(&cmd->ap->ioctl_event);
                                           break;
                                        case SD_CTRL_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->cp->ioctl_timer.watch);
                                           cmd->cp->ioctl_intrpt = 0;
                                           e_wakeup(&cmd->cp->ioctl_event);
                                           break;
                                        case SD_DASD_CMD:
                                           /*
                                            * stop the timer, clear intrpt
                                            * flag, and wakeup the event
                                            */
                                           w_stop(&cmd->dp->ioctl_timer.watch);
                                           cmd->dp->ioctl_intrpt = 0;
                                           e_wakeup(&cmd->dp->ioctl_event);
                                           break;
                                }
                        break;
                case    SD_REASSIGN:
                        /*
                         * This command was an internal reassign block
                         */
                        dp = cmd->dp;
			dp->status &= ~SD_REASSIGN_PENDING;
                        /*
                         * stop the cmd timer if appropriate
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
                        break;
                case    SD_REQSNS:
                        /*
                         * This command was an internal request sense command
                         * so call process sense
                         */
                        if (cmd->type == SD_DASD_CMD) {
			        if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
				    w_stop(&(cmd->dp->cmd_timer.watch));
				cmd->dp->status &= ~SD_REQ_SNS_PENDING;
                                sd_process_sense(cmd->dp,cmd->dp->cp,
                                        (char)SD_DASD_CMD);
                        } else {
			        if ((struct sd_cmd *)cmd->cp->cmd_timer.pointer == cmd)
				    w_stop(&cmd->cp->cmd_timer.watch);
				cmd->cp->status &= ~SD_REQ_SNS_PENDING;
                                sd_process_sense((struct sd_dasd_info *)NULL,
                                        cmd->cp,(char)SD_CTRL_CMD);
                        }
                        sd_free_cmd(cmd);
                        break;
                case    SD_START_UNIT:
                        /*
                         * This command was an internal Start Unit
                         */
                        dp = cmd->dp;
			/*
			 * clear sick flag, since we know this will be the
			 * first successful completion after a device has
			 * been sick
			 */
			dp->sick = FALSE;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
                        sd_test_unit_ready(dp,SD_TEST_UNIT_READY);
                        break;
                case    SD_TEST_UNIT_READY:
                        /*
                         * This command was an internal Test Unit Ready
                         */
                        dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);

			if ((dp->cp->dds.fence_enabled) &&
			    (dp->fence_data_valid))
			    sd_fence(dp,SD_FENCE_POS_CHECK);
			else
			    sd_reserve(dp,SD_RESERVE);
                        break;
		case    SD_FENCE_POS_CHECK:
			/*
			 * This command was an internal fence position 
			 * check.
			 * We verify here that the fence position is still
			 * the same as it was when the fence data was set up.
			 * If it has changes, we must lock out the dasd until
			 * the links are swapped back or the dasd is closed
			 * and re-opened.
			 */
			dp = cmd->dp;
			fence_buf = (struct sd_fence_info *)&dp->sense_buf;
			fencepos = (ushort)fence_buf->fence_posn;
			if (fencepos != dp->fence_host_position)
			{
			    /*
			     * oops.....user swapped cables over...lock him out
			     * til he swaps them back.
			     */
			    cmd->uec = 0x0506;
			    cmd->status |= SD_LOG_ERROR;
			    cmd->b_error = EXDEV;
			    /*
			     * failing a verify sequence command leads to
			     * the dasd being failed.
			     */
			    sd_fail_cmd(cmd,(char)FALSE);
			}
			else
			{
			    /*
			     * stop the cmd timer
			     */
			    if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
				w_stop(&dp->cmd_timer.watch);
			    sd_free_cmd(cmd);
			    sd_fence(dp,SD_FENCE);
			}		
			break;
		case    SD_FENCE:
			/*
			 * This command was an internal fence command
			 */
			dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
			sd_reserve(dp,SD_RESERVE);
			break;
		case    SD_RST_QSC:
                        /*
                         * This command was an internal reset/quiesce command
                         */
                        sd_process_reset(cmd);
                        break;
                case    SD_QRYDEV:
                        /*
                         * This command was an internal query device command
                         * so just stop the query timer, increment the number
			 * of queries complete and free command
                         */
                        w_stop(&(cmd->dp->query_timer.watch));
			cmd->dp->query_count++;
                        sd_free_cmd(cmd);
                        break;
                case    SD_SPECIAL:
                        /*
                         * This command was an special internal command
                         */
                        if(cmd->type == SD_ADAP_CMD) {
                                /*
                                 * the only special command used for
                                 * adapters is set adapter parms,
                                 * stop the timer, clear intrpt
                                 * flag, and wakeup the event if
                                 * from open, else if result of
                                 * adapter reset, keep state machine
                                 * chugging by calling sd_verify
                                 */
			         if ((struct sd_cmd *)cmd->ap->cmd_timer.pointer == cmd)
				     w_stop(&cmd->ap->cmd_timer.watch);
                                 cmd->ap->adap_result = 0;
                                 cmd->ap->reset_count = 0;
                                 if (cmd->ap->reset_result == SD_RESET_A) {
                                        sd_verify(cmd->ap, (char)SD_ADAP_CMD);
                                        cmd->ap->reset_result = 0;
                                 } 
				 e_wakeup(&cmd->ap->adap_event);
                        }
                        sd_free_cmd(cmd);
                        break;
                case    SD_RESERVE:
                        /*
                         * This command was a reserve
                         */
                        dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
			dp->m_sense_status = SD_SENSE_CHANGEABLE;
                        sd_mode_sense(dp);
                        break;
                case    SD_MODE_SENSE:
                        /*
                         * This command was a Mode sense
                         */
                        dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
			if (dp->m_sense_status == SD_SENSE_CHANGEABLE) {
				/*
				 * if this was the sense of changeable 
				 * attributes, then prepare to sense current
				 * attributes. First, copy changeable data
				 * into separate data buffer, and format the
				 * data
				 */
        			bcopy (dp->sense_buf, dp->ch_data, 256);
				sd_format_mode_data(dp->ch_data, &dp->ch, 
					(int) (dp->ch_data[0]+1));
				dp->m_sense_status = SD_SENSE_CURRENT;
                        	sd_mode_sense(dp);
			} else {
				/*
				 * this was the sense of current data, so
				 * format the data and then determine if
				 * a select is necessary
				 */
				sd_format_mode_data(dp->sense_buf, &dp->cd,
					(int) (dp->sense_buf[0]+1));
                        	if (sd_mode_data_compare(dp))
                                	/*
	                                 * if a mode select is necessary
	                                 */
	                                sd_mode_select(dp);
	                        else
	                                /*
	                                 * bypass the mode select
	                                 */
	                                sd_inquiry(dp);
			}
                        break;
                case    SD_MODE_SELECT:
                        /*
                         * This command was a Mode Select
                         */
                        dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
                        sd_inquiry(dp);
                        break;
                case    SD_READ_CAPACITY:
			/*
			 * This is the end of the verify sequence, so clear his
			 * status
			 */
			cmd->dp->status &= ~SD_VERIFY_PENDING;
			/* Fall through */	
                case    SD_STOP_UNIT:
                case    SD_RELEASE:
                        /*
                         * This command was a Read Capacity, stop unit,or
                         * release, these are all terminal points of our
                         * state machine, so wakeup the process waiting
                         */
                        dp = cmd->dp;
                        /*
                         * stop the cmd timer
                         */
                        dp->restart_count = 0;
                        dp->seq_not_done = FALSE;
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        sd_free_cmd(cmd);
                        dp->dasd_result = 0;
                        e_wakeup(&dp->dasd_event);
                        break;
                case    SD_INQUIRY:
                        /*
                         * This command was an inquiry used for device
                         * verification
                         */
                        dp = cmd->dp;
                        /*
                         * stop the timer
                         */
			if ((struct sd_cmd *)dp->cmd_timer.pointer == cmd)
			    w_stop(&dp->cmd_timer.watch);
                        if (!dp->serial_num_valid) {
                                /*
                                 * If the serial number has not been stored
                                 * yet, then this is the first ever open to
                                 * this disk.  After the serial_num_valid
                                 * flag is set here, it is never cleared!!
                                 * then save the DE FRU (Disk Enclosure) Serial
                                 * Number of the removable DASD module for
                                 * later device verification. This field is
                                 * contained in bytes 36 - 43 of the inquiry
                                 * data.
                                 */
                                dp->serial_num1 = (dp->sense_buf[36] << 24) |
                                                  (dp->sense_buf[37] << 16) |
                                                  (dp->sense_buf[38] << 8) |
                                                  (dp->sense_buf[39] );
                                dp->serial_num2 = (dp->sense_buf[40] << 24) |
                                                  (dp->sense_buf[41] << 16) |
                                                  (dp->sense_buf[42] << 8) |
                                                  (dp->sense_buf[43] );
                                dp->serial_num_valid = TRUE;
                                sd_free_cmd(cmd);
                                /*
                                 * Keep our startup sequence state
                                 * machine chugging
                                 */
                                sd_read_cap(dp,FALSE);
                        } else {
                                /*
                                 * This is from device verification, so see
                                 * if this one is the same
                                 */
                                serial_num1 = (dp->sense_buf[36] << 24) |
                                              (dp->sense_buf[37] << 16) |
                                              (dp->sense_buf[38] << 8) |
                                              (dp->sense_buf[39] );
                                serial_num2 = (dp->sense_buf[40] << 24) |
                                              (dp->sense_buf[41] << 16) |
                                              (dp->sense_buf[42] << 8) |
                                              (dp->sense_buf[43] );
                                if ((serial_num1 == dp->serial_num1) &&
                                    (serial_num2 == dp->serial_num2)) {
					/*
					 * it is, so free command and continue
					 */
                                        sd_free_cmd(cmd);
                                        /*
                                         * Keep our startup sequence state
                                         * machine chugging
                                         */
                                        sd_read_cap(dp,FALSE);
                                } else {
                                        /*
                                         * Uh-Oh, not what we thought we were
                                         * talking to, so LOCK this DASD from
                                         * taking any more request, he must be
                                         * unconfigured
                                         */
                                        dp->status = SD_LOCKED;
					address = 
						cmd->mbox_copy.mb7.dev_address;
					/*
					 * Log an error so we know this
					 * happened.
					 */
					cmd->uec = 0x0506;
					cmd->status |= SD_LOG_ERROR;
					sd_log_error(cmd);
					sd_free_cmd(cmd);
                                        /*
                                         * Fail anything pending for this DASD
                                         */
                                        sd_fail_dasd(dp);
                                        /*
                                         * queue up event to config
                                         */
					if ((!dp->diag_mode) && 
					    (!dp->cp->diag_mode) && 
					    (!dp->ap->diag_mode))
						/*
						 * As long as this dasd and
						 * his ctrl and adapter are
						 * not in diagnostic mode
						 */
                                        	sd_async_event(dp->ap, address,
							(uchar)SD_CONFIG,
							       SIGTRAP);
					dp->dasd_result = EIO;
					e_wakeup(&dp->dasd_event);
                                }
                        }
                        break;
		case SD_REFRESH: 
		case SD_LOCK: 
		case SD_UNLOCK: 
		case SD_TEST: 
			/*
			 * Make sure b_error is good.
			 */
			cmd->b_error = 0;
			sd_process_conc_cmd(cmd);
			break;
                default:
                        /*
                         * Programming Error ........
                         */
                        ASSERT(FALSE);
        }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(ap,complete, exit,(char)0, (uint)cmd, (uint)0 , 
	       (uint)0, (uint)0,(uint)0);
#endif
#endif

}

/*
 * NAME: sd_process_buf
 *
 * FUNCTION: Processes good completion of a Normal Path Command (involving
 *      buf structures)
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_process_complete or
 *      sd_process_sense on the interrupt level, and can not page fault.
 *
 * (NOTES:) Possible operations : Completes Buf Commands
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command Structure
 *                      buf             - Buf structure
 *
 * RETURNS:     Void.
 */

void sd_process_buf(
struct sd_cmd *cmd)
{
        struct buf *bp,
                   *next;

#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,processbuf, entry,(char)0, (uint)cmd, (uint)cmd->bp, 
	       (uint)0, (uint)0,(uint)0);
        sd_dptrc(cmd->dp,processbuf, entry,(char)0, (uint)cmd->bp);
#endif
#endif
        /*
         * set interrupted flag to let activity timer know we're
         * trucking along
         */
        cmd->dp->interrupted = TRUE;
        cmd->dp->query_count = 0;
        bp = cmd->bp;
        while (bp != NULL) {
                /*
                 * for each buf we coalesced
                 */
                next = (struct buf *)bp->b_work; /* save next */
                if (cmd->reloc) {
                        /*
                         * if we successfully got the data,
                         * but only after subsystem retries,
                         * then set error to ESOFT
                         */
                        if ((cmd->last_rba_valid) && 
				((bp->b_blkno <= cmd->last_rba) && 
				(bp->b_blkno + (bp->b_bcount / SD_BPS)) > 
				cmd->last_rba)) {
                                /*
                                 * if we set the reloc flag and this
                                 * buf is the cause
                                 */
                                bp->b_resid = bp->b_bcount - ((cmd->last_rba - 
					bp->b_blkno) * SD_BPS);
                                bp->b_flags |= B_ERROR;
                                bp->b_error = ESOFT;
                                cmd->reloc = FALSE;
                        } 
                } 
		if (cmd->first_bp_resid != 0) {
		    /*
		     * We successfully got the data after a
		     * driver retry. ie. We had EMEDIA on the
		     * first try but the second try gave 
		     * good completion.
		     */
		    bp->b_resid = cmd->first_bp_resid;
		    bp->b_flags |= B_ERROR;
		    bp->b_error = ESOFT;
		    cmd->first_bp_resid = 0;
		}    
		    
#ifdef DEBUG
#ifdef SD_GOOD_PATH
                sd_trc(cmd->ap,sdiodone, trc,(char)0, (uint)bp, (uint)bp->b_error , (uint)bp->b_flags, (uint)bp->b_resid,(uint)0);
                sd_dptrc(cmd->dp,sdiodone, trc,(char)0, (uint)bp);
#endif
#endif
		DDHKWD2(HKWD_DD_SERDASDD, DD_SC_IODONE, bp->b_error, bp->b_dev,
			bp);
                iodone(bp);
                bp = next;      /* look at next one */
        }
#ifdef DEBUG
#ifdef SD_GOOD_PATH
        sd_trc(cmd->ap,processbuf, exit,(char)0, (uint)cmd, (uint)0, 
	       (uint)0, (uint)0,(uint)0);
#endif
#endif
        sd_free_cmd(cmd);
}

/*
 * NAME: sd_parse_err
 *
 * FUNCTION: Parses Error Conditions after detection of 0xFF in completion
 *      Register. This is made modular to reduce cache hits when processing
 *	good path.
 *
 * EXECUTION ENVIRONMENT: This routine is called by the sd_intr on the
 *      interrupt level and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - adapter info structure
 *                      sd_ctrl_info    - controller info structure
 *
 * RETURNS:     Void.
 */
int sd_parse_err(
struct sd_adap_info *ap,
uint	base)
{
	struct sd_cmd *cmd;
	struct sd_dasd_info *dp;
	struct sd_ctrl_info *cp;
	int	prc;
        union {
                uint    word;
                uchar   byte[4];
              }              alert_reg;
	uchar	address,
		erp,
		tag,
		queue = SD_QUEUE,
		processed = FALSE;
	ushort	uec = 0x0000;

	/*
	 * Read Alert Register
	 */
	prc = BUS_GETLX((long *)(base + SD_ALRT), (long *)&alert_reg.word);
	if (prc) {
		prc = sd_pio_recov(ap, prc, (uchar)GETL, 
			(void *)(base + SD_ALRT), (long)&alert_reg.word, 
			(uchar) VOLATILE);
		if (prc) 
			/*
			 * if PIO error, we know it was our interrupt,
			 * so return success and rely in timers
			 * to clean up
			 */
			return(prc);
	}

#ifdef DEBUG
#ifdef SD_ERROR_PATH
	sd_trc(ap,interrupt, trc, (char)1, (uint)alert_reg.word, (uint)0, (uint)0, (uint)0,(uint)0);
#endif
#endif
	/*
	 * Get byte 0 of the Alert Register
	 */
	tag =  alert_reg.byte[ 0];
	address = alert_reg.byte[1];     /* only valid for certain errors*/
	switch (tag) {
		/*
                 * Parse possible tag values ...
                 */
		case SD_ALRT_NO_ERR :
			/*
			 *  No adapter Error
			 */
			switch (alert_reg.byte[ 2]) {
				/*
                                 * Parse possible adap status
                                 * with ASYNC
                                 */
                                case SD_UNIT_POWERED_ON :
                                   /*
                                    * Parse the Async Ready Status ...
                                    */
				   sd_parse_ready_async(ap, alert_reg.byte,
					&processed,&erp,&uec);
				   break;
                                case SD_WBUFF_STARTED :
                                   /*
                                    * WRITE BUFFER STARTED
                                    * Halt this ( Quiesce ) controller,
				    * and delay 1 second 
				    * before sending more commands
                                    */
				   cp = ap->ctrllist[SD_TARGET(address)];
				   if (cp != NULL) 
				   {
				       sd_delay((struct sd_adap_info *)cp, 
						(char)SD_CTRL_CMD, (uint)1000);
				   }
                                   erp = SD_QUIESCE_C;
				   break;
				 case SD_ASYNCHRONOUS_EVENT :
				   /*
				    * asyncronous event 
				    * inform kernel extension if registered.
				    */
				   cp = ap->ctrllist[SD_TARGET(address)];
				   if (cp != NULL) {
				       /*
					* we know about this controller
					*/
				       dp = cp->dasdlist[SD_LUN(address)];
				       if (dp != NULL) {
					   /* 
					    * we know about this dasd
					    */
					   if (dp->conc_registered)
					   {
					       /*
						* inform registered kernel
						* extension
						*/
					       (dp->conc_intr_addr)(NULL,
						DD_CONC_RECV_REFRESH,
						alert_reg.byte[3],dp->devno);
					   }
				       }
				       
				   }
				   processed = TRUE;
				   break;
                                default :
                                   /*
                                    * UNKNOWN Adapter Status
                                    */
				   uec = 0xF4FE; /* generic adapter err */
                                   processed = TRUE;
                                   break;
			} /* switch adapter status */
			break;

		case SD_ALRT_ERROR :
                           /*
                            * ADAPTER DETECTED ERROR
                            */
                           tag = 0;     /* assume no tag */
                           switch ( alert_reg.byte[2]) {
                              /*
                               * Parse adapter status
                               */
			      case SD_CMD_TO_ACTIVE_TAG :
                              case SD_INVALID_MB_PTR :
                              case SD_INVALID_TAG :
                              case SD_MB_DMA_FAIL :
			      case SD_DATA_DMA_FAIL :

                                /*
                                 * Reenable DMA
                                 * Quiesce Adapter
                                 */
                                /*
                                 * enable this card's DMA
                                 */
                                ap->pos4 |= SD_DMA_ENABLE;
                                if (sd_write_POS(ap,SD_POS4,ap->pos4)) {
                                  /*
                                   * Problem enabling DMA on card
                                   */
                                   erp = SD_RESET_A;
                                } else
                                   erp = SD_QUIESCE_A;
				uec = ((0xF4 << 8) | alert_reg.byte[2]);
                                break;
                             case SD_TIME_OUT:
                                /*
				 * Query device timed out...
				 * tag is in ctrl status field
				 * BUT command is still active in subsystem
				 * SO...ignore the tag. It will be processed
				 * when it is purged by the reset.
				 */
				uec = 0xF407;
				erp = SD_RESET_C;
                                break;
                             case SD_UNEXP_SCSI_STATUS :
                             case SD_UNEXP_CTRL_STATUS :
                             case SD_MSG_TO_INACTIVE_TAG :
                             case SD_MSG_WRONG_LINK :
                             case SD_DMA_COUNT_ERROR :
                             case SD_INVALID_LINK_MSG :
                             case SD_RECOV_LINK_ERROR :
                                /*
                                 * Quiesce Controller
                                 */
				if (alert_reg.byte[2] != SD_RECOV_LINK_ERROR)
					uec = ((0xF4 << 8) | alert_reg.byte[2]);
                                erp = SD_QUIESCE_C;
                                break;
                             case SD_ADAP_HDW_ERROR :
                             case SD_SYSTEM_DMA_HUNG :
                                /*
                                 * Reset Adapter
                                 */
				uec = ((0xF4 << 8) | alert_reg.byte[2]);
                                erp = SD_RESET_A;
                                break;
                             case SD_DMA_DISABLED :

                                /*
                                 * Reenable DMA, will receive interrupt for
                                 * failing tag
                                 */
                                ap->pos4 |= SD_DMA_ENABLE;
                                if (sd_write_POS(ap,SD_POS4,ap->pos4)) {
                                  /*
                                   * Problem enabling DMA on card
                                   */
                                   erp = SD_RESET_A;
                                } else
                                   processed = TRUE;
				uec = ((0xF4 << 8) | alert_reg.byte[2]);
                                break;
                             default:
                                /*
                                 * UNKNOWN Adapter Status
                                 */
				uec = 0xF4FE;
                                processed = TRUE;
                                break;
                           } /* switch adapter status */
                           break;
		default :
                           /*
                            * MUST BE A VALID TAG that had an error
                            */
			   if ((tag < SD_MIN_TAG)||(tag > SD_MAX_TAG)) {
                                /*
                                 * Something is royally hosed
                                 */
				uec = 0xF419; /* invalid tag */
                                processed = TRUE;
                                break;
                           }
                           /*
                            * get pointer to cmd struct
                            */
                           cmd = (struct sd_cmd *)ap->cmd_map[tag];
                           if((cmd == NULL) || (!(cmd->status & SD_ACTIVE ))) {
                                /*
                                 * Assume the Worst ...  Halt Adapter,
                                 * Reset Adapter, Fail Outstanding cmds,
                                 * restart adapter ...
                                 */
				if (!(ap->status & SD_QUIESCE_PENDING)) 
					uec = 0xF419;
                                tag = 0;
                                erp = SD_QUIESCE_A;
                                break;
                           }
                           switch(alert_reg.byte[2]) {
                                case SD_SCSI_STATUS :
                                   /*
                                    * valid SCSI Status
                                    */
                                   cmd->status_validity = SD_VALID_SCSI_STATUS;
                                   cmd->scsi_status = alert_reg.byte[3];
                                   cmd->erp = SD_SCSI_ERP;  /*CSI error recov */
                                   /*
                                    * store address byte
                                    */
                                   cmd->dev_address = alert_reg.byte[1];
				   /* 
				    * Want to stack this failed command on 
				    * error queue, since further scsi error
				    * processing is dependent on this command
				    * being the head of the error queue
				    */
				   queue = SD_STACK;
                                   break;
                                case SD_CTRL_STATUS :
                                   switch (alert_reg.byte[3]) {
                                     case SD_ABORTED_CMD:
                                        /*
                                         * This tag was aborted, if we
                                         * originated the abort,
                                         * process as good
                                         * completion.  If we did not, save
                                         * necessary info and retry.
                                         */
                                        cmd->erp = SD_NONE;
                                        cmd->retries = 1;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                        break;
                                     case SD_INVALID_QC:
                                        /*
                                         * Invalid Queue Control...Shouldn't
                                         * happen if we sent the command , but
                                         * retry just in case.  If not us then
                                         * we'll return what happened.
                                         */
					cmd->uec = 0x0401;/* invalid q cntrl */
                                        cmd->erp = SD_NONE;
                                        cmd->retries = 1;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                	cmd->alert_tag = alert_reg.byte[0];
					cmd->elog_validity |= SD_ALERT_VALID;
                                	cmd->status |= SD_LOG_ERROR;
                                        break;
                                     case SD_PURG_OS:
                                        /*
                                         * Purged after outstanding sense
                                         */
					if (cmd->retry_count >= 7)
						/*
						 * Something's wrong, reset
						 * controller
						 */
                                        	cmd->erp = SD_RESET_C;
					else
                                        	cmd->erp = SD_NONE;
					/*
					 * allow 10 retries
					 */
					cmd->retries = 10;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = 
						alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                        break;
                                     case SD_PURG_EPOW:
                                        /*
                                         * Purged after Early Power Off  
                                         */
					cmd->uec = 0x0484;
                                        cmd->erp = SD_QUIESCE_C;
					/*
					 * set retries to 10
					 */
					cmd->retries = 10;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                	cmd->alert_tag = alert_reg.byte[0];
                                	cmd->status |= SD_LOG_ERROR;
					cmd->elog_validity |= SD_ALERT_VALID;
                                        break;
                                     case SD_WBUFF_INPROG:
                                        /*
                                         * Quiesce the controller         
                                         */
                                        cmd->erp = SD_QUIESCE_C;
                                        cmd->retries = 3;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
					/*
					 * Start a delay timer before 
					 * starting more commands....
					 * allow download to finish
					 */
					switch (cmd->type) {
					  case SD_ADAP_CMD:
					    /*
					     * start delay timer to delay before issueing
					     */
					    sd_delay(cmd->ap, 
						     (char)SD_ADAP_CMD, (uint)1000);
					    break;
					  case SD_CTRL_CMD:
					    sd_delay((struct sd_adap_info *)cmd->cp, 
						     (char)SD_CTRL_CMD, (uint)1000);
					    break;
					  case SD_DASD_CMD:
					    sd_delay((struct sd_adap_info *)cmd->dp, 
						     (char)SD_DASD_CMD, (uint)1000);
					    break;
					  default: 
					    ASSERT(FALSE);
					}
                                        cmd->dev_address = alert_reg.byte[1];
                                        break;
                                     case SD_INVALID_MSG:
                                     case SD_INVALID_MB:
                                        /*
                                         * Invalid Message or Mailbox, retry
                                         */
					cmd->uec = 0x0401;
                                        cmd->erp = SD_NONE;
                                        cmd->retries = 1;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                	cmd->alert_tag = alert_reg.byte[0];
					cmd->elog_validity |= SD_ALERT_VALID;
                                	cmd->status |= SD_LOG_ERROR;
                                        break;
                                     case SD_NO_VALID_DELAY:
                                        /*
                                         * Result of Query Device, reset
                                         * the Controller 
                                         */
					cmd->uec = 0xF48A; /* host time out */
                                        cmd->erp = SD_RESET_C;
                                        cmd->retries = 0;
                                        cmd->status_validity =
                                                SD_VALID_ADAP_STATUS |
                                                SD_VALID_CTRL_STATUS;
                                        cmd->adapter_status = alert_reg.byte[2];
                                        cmd->controller_status =
                                                alert_reg.byte[3];
                                        cmd->dev_address = alert_reg.byte[1];
                                	cmd->alert_tag = alert_reg.byte[0];
					cmd->elog_validity |= SD_ALERT_VALID;
                                	cmd->status |= SD_LOG_ERROR;
                                        break;
                                     default:
                                        /*
                                         * Unknown Controller status
                                         */
					uec = 0xF4FE;
                                        processed = TRUE;
                                        break;
                                    }
                                  break;
                                case SD_DNLD_ADAP_BUSY:
                                   /*
                                    * Must be IOCTL, noone else does
                                    * downloads, however if not an ioctl,
                                    * Reset Adapter
                                    */
				   cmd->uec = 0xF403;
                                   cmd->erp = SD_RESET_A;
                                   cmd->retries = 0;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
                                   break;
                                case SD_ILLEGAL_ADAP_CMD:
                                case SD_INVALID_ADAP_PARMS:
                                   /*
                                    * Hopefully not DD originated,
                                    * but if so set up to retry
                                    */
				   cmd->uec = ((0x04 << 8) | alert_reg.byte[2]);
                                   cmd->erp = SD_NONE;
                                   cmd->retries = 1;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
                                   break;
                                case SD_CTRL_NOT_RESPONDING:
                                   /*
                                    * Reset Controller
                                    */
				   if (cmd->cmd_info != SD_IOCTL) {
                                   	cmd->status |= SD_LOG_ERROR;
				   	cmd->uec = 0x0405;
				  	cmd->elog_validity |= SD_ALERT_VALID;
				   }
                                   cmd->erp = SD_RESET_C;
                                   cmd->retries = 2;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
                                   break;
                                case SD_OPEN_LINK:
				   if (cmd->cmd_info != SD_IOCTL) {
                                   	cmd->status |= SD_LOG_ERROR;
				   	cmd->uec = 0x0408;
				  	cmd->elog_validity |= SD_ALERT_VALID;
				   }
				   /* Fall Through */
                                case SD_RECOV_LINK_ERROR:
                                   /*
                                    * Quiesce Controller
                                    */
                                   cmd->erp = SD_QUIESCE_C;
                                   cmd->retries = 2;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   break;
				 case SD_DMA_COUNT_ERROR:
				   /*
                                    * Quiesce Controller
                                    */
				   cmd->uec = ((0x04 << 8) | alert_reg.byte[2]);
                                   cmd->erp = SD_QUIESCE_C;
                                   cmd->retries = 2;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
                                   break;
				   
                                case SD_RESET_PURGE_TAG:
                                   /*
                                    * Tag purged due to reset, set up for retry
                                    */
				   if ((cmd->type == SD_CTRL_CMD) ||
						(cmd->type == SD_DASD_CMD)) {
					   
					   if (cmd->cp->status & SD_DOWNLOAD_PENDING) { 
						   /*
						    * We do not want to count 
						    * errors due to our 
						    * downloading in the 
						    * commands retry count.  So 
						    * we will decrement it for 
						    * this case.
						    */
						   cmd->retry_count--;
					   }					   
				   }

				   if (cmd->retry_count >= 7)
					/*
					 * Something wrong, quiesce adapter
					 */
                                   	cmd->erp = SD_QUIESCE_A;
				   else
                                   	cmd->erp = SD_NONE;
				   /*
				    * allow 10 retries
				    */
				   cmd->retries = 10;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->dev_address = alert_reg.byte[1];
                                   break;
                                case SD_BAD_CTRL_ADDRESS:
                                   /*
                                    * if IOCTL not an error, otherwise,  
                                    * quiesce the adapter
                                    */
				   if (cmd->cmd_info != SD_IOCTL) {
	                                cmd->status |= SD_LOG_ERROR;
				   	cmd->uec = 0x0410;
				   	cmd->elog_validity |= SD_ALERT_VALID;
				   }
                                   cmd->erp = SD_QUIESCE_A;
                                   cmd->retries = 3;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
                                   break;
                                case SD_QUIESCE_TIME_OUT:
                                   /*
                                    * Quiesce to controller(s) timed out, so
                                    * reset all controllers in the bit mask
                                    */
				   cmd->uec = 0xF411;
                                   cmd->erp = SD_RESET_C_BM;
                                   cmd->retries = 0;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS
                                        | SD_VALID_CTRL_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[3];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
                                   break;
                                case SD_LINK_ERROR:
                                   /*
                                    * Reset the controller
                                    */
				   cmd->uec = 0x0417;
                                   cmd->erp = SD_QUIESCE_C;
                                   cmd->retries = 3;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
                                   break;
                                case SD_DNLD_MC_CHKSUM_FAIL:
                                   /*
                                    * Must have been IOCTL ...
                                    */
				   cmd->uec = 0xF413;
                                   cmd->erp = SD_NONE;
                                   cmd->retries = 0;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
                                   break;
                                case SD_MB_TERM_INTERN_RST:
                                   /*
                                    * Mailbox terminated due to internal
				    * reset, just retry 
                                    */
				   cmd->uec = 0x0421;
				   if (cmd->retry_count >= 7)
					/*
					 * Something wrong, quiesce adapter
					 */
                                   	cmd->erp = SD_QUIESCE_A;
				   else
                                   	cmd->erp = SD_NONE;
				   /*
				    * allow 10 retries
				    */
				   cmd->retries = 10;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   cmd->controller_status = alert_reg.byte[3];
                                   cmd->dev_address = alert_reg.byte[1];
                                   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
                                   cmd->status |= SD_LOG_ERROR;
				   sd_parse_ready_async(ap, alert_reg.byte,
					&processed, &erp, &uec);
				   processed = FALSE;
				   /*
				    * start 4 second delay timer before 
				    * allowing more commands.
				    */
				   if (cmd->type == SD_DASD_CMD) {
				       sd_delay((struct sd_adap_info *)cmd->dp->cp, 
						(char)SD_CTRL_CMD, (uint)4000);
				   } else if (cmd->type == SD_CTRL_CMD) {
				       sd_delay((struct sd_adap_info *)cmd->cp, 
						(char)SD_CTRL_CMD, (uint)4000);
				   }
				   break;
                                case SD_TRACE_SUPERCEDED:
                                case SD_PREV_TRC_DUMP_BUSY:
                                   /*
                                    * Must have been IOCTL ...
                                    */
                                   cmd->erp = SD_NONE;
                                   cmd->retries = 0;
                                   cmd->status_validity = SD_VALID_ADAP_STATUS;
                                   cmd->adapter_status = alert_reg.byte[2];
                                   break;
				 case SD_TRACE_DATA_DMA_ERR:
				    /*
				     * Find trace mailbox, and fail ....
				     * Pull tag out of alert reg
				     * get ptr to cmd
				     */
					
				       
				   cmd->uec = ((0xF4 << 8) |alert_reg.byte[2]);
				   cmd->erp = SD_NONE;
				   cmd->retries = 0;
				   cmd->status_validity = SD_VALID_ADAP_STATUS |
					   SD_VALID_CTRL_STATUS;
				   cmd->adapter_status = alert_reg.byte[2];
				   cmd->controller_status = alert_reg.byte[3];
				   cmd->dev_address = alert_reg.byte[1];
				   cmd->alert_tag = alert_reg.byte[0];
				   cmd->elog_validity |= SD_ALERT_VALID;
				   cmd->status |= SD_LOG_ERROR;
				   break;
                                default:
                                   /*
                                    * Unknown Adapter status
                                    */
				   uec = 0x04FE;
                                   processed = TRUE;
#ifdef SD_ERROR_PATH
				   ASSERT(uec != 0x04FE);
#endif
                                   break;
			} /* switch adapter status */
			break;
	} /* switch tag of alert register */
	if (uec) {
		/*
		 * Log Immediate Errors not associated with a command here
		 */
	         /*
		  * ************ NOTE ******************************
		  * uec MUST NOT be set to a value which indicates a
		  * controller error in sd_log_adap_error if address
		  * is not valid.
		  * ************************************************
		  */
		ap->elog.validity |= SD_ALERT_VALID;
		ap->elog.alert_reg = alert_reg.word;
		sd_log_adap_err(ap, (ushort)uec,address);
	}
	if (!processed) {
		/*
		 * if this condition has not been processed ...
		 */
		if (tag == 0) {
			/*
			 * if error but not associated
			 * with a mailbox ...processes errors
			 * with no tag assigned
			 */
			sd_alert_notag(ap, erp, address);
		} else {
			/*
			 * This processes adapter errors and SCSI errors
			 */
			 sd_process_error(ap, cmd, (char)FALSE, (char)queue);
		}
	} 
	return(0);
}

/*
 * NAME: sd_parse_ready_async
 *
 * FUNCTION: Parses Ready Async Messages either from an Asynchronous interrupt,
 *      or after a Mailbox has been purged due to internal reset.            
 *
 * EXECUTION ENVIRONMENT: This routine is called by the sd_parse_err on the
 *      interrupt level and can not page fault.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - adapter info structure
 *                      sd_ctrl_info    - controller info structure
 *                      sd_dasd_info    - dasd info structure
 *
 * RETURNS:     Void.
 */
void sd_parse_ready_async(
struct sd_adap_info *ap,
char	*alert_reg,
uchar	*processed,
uchar	*erp,
ushort	*uec)
{
    uchar	address;
    struct sd_ctrl_info *cp;
    struct sd_dasd_info *dp;
    int disk;
    
    switch (alert_reg[3])
    {
      case SD_DEVICE_READY:
	/*
	 * DEVICE has come ready
	 *      Could be for a controller or a DASD.
	 *
	 *      If we know about this one
	 *      already, then verify this device
	 *      If we don't know about one here
	 *      then post async event to config
	 */
	address = alert_reg[1];
	if (!(address & SD_TARDEV))
	{
	    /*
	     * This is a dasd ready async
	     */
	    cp = ap->ctrllist[SD_TARGET(address)];
	    if (cp != NULL) {
		/*
		 * we already know about this controller
		 */
		dp = cp->dasdlist[SD_LUN(address)];
		if (dp == NULL) {
		    /*
		     * we don't know about this dasd
		     */
		    if ((!cp->diag_mode) && (!ap->diag_mode))
			/*
			 * As long as this ctrl and
			 * adapter are not in diagmode
			 */
			sd_async_event(ap, address,
				       (uchar)SD_CONFIG,SIGTRAP);
		} else {
		    /*
		     * we thought we already
		     * knew about this dasd
		     * verify unless there is a
		     * reset or quiesce going that
		     * will verify it anyway
		     */
		    if (!(ap->status &
			  (SD_RESET_PENDING 
			   | SD_QUIESCE_PENDING)) &&
			!(cp->status &
			  (SD_RESET_PENDING 
			   | SD_QUIESCE_PENDING)))
			sd_verify((struct sd_adap_info *)dp,
				  (char)SD_DASD_CMD);
		    /*
		     * Ignore async, and rely on Unit 
		     * Attention
		     */
		}
	    } else
		/*
		 * we don't even know about this controller ??
		 */
		if (!ap->diag_mode)
		    /*
		     * if adapter not in diagmode
		     */
		    sd_async_event(ap, address, 
				   (uchar)SD_CONFIG,
				   SIGTRAP);
	    if (alert_reg[2] == SD_UNIT_POWERED_ON)
		/*
		 * if this was from a Unit powered on 
		 * and not a
		 * Terminated due to internal reset, 
		 * then set
		 * processed to true.
		 */
		*processed = TRUE;
	}
	else
	{
	    /* Controller! */
	    /*
	     * POWER ON RESET :
	     *      Quiesce Controller
	     *      Queue up Config Event to daemon
	     *      Re-init each DASD on controller
	     */
	    address = alert_reg[1];
	    cp = ap->ctrllist[SD_TARGET(address)];
	    if (cp != NULL) {
		/*
		 * we already know about this controller, 
		 * quiesce
		 */
		*erp = SD_QUIESCE_C;
		/*
		 * This controller has just been reset.
		 * During the reset, we may have missed an 
		 * asynchronous event.
		 * We must inform any registered kernel extensions
		 */
		for (disk=0; disk < SD_NUM_DASD; disk++) {
		    dp = cp->dasdlist[disk];
		    if (dp != NULL) 
			if (dp->conc_registered)
			    (dp->conc_intr_addr)(NULL,DD_CONC_RESET,0,dp->devno);
		}
				
	    } else {
		/*
		 * we don't know about this controller
		 */
		if (alert_reg[2] == SD_UNIT_POWERED_ON)
		    /*
		     * if this was from a Unit powered on 
		     * and not a
		     * Terminated due to intern reset, 
		     * then set
		     * processed to true.
		     */
		    *processed = TRUE;
		if (!ap->diag_mode)
		    /*
		     * As long as this adapter not in  
		     * diagmode
		     */
		    sd_async_event(ap, address, 
				   (uchar)SD_CONFIG,
				   SIGTRAP);
	    }
	}
	break;
	
      case SD_TYPE1_NO_IML:
	/*
	 * TYPE 1 CHECK WITHOUT IML :
	 *      Request Sense and log against
	 *              controller
	 *      Quiesce Controller
	 *      Queue up Config Event to daemon
	 *      Re-init each DASD on controller
	 *      Inform registered kernel extension.
	 *
	 *      Fall Through
	 */
      case SD_TYPE1_WITH_IML:
	/*
	 * TYPE 1 CHECK WITH IML :
	 *      Request Sense and log against
	 *              controller.
	 *      Quiesce Controller
	 *      Queue up Config Event to daemon
	 *      Queue up Download event
	 *      Re-init each DASD on Controller
	 *      Inform registered kernel extension.
	 */
	address = alert_reg[1];
	*erp = SD_SENSE_QUIESCE_C;
	cp = ap->ctrllist[SD_TARGET(address)];
	if (cp != NULL) 
	    for (disk=0; disk < SD_NUM_DASD; disk++) {
		dp = cp->dasdlist[disk];
		if (dp != NULL) 
		    if (dp->conc_registered)
			(dp->conc_intr_addr)(NULL,DD_CONC_RESET,0,dp->devno);
	    }
					
	break;
	
      default:
	/*
	 * UNKNOWN ASYNC STATUS :   Ignore
	 */
	*uec = 0xF4FE; 
	if (alert_reg[ 2] == SD_UNIT_POWERED_ON)
	    /*
	     * if this was from a Unit powered on and not a
	     * Terminated due to intern reset, then set processed
	     * to true.  
	     */
	    *processed = TRUE;
	break;
    } /* switch CTRL_STATUS */
}

/*
 * NAME: sd_alert_notag
 *
 * FUNCTION: Perform Error Recovery following an error not associated with
 *      a mailbox tag.
 *
 * EXECUTION ENVIRONMENT: This routine is called by the sd_intr on the
 *      interrupt level and can not page fault.
 *
 * (NOTES:) Possible operations :
 *      SD_RESET_A              - Reset the adapter
 *      SD_QUIESCE_A            - Quiesce the adapter
 *      SD_RESET_C              - Reset the controller
 *      SD_QUIESCE_C            - Quiesce the controller
 *      SD_SENSE_QUIESCE_C      - Request sense and quiesce controller
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_adap_info    - adapter info structure
 *                      sd_ctrl_info    - controller info structure
 *
 * RETURNS:     Void.
 */
void sd_alert_notag(
struct sd_adap_info *ap,
uchar   erp,
uchar   address)
{
        struct sd_ctrl_info *cp;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,alertnotag, entry,(char)0, (uint)ap, (uint)erp ,(uint)address, (uint)0,(uint)0);
#endif
#endif

            switch (erp) {
        	case SD_RESET_A :
                	/*
	                 * reset adapter
	                 */
	                sd_reset_quiesce(ap, (uchar)SD_RESET_OP, 
				(uchar)SD_ADAP_CMD);
			break;
                case SD_QUIESCE_A :
                        /*
                         * Quiesce the adapter
                         */
                        sd_reset_quiesce(ap, (uchar)SD_QUIESCE_OP,
                                             (uchar)SD_ADAP_CMD);
                        break;
                case SD_RESET_C :
                        /*
                         * Assume that the reset will result in tags
                         * failed back
                         */
                        cp = ap->ctrllist[SD_TARGET(address)];
                        if (cp != NULL) {
                                sd_reset_quiesce((struct sd_adap_info *)cp,
                                        (uchar)SD_RESET_OP, (uchar)SD_CTRL_CMD);
                        }
                        break;
                case SD_QUIESCE_C:
                        /*
                         * Assume that the quiesce will result in tags
                         * failed back
                         */
                        cp = ap->ctrllist[SD_TARGET(address)];
                        if (cp != NULL) {
                               sd_reset_quiesce((struct sd_adap_info *)cp,
                                  (uchar)SD_QUIESCE_OP, (uchar)SD_CTRL_CMD);
                        }
                        break;
                case SD_SENSE_QUIESCE_C:
                        /*
                         * Perform a request sense before the quiesce
                         * Notice, the sd_reset_quiesce and sd_request_sense
                         * routines STACK their commands on the queue
                         */
                        cp = ap->ctrllist[SD_TARGET(address)];
                        if (cp != NULL) {
				sd_reset_quiesce((struct sd_adap_info *)cp,
                                  (uchar)SD_QUIESCE_OP,(uchar)SD_CTRL_CMD);
                               sd_request_sense(0,cp, (char)SD_CTRL_CMD);
                        }
                        break;
                default:
                        /*
                         * Programming error
                         */
                        ASSERT(FALSE);
           }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,alertnotag, exit, (char)0, (uint)ap, (uint)erp ,(uint)address, (uint)0,(uint)0);
#endif
#endif
}



/*
 * NAME: sd_prepare_buf_retry
 *
 * FUNCTION: Prepares a Normal Path command (involving buf structures) for
 *      retries.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_process_sense on the
 *      interrupt level, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine updates the command information
 *      to retry a buf command
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd  - command structure
 *                      buf     - buf structure
 *
 * RETURNS:     Void.
 */

void sd_prepare_buf_retry(
struct sd_cmd *cmd)
{
        char    buf_found;
	uint	blk_cnt;
        struct buf *bp;
	struct sc_cmd *scsi;
        int     i;


#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,bufretry, entry,(char)0, (uint)cmd->bp, (uint)cmd->last_rba_valid, (uint)cmd->last_rba,(uint)0, (uint)0);
        sd_dptrc(cmd->dp,bufretry, entry,(char)0, (uint)cmd->bp);
#endif
#endif
        /*
         * set interrupted flag to let our activity timer know we are
         * still trucking
         */
        cmd->dp->interrupted = TRUE;
        cmd->dp->query_count = 0;
        if (cmd->last_rba_valid) {
                /*
                 * if the last RBA field is valid
                 */
                buf_found = FALSE;
                bp = cmd->bp;
		cmd->first_bp_resid = 0;
                while (!buf_found && bp != NULL) {
                        /*
                         * attempt to complete any bufs that were
                         * successful before the error
                         */
                        if ((bp->b_blkno <= cmd->last_rba) &&
                           ((bp->b_blkno + (bp->b_bcount / SD_BPS)) >
                            cmd->last_rba)) {
                                /*
                                 * if the last rba processed
                                 * falls within this buf, go ahead and set up
				 * the info for this buf       
                                 */
                                buf_found = TRUE;
				cmd->first_bp_resid = bp->b_bcount - 
				       ((cmd->last_rba - bp->b_blkno) * SD_BPS);
				bp->b_flags |= B_ERROR;
                                bp->b_error = ESOFT;
                        } else {
                                /*
                                 * this buf wasn't at fault, complete
                                 * with no error
                                 */
                                cmd->bp = (struct buf *)bp->b_work;
                                /*
                                 * adjust transfer length in case
                                 * of retry
                                 */
                                cmd->b_length -= bp->b_bcount;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
                                sd_trc(cmd->ap,sdiodone, trc,(char)0, (uint)bp, (uint)bp->b_error , (uint)bp->b_flags, (uint)bp->b_resid,(uint)0);
                                sd_dptrc(cmd->dp,sdiodone, trc,(char)0, (uint)bp);
#endif
#endif
				DDHKWD2(HKWD_DD_SERDASDD, DD_SC_IODONE, 
					bp->b_error, bp->b_dev, bp);
                                iodone(bp);
                                /*
                                 * look at next bp
                                 */
                                bp = cmd->bp;
                        }
                }
                if (cmd->bp == NULL) {
                        /*
                         * we completed all of the bufs as successful
                         * so set retry limit so start command will
                         * dequeue and free this command
                         */
                        cmd->retries = 0;
                        return;
                } else {
			/*
			 * Adjust Remaining Command fields 
			 */
			/*
			 * new buffer address is now the
			 * next guy's
			 */
			cmd->b_addr = cmd->bp->b_un.b_addr;
			/*
			 * new target RBA is next guy's
			 */
			cmd->rba = cmd->bp->b_blkno;
			cmd->last_rba_valid = 0;
			/*
			 * Now adjust SCSI command
			 */
			blk_cnt = cmd->b_length / SD_BPS;
        		scsi = &(cmd->mbox_copy.mb8.fields.scsi_cmd);
			scsi->scsi_bytes[0] = ((cmd->rba >> 24) & 0xff);
			scsi->scsi_bytes[1] = ((cmd->rba >> 16) & 0xff);
			scsi->scsi_bytes[2] = ((cmd->rba >> 8) & 0xff);
			scsi->scsi_bytes[3] = (cmd->rba & 0xff);
			scsi->scsi_bytes[4] = 0x00;
			scsi->scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
			scsi->scsi_bytes[6] = (blk_cnt & 0xff);
			scsi->scsi_bytes[7] = 0x00;
		}
	}
        
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,bufretry, exit,(char)0, (uint)cmd, (uint)0 , (uint)0, (uint)0,(uint)0);
#endif
#endif
}

/*
 * NAME: sd_fail_buf_cmd
 *
 * FUNCTION: Fails a Normal Path Command (involving buf structures)
 *
 * EXECUTION ENVIRONMENT: This routine is called on the interrupt level by
 *      sd_fail_cmd, and can not page fault.
 *
 * (NOTES:) Possible operations : Appropriately fails the buf structures 
 * 	involved in the command.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command structure
 *                      buf             - Buf structure
 *
 * RETURNS:     Void.
 */
void sd_fail_buf_cmd(
struct sd_cmd *cmd,
char	fail_dasd)
{
        struct buf *bp;
	struct sc_cmd *scsi;
	uint	blk_cnt;
	struct sd_adap_info *ap;

        /*
         * assume that the failing buf is the first in chain
         */

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,failbuf, entry,(char)0, (uint)cmd->bp, (uint)cmd->last_rba_valid, (uint)cmd->last_rba,(uint)0,(uint)0);
        sd_dptrc(cmd->dp,failbuf, entry,(char)0, (uint)cmd->bp);
#endif
#endif
        /*
         * set interrupted flag to let our activity timer know we are
         * still trucking
         */
	ap = cmd->ap;

        cmd->dp->interrupted = TRUE;
        cmd->dp->query_count = 0;
	bp = cmd->bp;
	if (bp == NULL) {
		/*
		 * catch the failing of an emptied buf command
		 */
		sd_free_cmd(cmd);
		return;
	}
        /*
         * Out of retries, Fail the first buf, unless this is from fail dasd,
	 * then fail all the bufs
         */
	if (fail_dasd) {
		/*
		 * this is from fail dasd, so fail all the bufs
		 */
	        while (bp != NULL) {
	                cmd->bp = (struct buf *)cmd->bp->b_work;
			bp->b_error = cmd->b_error;
			bp->b_resid = bp->b_bcount;
	                bp->b_flags |= B_ERROR;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
	                sd_trc(cmd->ap,sdiodone, trc,(char)0, (uint)bp, (uint)bp->b_error , (uint)bp->b_flags, (uint)bp->b_resid,(uint)0);
	                sd_dptrc(cmd->dp,sdiodone, trc,(char)0, (uint)bp);
#endif
#endif
			DDHKWD2(HKWD_DD_SERDASDD, DD_SC_IODONE, bp->b_error, 
				bp->b_dev, bp);
	                iodone(bp);
	                /*
	                 * look at next bp
	                 */
	                bp = cmd->bp;
	        }
		sd_free_cmd(cmd);
	} else {
		/*
		 * fail the first buf, and prepare rest of command for
		 * clean start
		 */
		if (cmd->reloc &&  cmd->first_bp_resid) {
			/*
			 * if reloc was set, and we have a resid to prove it
			 * upgrade to EMEDIA.  NOTICE:  For a permanent media
			 * error like this one, we have been through prepare_
			 * buf_retry, which sets the resid and error to ESOFT,
			 * in hopes it will be successful after retries.  If
			 * it is successful, then ESOFT is returned with the
			 * preset resid, if not it ends up here.
			 */
			bp->b_resid = cmd->first_bp_resid;
                        bp->b_error = EMEDIA;
                } else {
                        bp->b_error = cmd->b_error;
                        bp->b_resid = bp->b_bcount;
                }
                bp->b_flags |= B_ERROR;

		/*
		 * Adjust Command fields 
		 */
		cmd->bp = (struct buf *)bp->b_work;
		if (cmd->bp != NULL) {
			/*
			 * if this wasn't the only buf
			 */
			if (cmd->status & SD_LOG_ERROR)
				/*
				 * if log error flag set
				 */
				sd_log_error(cmd);
			cmd->b_length -= bp->b_bcount;
			cmd->b_addr = cmd->bp->b_un.b_addr;
			cmd->rba = cmd->bp->b_blkno;
			/*
			 * set status to Retry, so his timer will be stopped
			 * upon completion.  Set retry count to -1 for this
			 * first "free" retry
			 */
			cmd->status = SD_RETRY;
			cmd->retry_count = (signed char) -1;
			cmd->retries = 0;
			cmd->status_validity = 0;
			cmd->erp = 0;
			cmd->b_error = 0;
			cmd->uec = 0;
			cmd->last_rba_valid = 0;
			cmd->reloc = 0;

			/*
			 * Now adjust SCSI command
			 */
			blk_cnt = cmd->b_length / SD_BPS;
        		scsi = &(cmd->mbox_copy.mb8.fields.scsi_cmd);
			scsi->scsi_bytes[0] = ((cmd->rba >> 24) & 0xff);
			scsi->scsi_bytes[1] = ((cmd->rba >> 16) & 0xff);
			scsi->scsi_bytes[2] = ((cmd->rba >> 8) & 0xff);
			scsi->scsi_bytes[3] = (cmd->rba & 0xff);
			scsi->scsi_bytes[4] = 0x00;
			scsi->scsi_bytes[5] = ((blk_cnt >> 8) & 0xff);
			scsi->scsi_bytes[6] = (blk_cnt & 0xff);
			scsi->scsi_bytes[7] = 0x00;

		        sd_q_cmd(cmd,(char)SD_QUEUE);
		} else
			/*
			 * Must have been only buf
			 */
			sd_free_cmd(cmd);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
                sd_trc(ap,sdiodone, trc,(char)0, (uint)bp, (uint)bp->b_error , (uint)bp->b_flags, (uint)bp->b_resid,(uint)0);
#endif
#endif
		DDHKWD2(HKWD_DD_SERDASDD, DD_SC_IODONE, bp->b_error, bp->b_dev,
			bp);
		/*
		 * iodone the failed buf
		 */
                iodone(bp);
	}

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,failbuf, exit,(char)0, (uint)cmd, (uint)0, (uint)0, (uint)0,(uint)0);
#endif
#endif
}



/*
 * NAME: sd_process_error
 *
 * FUNCTION: Performs Error Recovery for a failed command.
 *
 * EXECUTION ENVIRONMENT: This routine runs on the interrupt level, called by
 *      sd_intr or sd_process_complete, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine performs DMA cleanup on the
 *      Mailbox and data transfer(if any), frees resources used by this command,
 *      and then determines the origin of the command and takes the appropriate
 *      recovery action.
 *
 * (RECOVERY OPERATION:)  None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command structure
 *                      sd_adap_info    - Adapter info structure
 *                      sd_ctrl_info    - Controller info structure
 *                      sd_dasd_info    - DASD info structure
 *
 * RETURNS:     Void.
 */

void sd_process_error(
struct  sd_adap_info *ap,
struct  sd_cmd  *cmd,
char    skip,
char	queue)
{
        struct  sd_dasd_info *dp;
        struct  sd_ctrl_info *cp;
        uchar   mask,
                target;

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,error, entry,(char)0, (uint)ap, (uint)cmd , (uint)cmd->cmd_info, (uint)cmd->erp,(uint)0);
#endif
#endif
        if (!skip)
                /*
                 * if not called from sd_process_complete after an error
                 * in sd_dma_cleanup, then perform
                 * DMA cleanup on Mailbox and any data transfer
                 */
                (void)sd_dma_cleanup(cmd,(uchar)1);

        /*
         * Free any TCW's or STA's
         */
        sd_TCW_dealloc(cmd);
        sd_STA_dealloc(cmd);

        /*
         * save a copy of our mailbox in cmd jacket
         */
        bcopy (cmd->mb, &(cmd->mbox_copy), SD_MB_SIZE);

        /*
         * free this commands mailbox
         */
        sd_free_MB(cmd, (char)SD_USED);

	if ((cmd->status & SD_TIMEDOUT) && ((cmd->cmd_info == SD_IOCTL) ||
	    (cmd->cmd_info == SD_QRYDEV))) {
		/*
		 * if this command previously timed out, and was
		 * an ioctl or query dev, just free it
		 */
		sd_free_cmd(cmd);
	} else if (cmd->cmd_info != SD_IOCTL) {
                /*
                 * All Other commands
                 */
                /*
                 * Place cmd that caused the error on the error
                 * stack, and then determine what procedure to take
                 */
	        if (cmd->cmd_info == SD_QRYDEV) {
                        /*
                         * This command was an internal query device command
                         * so just stop the query timer, 
                         */
                        w_stop(&(cmd->dp->query_timer.watch));
		}
                else if (cmd->type == SD_DASD_CMD) {
		    if ((struct sd_cmd *)cmd->dp->cmd_timer.pointer == cmd)
			w_stop(&cmd->dp->cmd_timer.watch);
		}
		else if (cmd->type == SD_CTRL_CMD) {
		    if ((struct sd_cmd *)cmd->cp->cmd_timer.pointer == cmd)
                        w_stop(&cmd->cp->cmd_timer.watch);
                } else {
		    if ((struct sd_cmd *)cmd->ap->cmd_timer.pointer == cmd)
                        w_stop(&cmd->ap->cmd_timer.watch);
		}
		if (((cmd->cmd_info & SD_OK_DURING_VERIFY) &&
		     (cmd->cmd_info != SD_QRYDEV))||
	 	     (cmd->cmd_info == SD_REASSIGN))
			/*
			 * if this is a failed reset/quiesce, request sense, 
			 * start unit, test unit ready, reserve, mode sense
			 * mode select, read capacity, release, inquiry,
			 * or reassign, set queue type to stack.
			 */ 
			queue = SD_STACK;
	
                cmd->status |= SD_RETRY;
                sd_q_cmd(cmd, queue);
                switch(cmd->erp) {
                        case SD_NONE:
                                /*
                                 * No Procedure Necessary, just let
                                 * nature take its course.  The command
                                 * will now go through retries.
                                 */
                                break;
                        case SD_SCSI_ERP:
                                sd_process_scsi_error(ap,cmd);
                                break;
                        case SD_RESET_A:
                                /*
                                 * reset the adapter, and then fail
                                 * all mapped commands for this adap
                                 */
                                sd_reset_quiesce(ap, (uchar)SD_RESET_OP,
                                        (uchar)SD_ADAP_CMD);
                                break;
                        case SD_QUIESCE_A :
                                /*
                                 * quiesce adapter
                                 */
                                sd_reset_quiesce(ap, (uchar)SD_QUIESCE_OP,
                                        (uchar)SD_ADAP_CMD);
                                break;
                        case SD_RESET_C :
                                /*
                                 * Assume that the reset will result
                                 * in tags failed back
                                 */
                                if (cmd->cp == NULL) {
                                        if (cmd->dp == NULL)
                                           cp = ap->ctrllist[SD_TARGET(
                                                cmd->dev_address)];
                                        else
                                           cp = cmd->dp->cp;
                                } else
                                        cp = cmd->cp;
                                if (cp != NULL) {
                                        sd_reset_quiesce(
                                           (struct sd_adap_info *)cp,
                                           (uchar)SD_RESET_OP,
                                           (uchar)SD_CTRL_CMD);
                                }
                                break;
                        case SD_QUIESCE_C:
                                /*
                                 * Assume that the quiesce will
                                 * result in tags failed back
                                 */
                                if (cmd->cp == NULL) {
                                        if (cmd->dp == NULL)
                                           cp = ap->ctrllist[SD_TARGET(
                                                cmd->dev_address)];
                                        else
                                           cp = cmd->dp->cp;
                                } else
                                        cp = cmd->cp;
                                if (cp != NULL) {
                                        sd_reset_quiesce(
                                           (struct sd_adap_info *)cp,
                                           (uchar)SD_QUIESCE_OP,
                                           (uchar)SD_CTRL_CMD);
                                }
                                break;
                        case SD_RESET_C_BM :
                                /*
                                 * Reset all controllers as specified
                                 * by the bitmask in the dev_address
                                 */
                                mask = 0x80;
                                for (target = 0;target<SD_NUM_CTRLS;target++) {
                                  /*
                                   * for each possible controller on
                                   * this adapter, see if corresponding
                                   * bit is set in dev_address
                                   */
                                   if ((mask >> target) &
                                        cmd->dev_address) {
                                          /*
                                           * This one is set
                                           */
                                           cp = ap->ctrllist[target];
                                           if (cp != NULL) 
					       /*
						* If the controller is valid
						*/
					       if ((cmd->type != SD_CTRL_CMD) ||
						   (cmd->cp != cp) || 
						   (cmd->cmd_info != SD_RST_QSC)) {
						   /*
						    * Do not reset the controller 
						    * if this controller was being
						    * quiesced as the quiesce
						    * will get escallated to a reset
						    * when the command is failed
						    */
						   sd_reset_quiesce(
								    (struct sd_adap_info *)cp,
								    (uchar)SD_RESET_OP,
								    (uchar)SD_CTRL_CMD);
					       }
				       } /* end if */
                                } /* end for */
                                break;
                        case SD_RESET_D:
                                /*
                                 * Assume that the reset will
                                 * result in tags failed back
                                 */
                                if (cmd->dp == NULL) {
                                   cp = ap->ctrllist[SD_TARGET(
                                        cmd->dev_address)];
                                   dp = cp->dasdlist[SD_LUN(
                                        cmd->dev_address)];
                                } else
                                        dp = cmd->dp;
                                sd_reset_quiesce(
                                   (struct sd_adap_info *)dp,
                                   (uchar)SD_RESET_OP,
                                   (uchar)SD_DASD_CMD);
                                break;
                        case SD_QUIESCE_D:
                                if (cmd->dp == NULL) {
                                   cp = ap->ctrllist[SD_TARGET(
                                        cmd->dev_address)];
                                   dp = cp->dasdlist[SD_LUN(
                                        cmd->dev_address)];
                                } else
                                   dp = cmd-> dp;
                                sd_reset_quiesce(
                                   (struct sd_adap_info *)dp,
                                   (uchar)SD_QUIESCE_OP,
                                   (uchar)SD_DASD_CMD);
                                break;
                        default:
                                /*
                                 * Programming Error ...
                                 */
                                ASSERT(FALSE);
                                break;
                }
        } else {
                /*
                 * this was an ioctl command
                 */
		/*
		 * else wakeup the ioctl routine
		 */
		switch(cmd->type) {
			/*
			 * Stop the appropriate timer, clear the
			 * ioctl interrupt flag, and wakeup the event
			 */
			case SD_ADAP_CMD:
				w_stop(&cmd->ap->ioctl_timer.watch);
				cmd->ap->ioctl_intrpt = 0;
				e_wakeup(&cmd->ap->ioctl_event);
				break;
			case SD_CTRL_CMD:
				w_stop(&cmd->cp->ioctl_timer.watch);
				cmd->cp->ioctl_intrpt = 0;
				e_wakeup(&cmd->cp->ioctl_event);
				break;
			case SD_DASD_CMD:
				w_stop(&cmd->dp->ioctl_timer.watch);
				cmd->dp->ioctl_intrpt = 0;
				e_wakeup(&cmd->dp->ioctl_event);
				break;
		}
	}
	
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,error, exit,(char)0, (uint)ap, (uint)cmd , (uint)0, (uint)0,(uint)0);
#endif
#endif
}


/*
 * NAME: sd_process_scsi_error
 *
 * FUNCTION: Performs Error Recovery for a command with a SCSI Error.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_process_error on the
 *      interrupt level, and can not page fault.
 *
 * (NOTES:) Possible operations : This routine parses the SCSI status and
 *      takes the appropriate action.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command Structure
 *                      sd_adap_info    - Adapter Info Structure
 *
 * RETURNS:     Void.
 */

void sd_process_scsi_error(
struct  sd_adap_info    *ap,
struct  sd_cmd  *cmd)
{

#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(cmd->ap,scsierror, trc,(char)0, (uint)ap, (uint)cmd , (uint)cmd->scsi_status, (uint)cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_op_code,(uint)0);
        sd_dptrc(cmd->dp,scsierror, trc,(char)0, (uint)cmd->scsi_status);
#endif
#endif
	/*
	 * Set flag to log error when processing of this
	 * command is complete
	 */
        switch(cmd->scsi_status) {
                case SD_GOOD:
                        /*
                         * Set error flag to generic EIO, command is on error
                         * queue and will be retried if able
                         */
                        cmd->uec = 0x04FE;
			cmd->status |= SD_LOG_ERROR;
                        cmd->retries = 3;
                        cmd->b_error = EIO;
#ifdef SD_ERROR_PATH
		        ASSERT(cmd->uec != 0x04FE);
#endif
                        break;
                case SD_CHECK:
                        if (cmd->cmd_info == SD_REQSNS) {
                                /*
                                 * Check condition on a request sense is FATAL
                                 * dequeue the request sense, and fail it.
				 * This can actually happen if the request sense
				 * overlaps with a start motor command.
				 * The original checked command will be retried
                                 */
				cmd->status |= SD_LOG_ERROR;
				sd_d_q_cmd(cmd);
				sd_fail_cmd(cmd, (char) FALSE);
                        } else {
                                /*
                                 * Set up default number of retries for this
                                 * command to 3, and default error code to EIO,
                                 * these may be changed by process sense.
                                 */
                                cmd->b_error = EIO;
                                cmd->retries = 3;
				if (cmd->type == SD_DASD_CMD)
					/*
					 * Save a reference to the checked
					 * command.
					 */
					cmd->dp->checked_cmd = cmd;
                                /*
                                 * queue up a request sense to appropriate
                                 * device queue
                                 */
                                sd_request_sense(cmd->dp, cmd->cp, cmd->type);
                        }
                        break;
                case SD_QUEUE_FULL:
			if (cmd->type == SD_DASD_CMD) {
			    /*
			     * Controller Queue is full , set retries to 10, command
			     * will be retried. Start delay timer for this DASD. 
			     */
			    cmd->uec = 0x0500;
			    cmd->b_error = EBUSY;
			    cmd->retries = 10;
			    /*
			     * Give a free retry
			     */
			    cmd->retry_count--;
			    sd_delay((struct sd_adap_info *)cmd->dp, 
				     (char)SD_DASD_CMD, (uint)1000);
			} else {
			    /* 
			     * Non DASD addressed commands should not
			     * get Q full.
			     */
			    cmd->uec = 0x04FE;
#ifdef SD_ERROR_PATH
			    ASSERT(cmd->uec != 0x04FE);
#endif
			    cmd->status |= SD_LOG_ERROR;
			    cmd->b_error = EIO;
			    cmd->retries = 1;
			}
			break;
		case SD_RES_CONFLICT:
			if (cmd->type == SD_DASD_CMD) {
			    cmd->b_error = EBUSY;
			    if (cmd->dp->cp->dds.conc_enabled)
				cmd->retries = 4;
			    else
				cmd->retries = 1;
			    sd_delay((struct sd_adap_info *)cmd->dp, 
				     (char)SD_DASD_CMD, (uint)2000);
			} else {
			    /* 
			     * Non DASD addressed commands should not
			     * get Reservation Conflict.
			     */
			    cmd->uec = 0x04FE;
#ifdef SD_ERROR_PATH
			    ASSERT(cmd->uec != 0x04FE);
#endif
			    cmd->status |= SD_LOG_ERROR;
			    cmd->b_error = EIO;
			    cmd->retries = 1;
			}
			break;
		case SD_FENCED_OUT:
                        cmd->b_error = ENOCONNECT;
                        cmd->retries = 0; /* do not allow retry */
			break;
                default:
                        /*
                         * UNKNOWN SCSI ERROR CONDITION
                         */
                        cmd->uec = 0x04FE;
#ifdef SD_ERROR_PATH
		        ASSERT(cmd->uec != 0x04FE);
#endif
			cmd->status |= SD_LOG_ERROR;
                        cmd->b_error = EIO;
                        cmd->retries = 1;
                        break;
        }
        /*
         * Notice: when exitting this routine, the command causing the error
         * is on the error queue, with possibly a request sense command
         * stacked in front of it. Failure of the command is decided by
         * sd_start_cmd
         */
}


/*
 * NAME: sd_process_sense
 *
 * FUNCTION: Determines what recovery action to take from Sense Data returned
 *      from a SCSI Request Sense command.
 *
 * EXECUTION ENVIRONMENT: This routine is called by sd_process_complete upon
 *      completion of a request sense command on the interrupt level, and can
 *      not page fault.
 *
 * (NOTES:) Possible operations : Retrieves the Host Action Code from the
 *      sense buffer, determines whether to log the sense data, and then
 *      performs recovery per HAC.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_dasd_info    - DASD info structure
 *                      sd_ctrl_info    - Controller info structure
 *
 * RETURNS:     Void.
 */

void sd_process_sense(
struct sd_dasd_info *dp,
struct sd_ctrl_info *cp,
char    type)
{
	struct sd_adap_info *ap;
        uchar   log = FALSE,
                hac,
		deferred = FALSE;
	short	uec,cme;
        struct sd_cmd *cmd = NULL;
	int     freed  = FALSE;          /* indicates checked command has been*/
					 /* freed since we issued the request */
					 /* sense			      */


        if (type == SD_DASD_CMD) {
                /*
                 * this was a request sense to a DASD, so set
                 * the command pointer to the original checked command, 
		 * and extract the HAC from the DASD's sense buffer
		 * as well as the unit error code
                 */
                cmd = dp->checked_cmd;
		if (cmd == NULL) {
			/*
			 * This must have been sense data from an unsolicited 
			 * request sense, since their is no command on the 
			 * error queue for which this sense data applies.  One 
			 * example is the sense and quiesce error recovery for 
			 * a type 1 controller check.  There is not a specific 
			 * command associated with the error, but
			 * the PDG says to perform a request sense and log.
			 */
			return;
		}
		if (cmd->status == SD_FREE)
                        freed = TRUE;
        	bcopy (dp->sense_buf, cmd->req_sns_bytes, SD_REQ_SNS_LENGTH);
                dp->checked_cmd = NULL;
                hac = dp->sense_buf[18];
		uec = ((dp->sense_buf[20] << 8) | dp->sense_buf[21]);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
		cme = dp->sense_buf[19];
#endif
#endif
		cp = dp->cp;
		ap = dp->ap;
		if (dp->sense_buf[0] & 0x01)
			/*
			 * This was a deferred error, no corresponding command
			 */
			deferred = TRUE;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        	sd_dptrc(dp,scsisense, trc,(char)0, (uint)cmd);
#endif
#endif
        } else {
                /*
                 * this was a request sense to a controller, so there is
                 * no associated cmd since the DD does not issue SCSI cmds to
		 * controllers internally, only DASD.  Therefore, we will 
		 * never process sense for a controller in response to a 
		 * checked command. We only request sense from the controller
		 * for info only after a controller type 1 error. 
		 * Extract the HAC from the controller's
                 * sense buffer
                 */
                hac = cp->sense_buf[18];
		uec = ((cp->sense_buf[20] << 8) | cp->sense_buf[21]);
#ifdef DEBUG
#ifdef SD_ERROR_PATH
		cme = cp->sense_buf[19];
#endif
#endif		
		ap = cp->ap;
		if (cp->sense_buf[0] & 0x01)
			/*
			 * This was a deferred error, no corresponding command
			 */
			deferred = TRUE;
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,scsisense, entry,(char)0, (uint)cmd, (uint)hac, (uint)cme, (cmd ? (uint)cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_op_code : 0),(uint)0);
#endif
#endif

        if ( hac & SD_HAC_LOG_SENSE ) 
                /*
                 * write sense record to error log
                 */
		if (cmd && !deferred && !freed) {
			cmd->status |= SD_LOG_ERROR;
			cmd->uec = uec;
		} else if (type == SD_DASD_CMD) 
			sd_log_dasd_err(dp, (ushort)uec);
		else
			sd_log_ctrl_err(cp, (ushort)uec);

        if (cmd == NULL)
                /*
                 * This must have been sense data from an unsolicited request
                 * sense, since their is no command on the error queue for
                 * which this sense data applies.  One example is the sense and
                 * quiesce error recovery for a type 1 controller check.  There
                 * is not a specific command associated with the error, but
                 * the PDG says to perform a request sense and log.
                 */
                return;

        if (hac == 0) {
                /*
                 * The only sense with a zero hac is that availble when
                 * no command has been checked. Yet this command WAS checked
                 * so there must have been a reset.
                 * We cannot tell what the
                 * sense for the checked command was. We have no choice
                 * but to retry the command.
                 * Command is already head of queue, so set his
                 * retries to 1. Notice that sd_start_cmd will make
                 * sure this command has not exhausted retries.
                 */
                if (deferred || freed)
                        return;
                cmd->retries = 1;
                return;
        }

        switch ((hac & SD_HAC_MASK)) {
                /*
                 * Now take the recommended Host Action Code Recovery Procedure
                 */
                case SD_NO_RECOV_HAC:
                        /*
                         * Complete the head of the appropriate queue
                         * as successful (dequeue and call process complete )
                         */
			if (deferred || freed)
				return;
                        sd_d_q_cmd(cmd);
                        sd_process_complete(cmd, (char)TRUE);
                        break;
                case SD_NO_RETRY_HAC:
                case SD_REASSIGN_FAIL:
                        /*
                         * Fail the head of the appropriate queue
                         * (dequeue and call fail cmd)
                         */
			if (deferred || freed)
				return;
                        sd_d_q_cmd(cmd);
                        cmd->b_error = EIO;
                        cmd->retries = 0;
                        sd_fail_cmd(cmd, (char) FALSE);
                        break;
                case SD_RETRY_1_HAC:
                        /*
                         * Command is already head of queue, so set his
                         * retries to 1. Notice that sd_start_cmd will make
                         * sure this command has not exhausted retries.
                         */
			if (deferred || freed)
				return;
                        cmd->retries = 1;
                        break;
                case SD_RETRY_10_HAC:
                        /*
                         * Command is already head of queue, so set his
                         * retries to 10. Notice that sd_start_cmd will make
                         * sure this command has not exhausted retries.
                         */
			if (deferred || freed)
				return;
                        cmd->retries = 10;
                        break;
                case SD_NO_VALID_SENSE:
			/*
			 * The HAC indicates that the sense data is invalid,
			 * so reset the UEC and fall through to reset the
			 * controller
			 */
			if (!deferred && !freed) {
				cmd->uec = 0x03FF;
				cmd->status |= SD_LOG_ERROR;
			}
                case SD_RESET_CTRL_HAC:
                        /*
                         * set retries for head of queue to 1
                         * Put a controller reset on controller error queue
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        sd_reset_quiesce((struct sd_adap_info *)cp,
                                (uchar)SD_RESET_OP, (uchar)SD_CTRL_CMD);
                        break;
                case SD_QUIESCE_CTRL_HAC:
                        /*
                         * set retries for head of queue to 1
                         * Put a controller quiesce on controller err queue
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        sd_reset_quiesce((struct sd_adap_info *)cp,
                                (uchar)SD_QUIESCE_OP, (uchar)SD_CTRL_CMD);
                        break;
                case SD_RESET_DASD_HAC:
                        /*
                         * set retries for head of queue to 1
                         * Put a dasd reset on dasd error queue
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        if (dp != NULL)
                        	sd_reset_quiesce((struct sd_adap_info *)dp,
                                (uchar)SD_RESET_OP, (uchar)SD_DASD_CMD);
                        break;
                case SD_QUIESCE_DASD_HAC:
                        /*
                         * set retries for head of queue to 1
                         * Put a dasd quiesce on dasd error queue
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        if (dp != NULL)
                        	sd_reset_quiesce((struct sd_adap_info *)dp,
                                (uchar)SD_QUIESCE_OP, (uchar)SD_DASD_CMD);
                        break;
                case SD_VERIFY_CTRL_HAC:
                        /*
                         * perform controller verification, followed by reset
                         * cycle for each affected dasd.
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        sd_verify((struct sd_adap_info *)cp,(char)SD_CTRL_CMD);
                        break;
                case SD_ESOFT_ERR_HAC:
                        /*
                         * This means that the command completed successfully,
                         * but only after internal device retries.
                         * dequeue this command, set error to ESOFT,
                         * update last rba, etc. set reloc flag., and call
                         * sd_process_buf
                         */
			if (deferred || freed)
				return;
                        sd_d_q_cmd(cmd);
                        cmd->b_error = ESOFT;
                        cmd->status |= SD_RECOV_ERROR;
                        if (dp->sense_buf[0] & 0x80) {
                                /*
                                 * if RBA valid bit set in sense data
                                 */
                                cmd->reloc = TRUE;
                                cmd->last_rba_valid = TRUE;
                                cmd->last_rba = (dp->sense_buf[3] << 24) |
                                                (dp->sense_buf[4] << 16) |
                                                (dp->sense_buf[5] << 8)  |
                                                (dp->sense_buf[6]);
                        }
                        sd_process_buf(cmd);
                        break;
                case SD_EMEDIA_ERR_HAC:
                        /*
                         * This means we had some sort of hard error, prepare
                         * this command for retries.
                         * set error field (EMEDIA) of command on head of queue,
                         * update last rba, etc. set reloc flag. call process
                         * buf error for command on head of queue
                         */
			if (deferred || freed)
				return;
                        cmd->retries = 3;
                        cmd->b_error = EMEDIA;
                        if (dp->sense_buf[0] & 0x80) {
                                /*
                                 * if RBA valid bit set in sense data
                                 */
                                cmd->reloc = TRUE;
                                cmd->last_rba_valid = TRUE;
                                cmd->last_rba = (dp->sense_buf[3] << 24) |
                                                (dp->sense_buf[4] << 16) |
                                                (dp->sense_buf[5] << 8)  |
                                                (dp->sense_buf[6]);
                        }
                        sd_prepare_buf_retry(cmd);
                        break;
                case SD_A_RESET_DASD_HAC:
                        /*
                         * set retries for head of queue to 1
                         * Put an absolute dasd reset on dasd error queue
                         * (send diagnostics dasd reset)
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        if (dp != NULL)
                        	sd_reset_quiesce((struct sd_adap_info *)dp,
                                (uchar)SD_RESET_DASD, (uchar)SD_DASD_CMD);
                        break;
                case SD_VERIFY_DASD_HAC:
                        /*
                         * perform dasd verification
                         */
			if (!deferred && !freed)
                        	cmd->retries = 3;
                        if (dp != NULL)
                                /*
                                 * Make sure dp is valid
                                 */
                                sd_verify((struct sd_adap_info *)dp,
                                        (char)SD_DASD_CMD);
                        break;
                case SD_ASYNC_EVNT_HAC:
                        /*
			 * This hac is not used in this release.
                         * Allow the command to have 10 retries.
                         * Since this is NOT an error, decrement the 
                         * retry count so that this does not count towards the 
                         * retries.     
                         */
                        if (deferred || freed)
                                return;
                        cmd->retry_count--;
                        cmd->retries = 10;
                        break;
                default:
                        /*
                         * Unknown HAC, head of queue
                         * will be retried.
                         */
			if (!deferred && !freed) {
				cmd->status |= SD_LOG_ERROR;
				cmd->uec = uec;
			}
                        break;
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,scsisense, exit,(char)0, (uint)cmd, (uint)hac, (uint)0, (cmd ? (uint)cmd->mbox_copy.mb8.fields.scsi_cmd.scsi_op_code : 0),(uint)0);
#endif
#endif
        return;
}


/*
 * NAME: sd_process_reset
 *
 * FUNCTION: Processes Successful completion of a Reset/Quiesce command
 *
 * EXECUTION ENVIRONMENT: This routine is called from sd_process_complete on
 *      the interrupt level, and can not page fault.
 *
 * (NOTES:) Possible operations : Determines the level of the command
 *      (adapter, controller, or dasd), and frees the command and then begins
 *      device verification at the appropriate level.
 *
 * (RECOVERY OPERATION:) None.
 *
 * (DATA STRUCTURES:)   sd_cmd          - Command structure
 *                      sd_adap_info    - Adapter infor structure
 *                      sd_ctrl_info    - Controller infor structure
 *                      sd_dasd_info    - DASD infor structure
 *
 * RETURNS:  Void.
 */

void sd_process_reset(
struct sd_cmd *cmd)
{
        struct sd_adap_info *ap;
        struct sd_ctrl_info *cp;
        struct sd_dasd_info *dp;
        char    reset_type;

        ap = cmd->ap;
        cp = cmd->cp;
        dp = cmd->dp;
        reset_type = cmd->mbox_copy.mb6.reset_type;
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,processreset,entry,(char)0,(uint)cmd,(uint)cmd->type, (uint)reset_type, (uint)0,(uint)0);
#endif
#endif
        switch(cmd->type) {
                case SD_ADAP_CMD:
                        /*
                         * Must have been an adapter quiesce, which
                         * completed successfully, so free command
                         * and take next step in sequence
                         */
                        /*
                         * stop the cmd timer
                         */
                        w_stop(&ap->cmd_timer.watch);
                        /*
                         * clear quiesce pending flag
                         */
                        ap->status &= ~SD_QUIESCE_PENDING;
                        ap->reset_count = 0;
                        sd_free_cmd(cmd);
			/*
			 * start delay timer to delay before allowing more
			 * commands 
			 */
			sd_delay(ap, (char)SD_ADAP_CMD, (uint)1000);
                        /*
                         * start device verification for this adapter
                         */
                        sd_verify(ap, (char)SD_ADAP_CMD);
                        break;
                case SD_CTRL_CMD:
                        /*
                         * Either a controller quiesce or a controller reset
                         * has completed, so free command and take next step
                         * in sequence
                         */
                        /*
                         * stop the cmd timer
                         */
                        w_stop(&cp->cmd_timer.watch);
                        cp->status &= ~(SD_QUIESCE_PENDING | SD_RESET_PENDING);
			/*
			 * clear reset_count
			 */
			cp->reset_count = 0;
                        sd_free_cmd(cmd);
                        /*
                         * Check the command map for this adapter, should be
                         * no commands for this controller, if not, escalate
                         */
                        if (sd_check_map(ap, cp, dp, (char)SD_CTRL_CMD)) {
                                if (reset_type == SD_RESET_CTRL_MB)
                                        /*
                                         * if this was a reset controller,
                                         * then escalate to a quiesce adapter
                                         */
                                        sd_reset_quiesce(ap,
                                                (uchar)SD_QUIESCE_OP,
                                                (uchar)SD_ADAP_CMD);
                                else
                                        /*
                                         * must of been a quiesce controller,
                                         * so escalate to a reset controller
                                         */
                                        sd_reset_quiesce(
                                                (struct sd_adap_info *)cp,
                                                (uchar)SD_RESET_OP,
                                                (uchar)SD_CTRL_CMD);
                        } else {
				/*
				 * start delay timer to delay before allowing 
				 * more commands 
				 */
				sd_delay((struct sd_adap_info *)cp, 
					(char)SD_CTRL_CMD, (uint)1);
                                /*
                                 * start device verification for this controller
                                 */
                                sd_verify((struct sd_adap_info *)cp,
                                        (char)SD_CTRL_CMD);
			}
                        break;
                case SD_DASD_CMD:
                        /*
                         * Either a DASD quiesce or a DASD reset has
                         * completed, so free command and take next step
                         * in sequence
                         */
                        /*
                         * stop the cmd timer
                         */
                        w_stop(&dp->cmd_timer.watch);
                        dp->status &= ~(SD_RESET_PENDING | SD_QUIESCE_PENDING);
			/*
			 * clear reset_count
			 */
			dp->reset_count = 0;
                        sd_free_cmd(cmd);
                        if (dp->forced_open) {
                                /*
                                 * if this was from a dasd open (forced open)
                                 * just wake him up
                                 */
                                dp->dasd_result = 0;
                                dp->forced_open = FALSE;
                                e_wakeup(&(dp->dasd_event));
                        } else {
                                /*
                                 * Check the command map for this adapter,
                                 * should be no commands for this dasd, if
                                 * not, escalate
                                 */
                                if (sd_check_map(ap,cp,dp,(char)SD_DASD_CMD)) {
                                        if (reset_type == SD_RESET_DASD_MB)
                                                /*
                                                 * if this was a reset dasd,
                                                 * then escalate to a
                                                 * quiesce controller
                                                 */
                                                sd_reset_quiesce(
                                                 (struct sd_adap_info *)dp->cp,
                                                 (uchar)SD_QUIESCE_OP,
                                                 (uchar)SD_CTRL_CMD);
                                        else
                                                /*
                                                 * must of been a quiesce dasd,
                                                 * so escalate to a reset dasd
                                                 */
                                                sd_reset_quiesce(
                                                 (struct sd_adap_info *)dp,
                                                 (uchar)SD_RESET_OP,
                                                 (uchar)SD_DASD_CMD);
                                } else {
					/*
					 * start delay timer to delay before 
					 * allowing more commands 
					 */
					sd_delay((struct sd_adap_info *)dp, 
						(char)SD_DASD_CMD, (uint)1000);
                                        /*
                                         * start device verification
                                         * for this DASD
                                         */
                                        sd_verify((struct sd_adap_info *)dp,
                                                (char)SD_DASD_CMD);
				}
                        }
                        break;
        }
#ifdef DEBUG
#ifdef SD_ERROR_PATH
        sd_trc(ap,processreset,exit,(char)0,(uint)0,(uint)0,(uint)0, (uint)0,(uint)0);
#endif
#endif
}


