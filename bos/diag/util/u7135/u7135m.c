static char sccsid[] = "@(#)55  1.9  src/bos/diag/util/u7135/u7135m.c, dsau7135, bos41J, 9519A_all 5/4/95 17:08:21";
/*
 *   COMPONENT_NAME: DSAU7135
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "u7135m.h"

/*
 *
 * NAME: main()
 *
 * FUNCTION: Call initialization routines for a Diagnostic Application (DA).
 *           Get the initial sequence of tasks for the DA and call the
 *           do_seq() function to execute the sequence of tasks.
 *
 * NOTES:
 *
 * RETURNS:  Return to the Diag Controller through clean_up() with the rc
 *           from the do_seq() call.
 *
 */

main(int argc, char **argv, char **envp) {
        int i,j,rc;
        struct sigaction act;

        setlocale(LC_ALL,"");

	fdes = -1;
	sa_flags = 0;
	dev_cfg_state = -1;
	ep = (char **)NULL;

        /* Initialize the interrupt handler for the SIGINT. */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* Initialize ODM for Diag calls. */
        rc = init_dgodm();
        if (rc == -1) {
                dt(25,"ERROR: INIT_ODM");
                clean_up(DM_ERROR_OTHER);
        }
        sa_flags |= SA_INIT_ODM;

        dt(1);

        if (argc == 3) {
                /* Command line download. */
                sa_flags |= NO_CONSOLE;
                sa_flags |= CMDLINE_DAC_DOWNLOAD;
                sa_selected = SA_uCODE_CNTRL;
		dac_selected = 0;
                strcpy(dname, argv[1]);
                strcpy(ucode.download_fname, argv[2]);
        }

        if (!(sa_flags & NO_CONSOLE)) {
                /* Initialize ASL. Allow type ahead to catch Cancel or Exit. */
                rc = diag_asl_init("DEFAULT");
                if (rc == -1) {
                        dt(25,"ERROR: ASL_INIT");
                        clean_up(DM_ERROR_OTHER);
                }
                sa_flags |= SA_INIT_ASL;
                /* Open catalog file for messages. */
                catd = diag_catopen("u7135.cat",0);
                sa_flags |= SA_INIT_CAT;
        }

        /* Initialize structs and variables. */
        seq_mgr(SEQ_CREATE); /* Create the main sequence struct. */
        main_seq = sa.seq;   /* Save a pointer to it (head ptr). */
        retry = (struct retry *)malloc(sizeof(struct retry));
        retry->step = -1;
        retry->next = (struct retry *)NULL;
        arr.spt_flag = 0;
        dev.prev_percent_complete = -1;
        dev.percent_complete = 0;

        dev_cfg_state = -1;
        arr.dar_cfg_state = -1;
        arr.lun_name[0] = '\0';
        arr.lun_cfg_state = -1;
        arr.lun_counter = 0;
        arr.linfo.num = 0;
        arr.num_spares = 0;
        arr.spare_counter = -1;
 	router.ioctl.ctrl_path[0].ctrl_name[0] = '\0';
 	router.ioctl.ctrl_path[1].ctrl_name[0] = '\0';

	for (i = 0; i < MAX_SUPPORTED_CHs; i++) {
		for (j = 0; j < MAX_SUPPORTED_IDs; j++) {
			lun_owns_pdisk[i][j] = 0;
		}
	}

	if (sa_flags & CMDLINE_DAC_DOWNLOAD) { 
        	/* Get the dac device info from CuDv. */
        	sprintf(attr, "name = %s", dname);
	} else {
        	/* Get all the dac devices from CuDv. */
        	strcpy(attr, "PdDvLn = array/scsi/dac7135");
	}
        cudv = get_CuDv_list(CuDv_CLASS, attr, &linfo, 1, 2);
        if ((cudv == (struct CuDv *)(-1)) ||
            (cudv == (struct CuDv *)NULL) ||
            (linfo.num == 0)) {
             	/* No device info. */
               	dt(25,"CUDV info not found.");
               	rc = display_screen("3068");
               	clean_up(DM_STATUS_GOOD);
        }
        arr.num_cntrls = linfo.num;

        /* Perform the tasks in the sequnce string. */
        rc = do_seq(Main_SA_Seq);

        /* Exit through clean_up() with the status from do_seq(). */
        /* The da struct will contain additional info on the      */
        /* result of the test sequence.                           */
        clean_up(rc);

        return(0);

} /* end main() */


/*
 *
 * NAME: do_seq()
 *
 * FUNCTION: Perform the tasks in the current sequence string.
 *
 * NOTES:
 *
 * RETURNS: The rc from do_erp() or DM_ERROR_ODM if no ODM data.
 *
 */

int do_seq(int seq) {
        int rc;
        int done = FALSE;
        char *temp_ptr = (char *)NULL;

        sa.seq->task_str = get_data(SEQUENCE_DATA, seq);

        if (sa.seq->task_str == (char *)NULL) {
                dt(10,1,"ERROR: DA SEQ",seq);
                clean_up(DM_ERROR_ODM);
        }
        dt(20,1,"Sequence",sa.seq->task_str);
        sa.seq->step = 1;

        sa.task = str_parse(sa.seq->task_str, " ", sa.seq->step);

        do {
                dt(10,1,"Task", atoi(sa.task));

                sa.task_rc = do_task();

                dt(10,1, "do_task() rc", sa.task_rc);

                /* The sa.task_rc will decide which ERP to perform. */
                rc = do_erp();
                dt(16,1,"do_erp() rc",rc);

                /* The rc from do_erp() will be added to the step   */
                /* counter to determine the next task to perform.   */
                sa.seq->step += rc;
                sa.task = str_parse(sa.seq->task_str, " ", sa.seq->step);

                while (!strcmp(sa.task, TASK_END_OF_SEQ)) {
                        /* No more tasks in this sequence. */
                        rc = DM_STATUS_GOOD;
                        seq_mgr(SEQ_DELETE);
                        if (sa.seq == (struct sa_seq *)NULL) {
                                done = TRUE;
                                break;
                        } else {
                                /* A task sequence finished, increment the */
                                /* previous step counter for next task.    */
                                ++sa.seq->step;
                                sa.task = str_parse(sa.seq->task_str, " ",
                                                    sa.seq->step);
                        }
                }
        } while (!done && (rc != ERP_FAIL) && (rc != DM_ERROR_UNEXPECTED));

        return(rc);

} /* end do_seq() */

/*
 * NAME: seq_mgr()
 *
 * FUNCTION: Manage a dynamic linked list of sequences. Create and delete
 *           sequence structs to allow multiple or recursive ERP sequences.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void seq_mgr(int action) {
        struct sa_seq *seq, *back;

        if (action == SEQ_CREATE) {
                seq = (struct sa_seq *)calloc(1,sizeof(struct sa_seq));
                if (sa.seq == (struct sa_seq *)NULL) {
                        seq->next = (struct sa_seq *)NULL;
                } else {
                        seq->next = sa.seq->next;
                        sa.seq->next = seq;
                }
                sa.seq = seq;
                sa.seq->step = 0;
                dt(25,"Create Sequence");
        } else if (action == SEQ_DELETE) {
                /* Find the previous sequence. */
                back = seq = main_seq;
                while((seq != sa.seq) && (seq != (struct sa_seq *)NULL)) {
                        back = seq;
                        seq = seq->next;
                }
                if (back == seq) {
                        sa.seq = main_seq = (struct sa_seq *)NULL;
                        dt(25,"Delete Main Sequence");
                } else {
                        back->next = seq->next;
                        sa.seq = back;
                        dt(25,"Delete Sequence");
                }
                free(seq);
        }

        return;

} /* end seq_mgr() */

/*
 *
 * NAME: do_task()
 *
 * FUNCTION: Call the routine for the task type (cfg, open, etc).
 *
 * NOTES:
 *
 * RETURNS: Return code from the task called.
 *
 */

int do_task(void) {
        int rc = DM_STATUS_GOOD;
        int rs_rc;
        int task_num;
        char *ptr;
        char *temp_ptr;

        dt(25,"In do_task()");

        /* Determine the task type and call the appropriate routine. */
        if (TASK_IS(CONFIG_TASK,sa.task)) {
                dt(25,"Config device");
                rc = config(sa.task);
                dt(10,1,"     rc",rc);
        } else if (TASK_IS(OPEN_TASK,sa.task)) {
                rc = dev_open(sa.task);
        } else if (TASK_IS(SCREEN_TASK,sa.task)) {
                rc = display_screen(sa.task);
        } else if (TASK_IS(SCARRY_TASK,sa.task)) {
                init_tucb(sa.task);
                dt(2,&tucb);
                if (arr.spt_flag) {
                        rc = ptrs_ioctl(sa.task);
                        dt(10,1,"  DKIOCMD_PTRS2 rc", rc);
                } else {
                        rc = rs_ioctl(sa.task);
                        dt(10,1,"  DKIOCMD_RS rc", rc);
                }
                if (CHECK_CONDITION(rc)) {
                        dt(30,0xFF,sa.sdata);
                } else {
			dt(30,tucb.scsitu.data_length,
			   tucb.scsitu.data_buffer);
		}
        } else if (TASK_IS(GET_ROUTER_TASK,sa.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(sa.task));
                sa.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
                rc = ioctl(fdes, RT_GET_ROUTER_STATE, &router.ioctl);
        } else if (TASK_IS(SET_ROUTER_TASK,sa.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(sa.task));
                sa.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
                rc = ioctl(fdes, RT_SET_ROUTER_STATE, &router.ioctl);
        } else if (TASK_IS(SCATU_TASK,sa.task)) { /* LUN Certify. */
                init_tucb(sa.task);
                dt(2,&tucb);
                rc = exectu(fdes, &tucb);
                dt(10,1,"exectu() rc",rc);
                if (CHECK_CONDITION(rc)) {
                        rs_rc = req_sense();
                        if (rs_rc != SCATU_GOOD) {
                                dt(10,1,"ERROR: REQUEST SENSE rc", rc);
                                return(ERP_FAIL);
                        }
                } else {
                        dt(30,tucb.scsitu.data_length,
                              tucb.scsitu.data_buffer);
                }
        } else if (TASK_IS(MISC_TASK,sa.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(sa.task));
                sa.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
                rc = DM_STATUS_GOOD;
        } else {
                rc = DM_ERROR_OTHER;
        }

        return(rc);

} /* end do_task() */

/*
 *
 * NAME: do_erp()
 *
 * FUNCTION: Call test_condition() until a true condition is found.
 *           Call perform_reactions() to perfrom the reaction associated
 *           with the true condition.
 *
 * NOTES:
 *
 * RETURNS: Value used to increment the task step counter.
 *
 */

int do_erp(void) {
        int rc;
        int found = FALSE;
        int erp_count = 0;
        char *c_code = (char *)NULL;
        char *r_code = (char *)NULL;

        /* Search the ERP data for a true condition. */
        do {
                /* Get the ERP condition code for this task. */
                c_code = get_erp_code(++erp_count, CONDITION);
                if (c_code == (char *)NULL) {
                        dt(25,"ERROR: COND CODE = NULL");
                        break;
                }
                /* Test if the condition code is true. */
                found = test_condition(c_code);

        } while (!found);

        if (!found) {
                dt(25,"ERROR: COND CODE");
                clean_up(DM_ERROR_OTHER);
        }

        /* Get the ERP reaction code for this task. */
        r_code = get_erp_code(erp_count, REACTION);

        dt(20,2,"In do_erp(), condition",c_code, "reaction", r_code);

        /* Perform the reaction(s) associated with the condition    */
        /* found to be true. This includes reacting to good status. */
        rc = perform_reactions(r_code);

        return(rc);

} /* end do_erp() */

/*
 *
 * NAME: test_conditoins()
 *
 * FUNCTION: Test each condition in the ERP data until a condition
 *           is found to be true or no more conditions exist.
 *
 * NOTES:
 *
 * RETURNS: TRUE if a condition is found to be true.
 *          FALSE if no condition is found to be true.
 *
 */

int test_condition(char *c_code) {
        int i, rc, found = FALSE;
        int condition_code;
        char *buff;

        /* dt(25,"In test_condition()"); */
        /*
         * Try and match the current device condition with
         * one of the condition codes. If a match is found,
         * set found = TRUE and return.
         */
        if (strlen(c_code) == 6) {
                /*
                 * CAC condition code, Check sense key,
                 * ASC, and ASCQ. X's means dont care.
                 */
                if (CHECK_CONDITION(sa.task_rc)) {
                        /*
                         * Put the sense key and sense code
                         * in buff and copy any X's in c_code
                         * to buff. Then compare buff and
                         * c_code. If they are the same,
                         * then set found equal to TRUE.
                         */
                        buff = (char *)calloc(1,
                                       strlen(c_code)+1);
                        sprintf(buff, "%02.2X%04.4X",
                                sa.skey, sa.scode);
                        for(i=0;i<strlen(c_code);i++) {
                                if (c_code[i] == 'X')
                                        buff[i] = 'X';
                        }
                        if (!strcmp(buff, c_code)) {
                                found = TRUE;
                        }
                        free(buff);
                }
        } else {
                condition_code = atoi(c_code);
                /****************************************************/
                /* This switch statement is used for the 4 digit    */
                /* condition codes (refer to the CIS).              */
                /****************************************************/
                switch(condition_code) {
        /* Common condition codes. */
                case 1: /* DM_STATUS_GOOD */
                        if (sa.task_rc == DM_STATUS_GOOD)
                                found = TRUE;
                        break;
                case 2:
                        if (sa.task_rc == DM_STATUS_BAD)
                                found = TRUE;
                        break;
                case 3:
                        if (sa.task_rc != DM_STATUS_GOOD)
                                found = TRUE;
                        break;
        /* ASL condition codes. */
                case 4: /* User selected a valid item. */
                        if (item_selected > 0)
                                found = TRUE;
                        break;
                case 5: /* User selected item #1. */
                        if (sa.task_rc == 1)
                                found = TRUE;
                        break;
                case 6: /* User selected item #2. */
                        if (sa.task_rc == 2)
                                found = TRUE;
                        break;
                case 7: /* User selected item #3. */
                        if (sa.task_rc == 3)
                                found = TRUE;
                        break;
                case 8: /* User selected item #4. */
                        if (sa.task_rc == 4)
                                found = TRUE;
                        break;
                case 9:
                        if ((sa.task_rc == DIAG_ASL_OK) ||
                            (sa.task_rc == DIAG_ASL_ENTER))
                                found = TRUE;
                        break;
                case 10:
                        if ((sa.task_rc == DM_USER_CANCEL) ||
                            (sa.task_rc == DM_USER_EXIT))
                                found = TRUE;
                        break;
        /* 7135 Array Conditions. */
		case 11: 
			if ((sa.task_rc == SCATU_GOOD) && 
			    (ucode.flags & DL_ALL_PDISKS) &&
			    (arr.spare_counter < arr.num_spares))
				found = TRUE;
			break;
                case 12:
                        if ((sa.task_rc == SCATU_GOOD) && CNTRL_IS_ACTIVE)
                                found = TRUE;
                        break;
                case 13:
                        if ((sa.task_rc == SCATU_GOOD) && !(CNTRL_IS_ACTIVE))
                                found = TRUE;
                        break;
		case 14: 
			if ((sa.task_rc == SCATU_GOOD) && 
			    (ucode.flags & DL_ALL_PDISKS) &&
			    (arr.spare_counter == arr.num_spares))
				found = TRUE;
			break;
		case 15: 
			if (ucode.flags & DL_ALL_PDISKS)
				found = TRUE;
			break;
                case 17:
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
                            (arr.lun_counter < arr.linfo.num))
                                found = TRUE;
                        break;
                case 18:
                        if ((sa_flags & NO_CONSOLE) &&
			    (sa_selected == SA_uCODE_CNTRL))
                                found = TRUE;
                        break;
                case 19:
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
                            (arr.spare_counter < arr.num_spares))
                                found = TRUE;
                        break;
                case 20:
                        if ((sa_selected == SA_uCODE_PDISK) &&
			    (sa.task_rc == DM_USER_CANCEL))
                                found = TRUE;
                        break;
                case 21:
                        if (sa.task_rc == DM_ERROR_EINVAL)
                                found = TRUE;
                        break;
                case 23:
                        if (sa_selected == SA_CERTIFY_LUN)
                                found = TRUE;
                        break;
                case 24:
                        if (sa_selected == SA_CERTIFY_PDISK)
                                found = TRUE;
                        break;
                case 25:
                        if (sa_selected == SA_FORMAT_PDISK)
                                found = TRUE;
                        break;
                case 26:
                        if (sa_selected == SA_uCODE_CNTRL)
                                found = TRUE;
                        break;
                case 27:
                        if (sa_selected == SA_uCODE_PDISK)
                                found = TRUE;
                        break;
                case 28:
                        if (sa_selected == SA_UPDATE_EEPROM)
                                found = TRUE;
                        break;
                case 29:
                        if (sa_selected == SA_REPLACE_CNTRL)
                                found = TRUE;
                        break;
                case 30:
                        if (CHECK_CONDITION(sa.task_rc) && NON_ZERO_FRUC)
                                found = TRUE;
                        break;
                case 31:
                        if (CHECK_CONDITION(sa.task_rc) &&
                            NOT_READY_NO_uCODE)
                                found = TRUE;
                        break;
                case 33:
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
                            (arr.cntrl_counter < arr.num_cntrls))
                                found = TRUE;
                        break;
                case 34:
                        if ((sa.task_rc == SCATU_GOOD) &&
                            (dev.current_lba < dev.last_lba) &&
                            (dev.percent_complete ==
                             dev.prev_percent_complete))
                                found = TRUE;
                        break;
                case 35:
                        if (arr.num_spares < 1)
                                found = TRUE;
                        break;
                case 36:
                        if ((arr.num_luns < 1) &&
                            (sa_selected == SA_CERTIFY_LUN))
                                found = TRUE;
                        break;
                case 38:
                        if (dev.current_lba >= dev.last_lba)
                                found = TRUE;
                        break;
                case 39: /* Check for non-spare drive for pdisk download. */
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
                            !(arr.pdisk_ptr->status_byte & SPARE))
                                found = TRUE;
                        break;
                case 40:
                        if (dev.current_lba < dev.last_lba)
                                found = TRUE;
                        break;
		case 41:
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
			    (ucode.flags & DL_ALL_PDISKS)) 
                                found = TRUE;
			break;
		case 42:
			if((strlen(router.ioctl.ctrl_path[0].ctrl_name) == 0)||
			   (strlen(router.ioctl.ctrl_path[1].ctrl_name) == 0))
                                found = TRUE;
			break;
                case 43:
                        if ((sa.task_rc == SCATU_GOOD) &&
                            (ucode.block_counter < (ucode.num_blocks - 1)))
                                found = TRUE;
                        break;
                case 44:
                        if ((sa.task_rc == SCATU_GOOD) &&
                            (ucode.block_counter == (ucode.num_blocks - 1)) &&
                            (sa_selected == SA_uCODE_CNTRL))
                                found = TRUE;
                        break;
        /* Retry condition codes. */
                case 45:
                        if ((rc = retry_mgr(CHECK_RETRY, 0)) == ERP_GOOD)
                                found = TRUE;
                        break;
        /* Config condition codes. */
                case 46:
                        if (sa.task_rc == DEV_CFG_FAILED)
                                found = TRUE;
                        break;
                case 49:
                        if (sa.task_rc == DM_ERROR_OPEN)
                                found = TRUE;
                        break;
        /* TU condition codes. */
                case 51:
                        if (sa.task_rc == SCATU_TIMEOUT)
                                found = TRUE;
                        break;
                case 52:
                        if (sa.task_rc == SCATU_RESERVATION_CONFLICT)
                                found = TRUE;
                        break;
                case 53:
                        if (CHECK_CONDITION(sa.task_rc))
                                found = TRUE;
                        break;
                case 54:  /* Also covers SCATU_BUSY. */
                        if (sa.task_rc == SCATU_COMMAND_ERROR)
                                found = TRUE;
                        break;
                case 55:
                        if (sa.task_rc == SCATU_IO_BUS_ERROR)
                                found = TRUE;
                        break;
                case 56:
                        if (sa.task_rc == SCATU_ADAPTER_FAILURE)
                                found = TRUE;
                        break;
                case 57:
                        if ((sa.task_rc == DM_STATUS_GOOD) &&
                            ((tucb.scsitu.data_buffer[10] != 0x02) &&
                             (tucb.scsitu.data_buffer[11] != 0x00)) &&
                            (sa_selected == SA_CERTIFY_PDISK))
                                found = TRUE;
                        break;
                case 58: /* Previous task sequence not complete. */
                        if (main_seq != sa.seq)
                                found = TRUE;
                        break;
                case 60:
                        found = TRUE; /* ANY CONDITION */
                        break;
                case 61:
                        if (CHECK_CONDITION(sa.task_rc) && SUBSYS_FRUC)
                                found = TRUE;
                        break;
                case 62: /* dac held-in-reset. */
                        if ((DAC_ID_ODD(dac_scsi_id) &&
                             (router.ioctl.state == ACTIVE_RESET)) ||
                            ((!DAC_ID_ODD(dac_scsi_id)) &&
                             (router.ioctl.state == RESET_ACTIVE)))
                                found = TRUE;
                        break;
                case 63:
                        if ((sa.task_rc == SCATU_GOOD) &&
                            (sa_selected == SA_uCODE_PDISK))
                                found = TRUE;
                        break;
                case 64:
                        if ((sa.task_rc == SCATU_GOOD) &&
                            ((sa_selected == SA_FORMAT_PDISK) ||
                             (sa_selected == SA_CERTIFY_PDISK)))
                                found = TRUE;
                        break;
                case 65: /* Paired dac is held-in-reset. */
                        if ((DAC_ID_ODD(dac_scsi_id) &&
                             (router.ioctl.state == RESET_ACTIVE)) ||
                            ((!DAC_ID_ODD(dac_scsi_id)) &&
                             (router.ioctl.state == ACTIVE_RESET)))
                                found = TRUE;
                        break;
                default:
                        found = FALSE;
                        break;
                } /* end switch */
        }

        return(found);

} /* end test_condition() */

/*
 *
 * NAME: get_erp_code
 *
 * FUNCTION: Get an ERP (condition_code:reaction_code) from the ERP string.
 *
 * NOTES:
 *
 * RETURNS: A pointer to the ERP string.
 *
 */

char *get_erp_code(int erp_count, int type) {
        char *ptr = (char *)NULL;
        char *temp_ptr = (char *)NULL;

        /* dt(25,"In get_erp_code()"); */
        /* Get an ERP (condition_code:reaction_code) from the ERP string. */
        temp_ptr = str_parse(sa.erp_str, " ", erp_count);

        /* Get the condition or reaction code from the ERP. */
        ptr = str_parse(temp_ptr, ":", (type & 0x0F));

        if (ptr == (char *)NULL) {
                dt(25,"ERROR: ERP CODE");
                clean_up(DM_ERROR_OTHER);
        }

        return(ptr);

} /* end get_erp() */

/*
 *
 * NAME: perform_reactions()
 *
 * FUNCTION: Perform the reaction(s) associated with a true condition.
 *
 * NOTES:
 *
 * RETURNS: Return code of the reaction performed.
 *
 */

int perform_reactions(char *r_code_str) {
        char *r_ptr;
        int i,rc;
        int erp_rc = ERP_FAIL;
        int erp_continue = TRUE;
        int reaction_code;
        int mult_reactions = FALSE;
        int count = 1;
        char *ms_buffer;
        struct CuAt cuat;
        char *outbuf = (char *)NULL;
        char *errbuf = (char *)NULL;
        struct lun *lun_ptr;
        char buff[128];

        /* dt(25,"In perform_reactions"); */

        r_ptr = str_parse(r_code_str, ",", count++);
        if (r_ptr == (char *)NULL) {
                dt(25,"ERROR: r_code == NULL");
                clean_up(DM_ERROR_OTHER);
        }

        do {
                reaction_code = atoi(r_ptr);

                dt(10,1,"Reaction code", reaction_code);

                /****************************************************/
                /* This switch statement is used for all reactions. */
                /****************************************************/
                switch(reaction_code) {
                case 1: /* Continue testing. */
                        erp_rc = retry_mgr(CLEAR_RETRY, 0);
                        break;
                case 2: /* Retry task. */
                        erp_rc = ERP_RETRY;
                        break;
                case 3: /* Restart DA */
                        while (sa.seq != (struct sa_seq *)NULL)
                                seq_mgr(SEQ_DELETE);
                        seq_mgr(SEQ_CREATE);
                        main_seq = sa.seq;   /* Save the pointer (head ptr). */
                        sa.seq->task_str = get_data(SEQUENCE_DATA,
                                                    Main_SA_Seq);
                        if (sa.seq->task_str == (char *)NULL) {
                                dt(25,"ERROR: TASK");
                                return(DM_ERROR_ODM);
                        }
                        sa.seq->step = 0;
                        erp_rc = ERP_GOOD;
                        dt(25,"Restart sa.");
                        break;
                case 4: /* Set device reserved flag. This will cause  */
                        /* a RELEASE_UNIT to be issued in clean_up(). */
                        if (sa_selected == SA_CERTIFY_LUN) {
                                sa_flags |= SA_RESERVED_LUN;
                        } else {
                                sa_flags |= SA_RESERVED_PDISK;
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 5: /* Reset step count to begining of task sequence. */
                        sa.seq->step = 0;
                        erp_rc = ERP_GOOD;
                        break;
		case 6: /* Set the download to all pdisks flag. */
			ucode.flags |= DL_ALL_PDISKS;
                        erp_rc = ERP_GOOD;
			break;
		case 7: 
			arr.spare_counter = 0;
                        erp_rc = ERP_GOOD;
                        break;
		case 8: 
			++arr.spare_counter;
			item_selected = arr.spare_counter;
                        erp_rc = ERP_GOOD;
                        break;
		case 9: /* Clear the download to all disks flag (if set). */
			if (ucode.flags & DL_ALL_PDISKS)
				ucode.flags ^= DL_ALL_PDISKS;
                        erp_rc = ERP_GOOD;
                        break;
                case 14: /* Call certify routine. */
                        rc = raid_certify();
                        erp_rc = ERP_GOOD;
                        break;
                case 15: /* Change state to make this dac held-in-reset and */
			 /* change the LUN ownership to the other dac.      */
                        dt(16,1,"router state (before)", router.ioctl.state);
                        dt(16,1,"LUN ownership (before)", 
			   router.ioctl.lun_ownership);
                        if (DAC_ID_ODD(dac_scsi_id)) {
                                router.ioctl.state = ACTIVE_RESET;
                                router.ioctl.lun_ownership = LUN_OWN_EVEN;
                        } else {
                                router.ioctl.state = RESET_ACTIVE;
                                router.ioctl.lun_ownership = LUN_OWN_ODD;
                        }
                        dt(16,1,"router state (after)", router.ioctl.state);
                        dt(16,1,"LUN ownership (after)", 
			   router.ioctl.lun_ownership);
                        erp_rc = ERP_GOOD;
                        break;
                case 16:
                        dev.lun = atoi((arr.cudv+arr.lun_counter)->connwhere);
                        dt(10,1,"device LUN",dev.lun);
                        erp_rc = ERP_GOOD;
                        break;
                case 17: /* Mark pdisk as failed (download). */
                        arr.pdisk_ptr->flags = PDISK_DL_FAILED;
                        erp_rc = ERP_GOOD;
                        break;
                case 18: /* Mark pdisk as passed (download). */
                        arr.pdisk_ptr->flags = PDISK_DL_PASSED;
                        erp_rc = ERP_GOOD;
                        break;
                case 20: /* Get array info from CuAt and CuDv.     */
                        /* Get the dar name from CuAt for this dac device. */
                        sprintf(attr, "name LIKE dar* and value LIKE '*%s*' and attribute = all_controller",
                                dname);
                        if (odm_get_first(CuAt_CLASS, attr, &cuat)) {
                                strcpy(arr.dar_name, cuat.name);
                        } else {
                                dt(25,"ERROR: dar not found in CuAt.");
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        erp_rc = DM_STATUS_GOOD;
                        break;
                case 21: /* Add hdisk to array status. */
                        erp_rc = raid_array_status(LUN_STATUS);
                        if (fdes != -1) {
                                close(fdes);
                        }
                        if (arr.lun_cfg_state == DEFINED) {
                                arr.lun_cfg_state = -1;
                                sprintf(attr, " -l %s",
                                        (arr.cudv + arr.lun_counter)->name);
                                rc = run_method(
                                        (arr.cudv + arr.lun_counter)->name,
                                        UCFG_METHOD, attr);
                                dt(10,1,"uncfg rc", rc);
                        }
                        dt(10,2,"lun counter",arr.lun_counter,"linfo",
                           arr.linfo.num);
                        break;
                case 22: /* Add spare disk to array status. */
                        arr.spare_counter = 0;
                        erp_rc = raid_array_status(SPARE_STATUS);
                        break;
                case 23: /* Add dead hdisk to array status. */
                        erp_rc = raid_array_status(LUN_STATUS | DEAD_LUN);
                        if (fdes != -1) {
                                close(fdes);
                        }
                        arr.lun_cfg_state = -1;
                        ++arr.lun_counter;
                        dt(10,2,"lun counter",arr.lun_counter,"linfo",
                           arr.linfo.num);
                        break;
                case 24: /* Add hdisk open error to array status. */
                        erp_rc = raid_array_status(LUN_STATUS | OPEN);
                        break;
                case 25:
                        ++arr.lun_counter;
                        dt(10,2,"lun counter",arr.lun_counter,"linfo",
                           arr.linfo.num);
                        erp_rc = ERP_GOOD;
                        break;
                case 26: /* Configire the dar. */
                        if (arr.dar_cfg_state == -1) {
                                arr.dar_cfg_state = get_cfg_state(arr.dar_name);
                                dt(10,1,"dar_cfg_state", arr.dar_cfg_state);
                        }
                        sprintf(attr, " -l %s", arr.dar_name);
                        rc = run_method(arr.dar_name, CFG_METHOD, attr);
                        dt(10,1,"cfgdar rc", rc);
                        erp_rc = ERP_GOOD;
                        break;
                case 27: /* Run the dac config method to update ODM. */
                        sprintf(attr, " -l %s", dname);
                        rc = run_method(dname, CFG_METHOD, attr);
                        dt(10,1,"  cfgdac rc", rc);
                        erp_rc = ERP_GOOD;
                        break;
                case 28: /* Get the CH/ID of the next spare disk. */
                        lun_ptr = raid_hptr;
                        while ((lun_ptr != (struct lun *)NULL) &&
                               (strcmp(lun_ptr->name, "SPARE"))) {
                                lun_ptr = lun_ptr->next;
                        }
                        arr.pdisk_ptr = lun_ptr->pdisk;
                        for (i = 0; i < arr.spare_counter; i++) {
                                arr.pdisk_ptr = arr.pdisk_ptr->next;
                        }
                        arr.pdisk_ch = arr.pdisk_ptr->ch;
                        arr.pdisk_id = arr.pdisk_ptr->id;
                        /* dt(10,1,"spare counter",arr.spare_counter); */
                        ++arr.spare_counter;
                        erp_rc = ERP_GOOD;
                        break;
                case 29: /* Determine the spare disk type from Inquiry data. */
                        strcpy(arr.pdisk_ptr->capacity,
                               get_pdisk_capacity(arr.dar_name,
                                                  arr.pdisk_ptr->ch,
                                                  arr.pdisk_ptr->id));
                        erp_rc = ERP_GOOD;
                        break;
		case 30:
			dac_selected = (item_selected -1); 
                        erp_rc = ERP_GOOD;
                        break;
                case 31: /* Get LUN name. */
                        strcpy(arr.lun_name,(arr.cudv+arr.lun_counter)->name);
                        dt(20,1,"arr.lun_name", arr.lun_name);
                        erp_rc = ERP_GOOD;
                        break;
                case 34: /* Set LUN value for dac device. */
			if (diag_get_sid_lun((cudv+dac_selected)->connwhere, 
			    &dac_scsi_id, &lun_id) == -1){
				dt(25,"ERROR: DAC SCSI ID");
				clean_up(DM_ERROR_UNEXPECTED);
			}
                        dev.lun = lun_id;
                        dt(10,1,"device LUN",dev.lun);
                        erp_rc = ERP_GOOD;
                        break;
                case 36:
                        if (sa_selected == SA_CERTIFY_LUN) {
                                sa_flags |= SA_RESERVED_LUN;
                        } else {
                                sa_flags |= SA_RESERVED_PDISK;
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 37:
                        sa_selected = item_selected;
                        dt(10,1,"SA Selected",sa_selected);
                        erp_rc = ERP_GOOD;
                        break;
                case 38: /* Get info for device selected. */
                        strcpy(dev.location,
                               (char *)raid_get_menu_item((item_selected - 1),
                                                  GET_DEV_INFO));
                        erp_rc = ERP_GOOD;
                        break;
                case 39:
                        /* Get the name of the dac and its parent. */
                        strcpy(dname, (cudv+dac_selected)->name);
                        strcpy(dnameloc, (cudv+dac_selected)->location);
                        strcpy(parent, (cudv+dac_selected)->parent);
                        dt(20,2,"dname", dname, "parent", parent);
                        erp_rc = ERP_GOOD;
                        break;
                case 40:
                        dev.last_lba =
                             (unsigned)((tucb.scsitu.data_buffer[0] << 24) |
                                        (tucb.scsitu.data_buffer[1] << 16) |
                                        (tucb.scsitu.data_buffer[2] << 8)  |
                                        (tucb.scsitu.data_buffer[3]));
                        dev.block_size =
                             (unsigned)((tucb.scsitu.data_buffer[4] << 24) |
                                        (tucb.scsitu.data_buffer[5] << 16) |
                                        (tucb.scsitu.data_buffer[6] << 8)  |
                                        (tucb.scsitu.data_buffer[7]));
                        dt(16,1,"last LBA", dev.last_lba);
                        dt(16,1,"block size", dev.block_size);
                        erp_rc = ERP_GOOD;
                        break;
                case 41:
                        dev.current_lba += CERTIFY_READ_SIZE;
                        if (dev.current_lba > dev.last_lba) {
                                dev.current_lba = dev.last_lba;
                        }
                        dev.prev_percent_complete = dev.percent_complete;
                        dev.percent_complete =
                               (100L * dev.current_lba) / dev.last_lba;
                        dt(10,1,"percent complete", dev.percent_complete);
                        erp_rc = ERP_GOOD;
                        break;
                case 42: /* Save the current MODE SENSE data. */
                        current_mode_data = (uchar *)calloc(1,
                                                 tucb.scsitu.data_buffer[0]);
                        memcpy(current_mode_data,
                               tucb.scsitu.data_buffer,
                               (unsigned)tucb.scsitu.data_buffer[0]);
                        erp_rc = ERP_GOOD;
                        break;
                case 43: /* Save the changable MODE SENSE data. */
                        ch_mode_data = (uchar *)calloc(1,
                                                 tucb.scsitu.data_buffer[0]);
                        memcpy(ch_mode_data,
                               tucb.scsitu.data_buffer,
                               (unsigned)tucb.scsitu.data_buffer[0]);
                        erp_rc = ERP_GOOD;
                        break;
                case 44:
                        ucode.flags |= DOWNLOAD_FROM_DISKETTE;
                        erp_rc = ERP_GOOD;
                        break;
                case 45: /* Build controller ucode file name. */
                        if (tucb.scsitu.scsi_cmd_blk[2] == 0) {
                                strcpy(ucode.current_fname,
                                       "/etc/array/fw/");
                                strncat(ucode.current_fname,
                                        &tucb.scsitu.data_buffer[16], 8);
                                rc = strlen(ucode.current_fname);
                                ucode.current_fname[rc+1] = '\0';
                        } else if (tucb.scsitu.scsi_cmd_blk[2] == 0xC2) {
                                /* Page C2, Software revision page. */
                                sprintf(buff, ".%02.2X.%02.2X",
                                        tucb.scsitu.data_buffer[8],
                                        tucb.scsitu.data_buffer[10]);
				strcat(ucode.current_fname, buff);
                        }
                        dt(20,1,"current file name",ucode.current_fname);
                        erp_rc = ERP_GOOD;
                        break;
                case 46: /* Build physical disk ucode file name. */
                        if (tucb.scsitu.data_buffer[1] == 0) {
                                /* First INQUIRY */
                                strcpy(ucode.current_fname, "/etc/array/fw/");
                                /* Product Type (bytes 16 - 19, ASCII) */
                                strncat(ucode.current_fname,
                                        &tucb.scsitu.data_buffer[16],4);
                                /* Model Number (bytes 20 - 22, ASCII) */
                                strncat(ucode.current_fname,
                                        &tucb.scsitu.data_buffer[20],3);
                                strcat(ucode.current_fname, ".");
                        } else {
                                /* Second INQUIRY, get LoadID and rev level */
                                sprintf(buff,
                          "%02.2X%02.2X%02.2X%02.2X.%02.2X%02.2X%02.2X%02.2X",
                                      /*  "08091011.12131415" */
                                     (unsigned)tucb.scsitu.data_buffer[8],
                                     (unsigned)tucb.scsitu.data_buffer[9],
                                     (unsigned)tucb.scsitu.data_buffer[10],
                                     (unsigned)tucb.scsitu.data_buffer[11],
                                     (unsigned)tucb.scsitu.data_buffer[12],
                                     (unsigned)tucb.scsitu.data_buffer[13],
                                     (unsigned)tucb.scsitu.data_buffer[14],
                                     (unsigned)tucb.scsitu.data_buffer[15]);
                                strcat(ucode.current_fname, buff);
				sprintf(arr.pdisk_ptr->ucode_level,
					"(%02.2X%02.2X)",
					(unsigned)tucb.scsitu.data_buffer[14],
					(unsigned)tucb.scsitu.data_buffer[15]);
                        }
                        dt(20,1,"current file name",ucode.current_fname);
                        erp_rc = ERP_GOOD;
                        break;
                case 47: /* Get ucode file names. */
                        rc = get_ucode_fname();
			dt(10,1,"rc from get_ucode_fname()", rc);
                        switch(rc) {
                        case DEF_SELECTION:
                        case DM_USER_CANCEL:
                        case DM_ERROR_OTHER:
                                if (sa_selected == SA_uCODE_PDISK) {
                                        /* Restart Sequence. */
                                        sa.seq->step = 0;
                                        erp_continue = FALSE;
					if (ucode.flags & DL_ALL_PDISKS) {
						ucode.flags ^= DL_ALL_PDISKS;
					}
                                } else {
                                        clean_up(DM_STATUS_GOOD);
                                }
                                break;
			case DM_STATUS_GOOD:
                                break;
                        case DM_USER_EXIT:
                        default:
                                clean_up(DM_STATUS_GOOD);
                                break;
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 48:
                        ucode.flags |= DOWNLOAD_PREVIOUS_LEVEL;
                        erp_rc = ERP_GOOD;
                        break;
                case 49:
                        sleep(15);
                        erp_rc = ERP_GOOD;
                        break;
                case 50: /* Update format percent complete. */
                        dev.percent_complete =
                                ((unsigned) sa.sdata[16]) << 8 |
                                ((unsigned) sa.sdata[17]);
                        dev.percent_complete *= 100;
                        dev.percent_complete /= 65536;
                        erp_rc = ERP_GOOD;
                        break;
                case 51:
                        ucode.read_size = 0;
                        ++ucode.block_counter;
                        fclose(ucode.fptr);
                        erp_rc = ERP_GOOD;
                        break;
                case 52: /* Open the ucode file. */
                        if (ucode.flags & DOWNLOAD_FROM_DISKETTE) {
                                /* Get ucode file from diskette. */
                                sprintf(attr,
                                  "%s -xqSf/dev/rfd0 %s 1>/dev/null 2>&1",
				  RESTBYNAME,
                                  ucode.download_fname);
                                rc = system(attr);
                        }
                        ucode.fptr = fopen(ucode.download_fname, "r");
                        if (ucode.fptr == NULL) {
				dt(25,"ERROR: Download file.");
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 53: /* Get the ucode file info (size, blocks, etc). */
                        ucode.block_counter = -1;
                        ucode.max_block_size = 0x8000;
                        ucode.fbuff = (char *)malloc(0x8000);
                        rc = stat(ucode.download_fname, &ucode.fstat);
                        if (rc == -1) {
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        /* Determine number of blocks */
                        ucode.num_blocks =
                                ucode.fstat.st_size / ucode.max_block_size;
                        ucode.block_resid =
                                ucode.fstat.st_size % ucode.max_block_size;
                        if (ucode.block_resid > 0) {
                                ++ucode.num_blocks;
                        }
                        dt(10,1,"ucode blocks", ucode.num_blocks);
                        erp_rc = ERP_GOOD;
                        break;
                case 54: /* Read a 32k block of ucode (pdisk). */
                        ++ucode.block_counter;
                        if ((ucode.block_resid > 0) &&
                            (ucode.block_counter == (ucode.num_blocks - 1))) {
                                ucode.read_size = ucode.block_resid;
                        } else {
                                ucode.read_size = ucode.max_block_size;
                        }
                        rc = fread(ucode.fbuff, ucode.read_size, 1, ucode.fptr);
                        if (sa_selected == SA_uCODE_PDISK) {
                                ucode.read_size = ucode.max_block_size;
                        }
                        if (rc <= 0) {
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 55:
                        if (item_selected == 2) {
                                ucode.flags |= DOWNLOAD_PREVIOUS_LEVEL;
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 56:
                        strcat(ucode.current_fname, ".02.00");
                        erp_rc = ERP_GOOD;
                        break;
                case 57:
                        arr.spt_flag = 1;
                        erp_rc = ERP_GOOD;
                        break;
                case 58:
                        arr.spt_flag = 0;
                        erp_rc = ERP_GOOD;
                        break;
                case 80: /* Save current EEPROM data (64 bytes). */
                        /* dt(25,"save current??"); */
                        erp_rc = ERP_GOOD;
                        break;
                case 81: /* Get latest level EEPROM data (64 bytes). */
                        erp_rc = get_eeprom_fptr();
                        if (erp_rc != DM_STATUS_GOOD) {
                                dt(25,"ERROR: GET EEPROM failed");
                                clean_up(DM_STATUS_BAD);
                        }
                        for (i=0; i<64; i++) {
                                if (feof(ucode.fptr)) {
                                        dt(25,"ERROR: EEPROM file EOF");
					display_screen("3185");
                                        clean_up(DM_STATUS_GOOD);
                                }
                                eeprom_buff[i] = (uchar)fgetc(ucode.fptr);
                        }
                        dt(25,"EEPROM BUFF");
                        dt(30, 64, eeprom_buff);
                        erp_rc = ERP_GOOD;
                        break;
                case 67: /* Clear retry entry for this step. */
                        erp_rc = retry_mgr(CLEAR_RETRY, 0);
                        break;
                case 68: /* Go ahead one task in the task sequence. */
                        sa.seq->step += 1;
                        erp_rc = ERP_GOOD;
                        break;
                case 69: /* Go back one task in the task sequence. */
                        sa.seq->step -= 1;
                        erp_rc = ERP_RETRY;
                        break;
                case 70: /* Exit DA with good status. */
                        clean_up(DM_STATUS_GOOD);
                        break;
                case 73: /* Exit DA with error open. */
                        clean_up(DM_ERROR_OPEN);
                        break;
                case 74: /* Exit the SA with a Software Error. */
                        clean_up(DM_ERROR_OTHER);
                        break;
                case 75: /* Exit DA with unexpected error. */
                        clean_up(DM_ERROR_UNEXPECTED);
                        break;
                case 76: /* Exit DA with ODM error. */
                        clean_up(DM_ERROR_ODM);
                        break;
                case 84: /* Get all the array LUN's (hdisks) from CuDv. */
                        sprintf(attr, "parent = %s AND chgstatus != 3",
                                arr.dar_name);
                        arr.cudv = get_CuDv_list(CuDv_CLASS, attr,
                                                 &arr.linfo, 1, 2);
                        if ((arr.cudv == (struct CuDv *)(-1)) ||
                            (arr.cudv == (struct CuDv *)NULL)) {
                                /* May be a valid condition. */
                                dt(25,"No LUN's found in CuDv.");
                        }
                        arr.num_luns = arr.linfo.num;
                        dt(10,1,"arr.linfo", arr.linfo.num);
                        erp_rc = DM_STATUS_GOOD;
                        break;
                case 85: /* Add all pdisks to the selection menu list. */
                        arr.spare_counter = 0;
                        erp_rc = raid_array_status(SPARE_STATUS | ALL_PDISKS);
                        if (arr.num_spares == 0) {
                                display_screen("3066"); /* No pdisks. */
                                clean_up(DM_STATUS_GOOD);
                        }
                        erp_rc = DM_STATUS_GOOD;
                        break;
                case 86: /* Add all spare/hot spare pdisks to the list. */
                        arr.spare_counter = 0;
                        erp_rc = raid_array_status(SPARE_STATUS | SPARE_HOT_SPARE);
                        if (arr.num_spares == 0) {
                                display_screen("3066"); /* No pdisks. */
                                clean_up(DM_STATUS_GOOD);
                        }
                        erp_rc = DM_STATUS_GOOD;
                        break;
                case 97: /* Open failed. */
                        dt(20,1,"ERROR: Open failed, task",sa.task);
                        clean_up(DM_STATUS_BAD);
                        break;
                case 98: /* Config failed. */
                        dt(20,1,"ERROR: Config failed, task",sa.task);
                        clean_up(DM_STATUS_BAD);
                        break;
                case 100: /* Exit DA with bad status. */
                        clean_up(DM_STATUS_BAD);
                        break;
                case 103:
                        if (fdes != -1) {
                                close(fdes);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 104:
                        erp_rc = retry_mgr(CLEAR_RETRY, 0);
                        seq_mgr(SEQ_DELETE);
                        erp_rc = ERP_GOOD;
                        break;
                case 3076: /* Stop format (N/y)? */
                        rc = display_screen("3076");
                        if (rc == 2) {
                                clean_up(DM_STATUS_GOOD);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                default:  /* Check for special reaction codes. */
                        if (REACTION_IS(SEQUENCE_REACTION,r_ptr)) {
                                /* Perform a Test Sequence. */
                                seq_mgr(SEQ_CREATE);
                                sa.seq->task_str = get_data(SEQUENCE_DATA,
                                      atoi(&r_ptr[strlen(SEQUENCE_REACTION)]));
                                if (sa.seq->task_str == (char *)NULL) {
                                        dt(10,1,"ERROR: TASK SEQ",erp_rc);
                                        return(DM_ERROR_ODM);
                                }
                                erp_rc = ERP_GOOD;
                                break;
                        } else if (REACTION_IS(ASL_SCREEN_REACTION,r_ptr)) {
                                rc = display_screen(r_ptr);
                                clean_up(DM_STATUS_GOOD);
                        } else if (REACTION_IS(RETRY_REACTION, r_ptr)) {
                                /* set max retry count to last two digits. */
                                erp_rc = retry_mgr(RETRY_RETRY,
                                   atoi(&r_ptr[strlen(RETRY_REACTION)]));
                        } else {
                                erp_rc = ERP_FAIL;
                        }
                        break; /* default */
                } /* end switch */

                /* Check for multiple reaction codes. */
                r_ptr = str_parse(r_code_str, ",", count++);

        } while ((r_ptr != (char *)NULL) && erp_continue &&
                 ((erp_rc == ERP_GOOD) || (erp_rc == ERP_RETRY)));

        return(erp_rc);

} /* end perform_reactions() */

/*
 *
 * NAME: init_tucb()
 *
 * FUNCTION: Initialize the tucb struct before calling exectu().
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_GOOD or DM_ERROR_XXXX on error.
 *
 */

int init_tucb(char *task) {
        int i, rc, ic = 0;              /* ic = item counter */
        int task_num = 0;
        int blocks;
        char *cmdblk = (char *)NULL;
        char *erp = (char *)NULL;
        char *paramlist = (char *)NULL;
        char *data_ptr = (char *)NULL;
        char *temp_ptr = (char *)NULL;

        /* dt(25,"In init_tucb()"); */
        task_num = atoi(task);

        /* Get the SCATU TUCB data. */
        data_ptr = get_data(SCATU_TUCB_DATA,
                            atoi(&task[strlen(SCARRY_TASK)]));
        if (data_ptr == (char *)NULL) {
                return(DM_ERROR_ODM);
        }

        /* The first TUCB item is the SCATU number */
        /* from the scsi_atu.h header file.        */

        tucb.header.tu = strtol(str_parse(data_ptr, ",", ++ic) , ep, 16);

        /* Zero out the TUCB struct. */
        rc = scsitu_init(&tucb);
        if (rc != SCATU_GOOD) {
                dt(25,"ERROR: INIT TUCB");
                return(DM_ERROR_OTHER);
        }

        /* Fill in the header portion of tucb struct. */
        tucb.header.mfg  = 0;
        tucb.header.loop = 1;
        tucb.header.r1   = 0;
        tucb.header.r2   = 0;

        /* Fill in the scsitu portion of the TUCB struct. */
        tucb.scsitu.cmd_timeout = atoi(str_parse(data_ptr,",",++ic));
        tucb.scsitu.command_length = atoi(str_parse(data_ptr,",",++ic));
        tucb.scsitu.data_length = atoi(str_parse(data_ptr,",",++ic));
        tucb.scsitu.flags = tucb_flags[atoi(str_parse(data_ptr,",",++ic))];
        tucb.scsitu.ioctl_pass_param =
                    tucb_passthru[atoi(str_parse(data_ptr,",",++ic))];
        tucb.header.r1 = atoi(str_parse(data_ptr,",",++ic));

        /* Get the CDB, PL and ERP data. */
        temp_ptr = str_parse(data_ptr,",", ++ic);
        cmdblk = str_parse(temp_ptr,":", 1);
        paramlist = str_parse(temp_ptr,":", 2);
        erp = str_parse(temp_ptr,":", 3);

        /* Get the SCATU Command Block data from ODM. */
        sprintf(attr, "SC_CDB_%s", cmdblk);
        data_ptr = get_data(SC_CDB_DATA, atoi(cmdblk));
        if (data_ptr == (char *)NULL) {
                dt(25,"ERROR: TUCB CDB");
                return(DM_ERROR_ODM);
        }

        /* Fill in the SCSI Command Block */
        for (i = 0; i < tucb.scsitu.command_length; i++) {
                temp_ptr = str_parse(data_ptr, " ", i+1);
                tucb.scsitu.scsi_cmd_blk[i] = strtol(temp_ptr, ep, 16);
        }

        /* Add the LUN value to the CDB. */
	if (arr.spt_flag) {
		tucb.scsitu.scsi_cmd_blk[1] |= 
			(unsigned)pdisk_lun_id[arr.pdisk_ch][arr.pdisk_id] << 5;
	} else {
        	tucb.scsitu.scsi_cmd_blk[1] |= dev.lun << 5;
	}

        memset(tu_buffer, 0x00, TU_BUFFER_SIZE);

        /* Check for Parameter List Data (if any). */
        if (tucb.scsitu.data_length) {
                if (strcmp(paramlist, "n/a")) {
                        /* Call cat_data() to get the SCATU Parameter List */
                        data_ptr = cat_data(SC_PL_DATA, " ", ";", paramlist);
                        if (data_ptr == (char *)NULL) {
                                dt(20,1,"ERROR: PL", paramlist);
                                return(DM_ERROR_ODM);
                        }
                        for (i = 0; i < tucb.scsitu.data_length; i++) {
                                tu_buffer[i] = strtol(str_parse(data_ptr,
                                                      " ", i+1), ep, 16);
                        }
                }
        }

        switch (task_num) {
        case 5031: /* MODE SELECT (PDisk Diag mode data). */
                /* rc = get_mode_data(SET_ODM_MD, "dac7135", "scsi"); */
                if (rc < 0) {
                        return(DM_ERROR_UNEXPECTED);
                }
                tucb.scsitu.scsi_cmd_blk[4] = (uchar)rc;
                tucb.scsitu.data_length = rc;
                memcpy(tu_buffer, diag_mode_data, rc);
                break;
        case 5013: /* MODE SELECT (LUN Diag mode data). */
                /* rc = get_mode_data(SET_ODM_MD, "array", "scsi"); */
                if (rc < 0) {
                        return(DM_ERROR_UNEXPECTED);
                }
                tucb.scsitu.scsi_cmd_blk[4] = (uchar) rc;
                tucb.scsitu.data_length = rc;
                memcpy(tu_buffer, diag_mode_data, rc);
                break;
        case 5014: /* WRITE BUFFER */
                dt(10,1,"read size", ucode.read_size);
                dt(10,1,"block count", ucode.block_counter);
                memcpy(tu_buffer, ucode.fbuff, ucode.read_size);
                if (ucode.block_counter == ucode.num_blocks) {
                        tucb.scsitu.scsi_cmd_blk[2] = 0;
                } else {
                        tucb.scsitu.scsi_cmd_blk[2] = ucode.block_counter;
                }
                tucb.scsitu.scsi_cmd_blk[6] = (uchar)(ucode.read_size >> 16);
                tucb.scsitu.scsi_cmd_blk[7] = (uchar)(ucode.read_size >> 8);
                tucb.scsitu.scsi_cmd_blk[8] = (uchar)ucode.read_size;
                tucb.scsitu.data_length = ucode.read_size;
                break;
        case 5003: /* MODE SENSE (Current LUN). */
        case 5004: /* MODE SENSE (Changable LUN). */
                tucb.scsitu.scsi_cmd_blk[7] =
                                        (uchar)(tucb.scsitu.data_length >> 8);
                tucb.scsitu.scsi_cmd_blk[8] = (uchar)tucb.scsitu.data_length;
                break;
        case 5019: /* MODE SENSE (Current PDisk). */
        case 5022: /* MODE SENSE (Changable PDisk). */
                tucb.scsitu.scsi_cmd_blk[4] = (uchar)tucb.scsitu.data_length;
                break;
        case 5037: /* FORMAT (PDisk) */
                tu_buffer[0] = 0;
                tu_buffer[1] = 0xB2;   /* Immediate bit , etc. */
                tu_buffer[2] = 0;
                tu_buffer[3] = 0;
                break;
        case 5039: /* EEPROM WRITE BUFFER */
                memcpy(tu_buffer, eeprom_buff, 64);
                break;
        default:
                break;
        }
        tucb.scsitu.data_buffer = tu_buffer;

        /* Get the ERP's for this SCATU. */
        sa.erp_str = cat_data(ERP_DATA, " ", ";", erp);
        if (sa.erp_str == (char *)NULL) {
                dt(25,"ERROR: TUCB ERP");
                return(DM_ERROR_ODM);
        }

        return(DM_STATUS_GOOD);

} /* end init_tucb() */

/*
 *
 * NAME: rs_ioctl()
 *
 * FUNCTION: Call the DKIOCMD_RS ioctl() to send SCSI commands
 *           to a LUN or dac.
 *
 * NOTES:
 *
 * RETURNS: SCATU_XXXXX (Same as a call to an SCATU).
 *
 */

int rs_ioctl(char *task) {
        int rc;
        struct sc_iocmd_rs iocmd_rs;
        char *req_sense_ptr;
        int errno_rs;

        /* dt(25,"In rs_ioctl()."); */

        /* Initialize the iocmd struct. */
        memset(&iocmd_rs, 0x00, sizeof(struct sc_iocmd_rs));

        /* Fill in the tucb info. */
        iocmd_rs.data_length = tucb.scsitu.data_length;
        iocmd_rs.flags = tucb.scsitu.flags;
        iocmd_rs.buffer = tucb.scsitu.data_buffer;
        iocmd_rs.timeout_value = tucb.scsitu.cmd_timeout;
        iocmd_rs.command_length = tucb.scsitu.command_length;
        memcpy(iocmd_rs.scsi_cdb,
               tucb.scsitu.scsi_cmd_blk,
               tucb.scsitu.command_length);
        iocmd_rs.req_sense_length = 255;
        iocmd_rs.request_sense_ptr = (char *)calloc(1,
                        iocmd_rs.req_sense_length);

        /* dt(30,iocmd_rs.command_length, iocmd_rs.scsi_cdb); */
        /* dt(30,iocmd_rs.data_length, iocmd_rs.buffer); */

        rc = ioctl(fdes, DKIOCMD_RS, &iocmd_rs);

        errno_rs = errno;

        if (rc == 0) {
                /* Put the Mode Sense data in the tucb struct */
                /* as if the Mode Sense tucb was called.      */
                memcpy(tucb.scsitu.data_buffer,
                       iocmd_rs.buffer,
                       iocmd_rs.data_length);
                return(SCATU_GOOD);
        }
        dt(10,1,"  DKIOCMD_RS errno", errno_rs);

        if (errno_rs == ETIMEDOUT) {
                return(SCATU_TIMEOUT);
        }
        if (iocmd_rs.status_validity & SC_ADAPTER_ERROR) {
                if (iocmd_rs.adapter_status & SC_HOST_IO_BUS_ERR)
                        return(SCATU_IO_BUS_ERROR);
                if (iocmd_rs.adapter_status & SC_SCSI_BUS_FAULT)
                        return(SCATU_COMMAND_ERROR);
                if (iocmd_rs.adapter_status & SC_CMD_TIMEOUT)
                        return(SCATU_TIMEOUT);
                if (iocmd_rs.adapter_status & SC_NO_DEVICE_RESPONSE)
                        return(SCATU_TIMEOUT);
                if (iocmd_rs.adapter_status & SC_ADAPTER_HDW_FAILURE)
                        return(SCATU_ADAPTER_FAILURE);
                if (iocmd_rs.adapter_status & SC_ADAPTER_SFW_FAILURE)
                        return(SCATU_ADAPTER_FAILURE);
                if (iocmd_rs.adapter_status & SC_FUSE_OR_TERMINAL_PWR)
                        return(SCATU_ADAPTER_FAILURE);
        }
        if (iocmd_rs.status_validity & SC_SCSI_ERROR) {
                /* keep only the informative bits */
                iocmd_rs.scsi_bus_status &= SCSI_STATUS_MASK;
                if (iocmd_rs.scsi_bus_status == SC_GOOD_STATUS)
                        return(SCATU_GOOD);
                if (iocmd_rs.scsi_bus_status & SC_CHECK_CONDITION) {
                        /* Put the request sense data in the tucb struct */
                        /* as if the request sense tucb was called.      */
                        sa.skey  = iocmd_rs.request_sense_ptr[2] & 0x0F;
                        sa.scode = (uchar)iocmd_rs.request_sense_ptr[13] |
                           ((uchar)iocmd_rs.request_sense_ptr[12] << 8);
                        memcpy(tucb.scsitu.data_buffer,
                               iocmd_rs.request_sense_ptr,
                               iocmd_rs.req_sense_length);
                        tucb.scsitu.data_length = iocmd_rs.req_sense_length;
                        sa.sdata = tucb.scsitu.data_buffer;
                        return(SCATU_CHECK_CONDITION);
                }
                if (iocmd_rs.scsi_bus_status & SC_RESERVATION_CONFLICT)
                        return(SCATU_RESERVATION_CONFLICT);
                /* catch everything else */
                if (iocmd_rs.scsi_bus_status)
                        return(SCATU_COMMAND_ERROR);
        }

        return (SCATU_COMMAND_ERROR);

} /* end rs_ioctl() */

/*
 *
 * NAME: ptrs_ioctl()
 *
 * FUNCTION: Call the DKIOCMD_PTRS2 ioctl() to send SCSI commands
 *           to physical disks.
 *
 * NOTES:
 *
 * RETURNS: SCATU_XXXXX (Same as a call to an SCATU).
 *
 */

int ptrs_ioctl(char *task) {
        int rc;
        struct sc_iocmd_ptrs iocmd_ptrs;
        char *req_sense_ptr;
        int errno_ptrs;

        /* dt(25,"In ptrs_ioctl()."); */

        /* Initialize the iocmd struct. */
        memset(&iocmd_ptrs, 0x00, sizeof(struct sc_iocmd_ptrs));

        iocmd_ptrs.q_tag_msg = SC_SIMPLE_Q;

        /* Fill in the tucb info. */
        iocmd_ptrs.data_length = tucb.scsitu.data_length;
        iocmd_ptrs.flags = tucb.scsitu.flags;
        iocmd_ptrs.buffer = tucb.scsitu.data_buffer;
        iocmd_ptrs.timeout_value = tucb.scsitu.cmd_timeout;
        iocmd_ptrs.command_length = tucb.scsitu.command_length;
        memcpy(iocmd_ptrs.scsi_cdb,
               tucb.scsitu.scsi_cmd_blk,
               tucb.scsitu.command_length);
        iocmd_ptrs.req_sense_length = 255;
        iocmd_ptrs.request_sense_ptr = (char *)calloc(1,
                        iocmd_ptrs.req_sense_length);

        /* Fill in the SET_PASSTHROUGH info. */
        iocmd_ptrs.passthru_direction = tucb.header.r1;
        iocmd_ptrs.dest_channel = arr.pdisk_ch;
        iocmd_ptrs.dest_scsi_id = arr.pdisk_id;

	dt(16,1,"Disk CH", arr.pdisk_ch);
	dt(16,1,"Disk ID", arr.pdisk_id);

        rc = ioctl(fdes, DKIOCMD_PTRS2, &iocmd_ptrs);

        errno_ptrs = errno;

        if (rc == 0) {
                /* Put the Mode Sense data in the tucb struct */
                /* as if the Mode Sense tucb was called.      */
                memcpy(tucb.scsitu.data_buffer,
                       iocmd_ptrs.buffer,
                       iocmd_ptrs.data_length);
                return(SCATU_GOOD);
        }
        dt(10,1,"  DKIOCMD_PTRS2 errno", errno_ptrs);
        dt(16,1,"  DKIOCMD_PTRS2 status validity", iocmd_ptrs.status_validity);
        dt(16,1,"  DKIOCMD_PTRS2 passthru status", iocmd_ptrs.passthru_status);

        if (errno_ptrs == ETIMEDOUT) {
                return(SCATU_TIMEOUT);
        }
        if (iocmd_ptrs.status_validity & SC_ADAPTER_ERROR) {
                if (iocmd_ptrs.adapter_status & SC_HOST_IO_BUS_ERR)
                        return(SCATU_IO_BUS_ERROR);
                if (iocmd_ptrs.adapter_status & SC_SCSI_BUS_FAULT)
                        return(SCATU_COMMAND_ERROR);
                if (iocmd_ptrs.adapter_status & SC_CMD_TIMEOUT)
                        return(SCATU_TIMEOUT);
                if (iocmd_ptrs.adapter_status & SC_NO_DEVICE_RESPONSE)
                        return(SCATU_TIMEOUT);
                if (iocmd_ptrs.adapter_status & SC_ADAPTER_HDW_FAILURE)
                        return(SCATU_ADAPTER_FAILURE);
                if (iocmd_ptrs.adapter_status & SC_ADAPTER_SFW_FAILURE)
                        return(SCATU_ADAPTER_FAILURE);
                if (iocmd_ptrs.adapter_status & SC_FUSE_OR_TERMINAL_PWR)
                        return(SCATU_ADAPTER_FAILURE);
        }
        if (iocmd_ptrs.status_validity & SC_SCSI_ERROR) {
                /* keep only the informative bits */
                iocmd_ptrs.scsi_bus_status &= SCSI_STATUS_MASK;
                if (iocmd_ptrs.scsi_bus_status == SC_GOOD_STATUS)
                        return(SCATU_GOOD);
                if (iocmd_ptrs.scsi_bus_status & SC_CHECK_CONDITION) {
                        /* Put the request sense data in the tucb struct */
                        /* as if the request sense tucb was called.      */
                        sa.skey  = iocmd_ptrs.request_sense_ptr[2] & 0x0F;
                        sa.scode = (uchar)iocmd_ptrs.request_sense_ptr[13] |
                           ((uchar)iocmd_ptrs.request_sense_ptr[12] << 8);
                        memcpy(tucb.scsitu.data_buffer,
                               iocmd_ptrs.request_sense_ptr,
                               iocmd_ptrs.req_sense_length);
                        tucb.scsitu.data_length = iocmd_ptrs.req_sense_length;
                        sa.sdata = tucb.scsitu.data_buffer;
                        return(SCATU_CHECK_CONDITION);
                }
                if (iocmd_ptrs.scsi_bus_status & SC_RESERVATION_CONFLICT)
                        return(SCATU_RESERVATION_CONFLICT);
                /* catch everything else */
                if (iocmd_ptrs.scsi_bus_status)
                        return(SCATU_COMMAND_ERROR);
        }

        return (SCATU_COMMAND_ERROR);

} /* end ptrs_ioctl() */

/*
 *
 * NAME: run_method
 *
 * FUNCTION: Run a configuration method.
 *
 * PARAMETERS: dev_name: Device logical name for CuDv query.
 *               method: Method type (CFG, CHG, UCFG)
 *      method_criteria: Criteria to pass to odm_run_method().
 *
 * RETURNS: Return code from odm_run_method() call, or
 *          -2 = Error trying to get device CuDv info, or
 *          -3 = Invalid method.
 *
 * NOTES:
 *
 */

int run_method(char *dev_name, uchar method, char *method_criteria) {
        int i;
        int rc = -1;
        struct CuDv *dev_cudv;
        struct listinfo dev_linfo;
        char dev_criteria[32];
        char *errbuf = (char *)NULL;
        char *outbuf = (char *)NULL;

        /* Get the CuDv info for the device name passed in. */
        sprintf(dev_criteria, "name = %s", dev_name);
        dev_cudv = get_CuDv_list(CuDv_CLASS, dev_criteria, &dev_linfo, 1, 2);
        if ((dev_cudv == (struct CuDv *)(-1)) ||
            (dev_cudv == (struct CuDv *)NULL)) {
                dt(20,1,"ERROR: run_method, CuDv name", dev_name);
                return(-2);
        }

        /* Run the method passed in. */
        for (i = 0; ((i < 3) && (rc != 0)); i++) {
                switch(method) {
                case CFG_METHOD:
                        rc = odm_run_method(MKDEV,
                                            method_criteria,
                                            &outbuf,&errbuf);
                        break;
                case CHG_METHOD:
                        rc = odm_run_method(CHDEV,
                                            method_criteria,
                                            &outbuf,&errbuf);
                        break;
                case UCFG_METHOD:
                        rc = odm_run_method(RMDEV,
                                            method_criteria,
                                            &outbuf,&errbuf);
                        break;
                default:
                        dt(10,1,"ERROR: run_method", method);
                        rc = -3;
                        break;
                }
                dt(10,1,"run_method() rc", rc);
                dt(20,1,"  device", dev_name);
                if (rc == 61) {
                        sleep(10);
                }
        	if (outbuf != (char *)NULL)
                	free(outbuf);
        	if (errbuf != (char *)NULL)
                	free(errbuf);
        }

        odm_free_list(dev_cudv, &dev_linfo);

        return(rc);

} /* end run_method */

/*
 *
 * NAME: get_cfg_state()
 *
 * FUNCTION: Get the cofig state of a device.
 *
 * NOTES:
 *
 * RETURNS: Config state or -1 on error.
 *
 */

int get_cfg_state(char *dev_name) {
        struct CuDv *dev_cudv;
        struct listinfo dev_linfo;
        char dev_criteria[32];

        /* Get the CuDv info for the device name passed in. */
        sprintf(dev_criteria, "name = %s", dev_name);
        dev_cudv = get_CuDv_list(CuDv_CLASS, dev_criteria, &dev_linfo, 1, 2);
        if ((dev_cudv == (struct CuDv *)(-1)) ||
            (dev_cudv == (struct CuDv *)NULL)) {
                dt(20,1,"ERROR: get_cfg_state, CuDv name", dev_name);
                return(-1);
        }

        odm_free_list(dev_cudv, &dev_linfo);

        return(dev_cudv->status);

} /* end get_cfg_state() */




/*
 *
 * NAME: cat_data()
 *
 * FUNCTION: Concatenate data strings into one string.
 *
 * NOTES:
 *
 * RETURNS: Pointer to data string.
 *
 */

char *cat_data(int data_type, char *pad, char *delim, char *code_str) {
        int i;
        char *ptr = (char *)NULL;
        char *temp = (char *)NULL;
        char *temp_ptr = (char *)NULL;

        for (i = 0;
             ((temp_ptr =
                     str_parse(code_str, delim, i+1)) != (char *)NULL); i++) {

                temp = get_data(data_type, atoi(temp_ptr));
                temp_ptr = (char *)calloc(1, strlen(ptr) + 1) ;
                strcpy(temp_ptr, ptr);
                if (ptr != (char *)NULL) {
                        free(ptr);
                }
                ptr = (char *)calloc(1, strlen(temp_ptr) + 1 +
                                        strlen(temp) + 1 +
                                        strlen(pad) + 1);
                if (i == 0) {
                        strcpy(ptr, temp);
                } else {
                        sprintf(ptr, "%s%s%s", temp_ptr, pad, temp);
                }
                free(temp_ptr);
        }

        /* dt(20,1,"In cat_data, ptr", ptr); */

        return(ptr);

} /* cat_data() */

/*
 *
 * NAME: req_sense()
 *
 * FUNCTION: Issue the REQUEST SENSE SCATU.
 *
 * NOTES:
 *
 * RETURNS: Return code from the SCATU.
 *
 */

int req_sense(void) {
        int rc;

        /* Set up for and issue the request sense command. */

        tucb.header.tu = 0x02;
        /* Zero out the TUCB struct. */
        rc = scsitu_init(&tucb);
        if (rc != SCATU_GOOD) {
                dt(25,"ERROR: REQ SENSE INIT TUCB");
                return(rc);
        }
        /* Fill in the header portion of tucb struct. */
        tucb.header.mfg  = 0;
        tucb.header.loop = 1;
        tucb.header.r1   = 0;
        tucb.header.r2   = 0;

        tucb.scsitu.cmd_timeout = 60;
        tucb.scsitu.command_length = 6;
        tucb.scsitu.data_length = 0xFF;
        tucb.scsitu.flags = B_READ;
        tucb.scsitu.ioctl_pass_param = DKIOCMD;

        tucb.scsitu.scsi_cmd_blk[1] |= dev.lun << 5;
        tucb.scsitu.scsi_cmd_blk[2] = 0;
        tucb.scsitu.scsi_cmd_blk[3] = 0;
        tucb.scsitu.scsi_cmd_blk[4] = 0xFF;
        tucb.scsitu.scsi_cmd_blk[5] = 0;
        tucb.scsitu.data_length = 0xFF;
        tucb.scsitu.data_buffer = tu_buffer;

        /* dt(2,&tucb); */
        rc = exectu(fdes, &tucb);
        if (rc != SCATU_GOOD) {
                /* Retry */
                dt(10,1,"WARNING: REQUEST SENSE rc",rc);
                rc = exectu(fdes, &tucb);
        }
        if (rc == SCATU_GOOD) {
                sa.skey  = tucb.scsiret.sense_key;
                sa.scode = tucb.scsiret.sense_code;
                sa.sdata = tucb.scsitu.data_buffer;
                dt(3,&tucb);
        } else {
                sa.skey  = 0;
                sa.scode = 0;
                dt(10,1,"ERROR: REQUEST SENSE rc", rc);
        }

        return(rc);

} /* end req_sense() */

/*
 * NAME: display_screen()
 *
 * FUNCTION: Display an ASL message or menu.
 *
 * NOTES:
 *
 * RETURNS: ASL return code or menu item selected.
 *
 */

int display_screen(char *screen) {
        int i, rc;
        int scr_type;
        int scr_alloc;
        long scr_num;
        long menu_num;
        ASL_SCR_INFO *menuinfo;
        ASL_SCR_TYPE *menutype;
        char *cat_nums = (char *)NULL;
        char *odm_scr_data = (char *)NULL;
        char *scr_info = (char *)NULL;
        char *temp_ptr = (char *)NULL;

        dt(20,1, "In display_screen(), screen", screen);

        scr_num = atoi(&screen[strlen(SCREEN_TASK)]);

        odm_scr_data = get_data(ASL_SCREEN_DATA, scr_num);
        if (odm_scr_data == (char *)NULL) {
                dt(25,"ERROR: ODM SCR DATA.");
                return(DM_ERROR_ODM);
        }

        /* Get the ERP for this screen. */
        temp_ptr = str_parse(odm_scr_data, ":", 3);
        sa.erp_str = cat_data(ERP_DATA, " ", ";", temp_ptr);
        if (sa.erp_str == (char *)NULL) {
                dt(20,1,"ERROR: SCR ERP");
                        return(DM_ERROR_ODM);
        }

        if (sa_flags & NO_CONSOLE) {
                /* No console mode for command line download. */
                dt(25, "No Console Mode");
                return(ASL_OK);
        }

        /* Get the screen info from scr_data. */
        scr_info = str_parse(odm_scr_data, ":", 1);

        /* Get the screen type from scr_info. */
        scr_type  = atoi(str_parse(scr_info, ",", 1));

        /* Get the allocation size of the ASL_INFO srtuct. */
        scr_alloc = atoi(str_parse(scr_info, ",", 2));

        if (scr_alloc == 0) {
                /* Dynamic message or selection menu.   */
                /* Determine scr_alloc for this screen. */
                switch(scr_num) {
                case 51: /* List all dac's in the system. */
                        scr_alloc = (2 + arr.num_cntrls);
                        break;
                case 102: /* List all LUN's in the array. */
                        scr_alloc = (2 + arr.num_luns);
                        break;
                case 71:
                case 101:
                case 155: /* List all physical disks in the array. */
                        scr_alloc = (2 + arr.num_spares);
                        break;
                case 152: /* List all valid ucode files. */
                        scr_alloc = (2 + ucode.num_files);
                        break;
                default:
                        dt(25,"ERROR: invalid screen allocation value!");
                        clean_up(DM_ERROR_UNEXPECTED);
                        break;
                }
        }

        /* Allocate for the ASL_INFO struct. */
        menuinfo = (ASL_SCR_INFO *)calloc(scr_alloc, sizeof(ASL_SCR_INFO));

        /* For each message, call alloc_msg() to allocate */
        /* for the message and do any sustitutions.       */
        msg_item = 1;
        dev.counter = 0;
        for (i = 0; i < scr_alloc; i++) {
                /* Allocate for the message and populate substitutions. */
                menuinfo[i].text = alloc_msg(odm_scr_data);
                ++msg_item;
        }

        /* Set menu_num to HEX for diag_display() call. */
        menu_num = (DAC_FFC * 0x1000) + strtol(&screen[strlen(SCREEN_TASK)], ep, 16);

        menutype = (ASL_SCR_TYPE *)calloc(1, sizeof(ASL_SCR_TYPE));
        menutype->screen_code = asl_scr_types[scr_type];
        menutype->max_index = scr_alloc - 1; /* Zero based */
        menutype->cur_index = 1;

        /* Display the message or selection menu.  */
        rc = diag_display(menu_num, catd, NULL, DIAG_IO,
                          asl_scr_types[scr_type], menutype, menuinfo);

        if (rc == DIAG_ASL_CANCEL) {
                return(DM_USER_CANCEL);
        }
        if (rc == DIAG_ASL_EXIT) {
                return(DM_USER_EXIT);
        }

        if (scr_type <= 3) {
                rc = DIAG_ITEM_SELECTED(*menutype);
                item_selected = rc;
        }

        free(menuinfo);
        free(menutype);

        return(rc);

} /* end display_screen() */

/*
 *
 * NAME: alloc_msg()
 *
 * FUNCTION: Allocate memory for an ASL message and substitutions.
 *
 * NOTES:
 *
 * RETURNS: Pointer to allocated space.
 *          Null on error.
 */

char *alloc_msg(char *scr_data) {
        int i, rc;
        int ch, id;
        int cat_num;
        int test_time;
        char *cat_ptr;
        char *buff;
        char *ptr;

        /* dt(10,1,"In alloc_msg(), msg_item",msg_item); */

        /* The message numbers start after the screen info. */
        /* Get the catalog numbers. */
        ptr = str_parse(scr_data, ":", 2);
        cat_num = atoi(str_parse(ptr, " ", msg_item));

        /* Take care of common substitutions. */
        switch(cat_num) {
        case STAND_BY:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                /* Get the test time (in minutes) for the message. */
                ptr = str_parse(scr_data, ":", 1);
                test_time = atoi(str_parse(ptr, ",", 3));
                sprintf(buff, cat_ptr, test_time);
                break;
        case CNTRL_SELECTION_ITEM:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, (cudv + dev.counter)->name,
                       (cudv + dev.counter)->location);
                if (++dev.counter < arr.num_cntrls) {
                        --msg_item;
                }
                break;
        case GET_LUN_STATUS:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + (NAMESIZE * 2) + 2);
                sprintf(buff, cat_ptr,
                        (arr.cudv + arr.lun_counter)->name,
                        (arr.cudv + arr.lun_counter)->location);
                break;
        case DETECT_SPARES:
        case DEGRADED_PDISK:
        case FORMAT_IN_PROGRESS:
	case STOP_IO_FOR_uCODE:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + NAMESIZE);
                sprintf(buff, cat_ptr,
                        pdisk_locs[arr.pdisk_ch][arr.pdisk_id]);
                break;
        case LUN_SELECTION_ITEM:
                dt(10,1,"LUN dev.counter",dev.counter);
                buff = raid_get_menu_item(dev.counter, GET_MENU_ITEM);
                if (++dev.counter < arr.num_luns) {
                        --msg_item;
                }
                break;
        case PDISK_SELECTION_ITEM:
                dt(10,1,"PDisk dev.counter",dev.counter);
                buff = raid_get_menu_item(dev.counter, GET_MENU_ITEM);
                if (++dev.counter < arr.num_spares) {
                        --msg_item;
                }
                break;
        case PDISK_FORMAT_P_COMPLETE:
        case PDISK_CERTIFY_P_COMPLETE:
                /* Get capacity and location. */
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, dev.capacity, dev.location,
                        dev.percent_complete);
                break;
        case LUN_CERTIFY_P_COMPLETE:
                /* Get capacity, name and location. */
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, dev.capacity, arr.lun_name,
                        dev.location, dev.percent_complete);
                break;
        case CERTIFY_PFA:
        case CERTIFY_BAD:
                /* Get capacity and location. */
                ch = arr.pdisk_ch;
                id = arr.pdisk_id;
                if ((ch < 1) || (ch > MAX_SUPPORTED_CHs) ||
                    (id < 0) || (id > (MAX_SUPPORTED_IDs - 2))) {
                        ch = 0;
                        id = 0;
                }
                if (sa_selected == SA_CERTIFY_LUN) {
                        strcpy(dev.capacity,
                            (char *)get_pdisk_capacity(arr.dar_name, ch, id));
                }
                strcpy(dev.location, pdisk_locs[ch][id]);
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, dev.capacity, dev.location);
                break;
        case CNTRL_uCODE_COMPLETE:
        case PDISK_uCODE_COMPLETE:
        case EEPROM_UPDATE_COMPLETE:
        case EEPROM_FILE_ERROR:
        case AT_CURRENT_LEVEL:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, ucode.download_fname);
                break;
        case NO_uCODE_FILES:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, ucode.current_fname);
                break;
        case EEPROM_FILE_REQ:
        case CNTRL_STATE_NOT_HIR:
        case PAIRED_DAC_HIR:
        case CNTRL_STATE_OK:
	case REMOVE_OLD_DAC:
	case INSERT_NEW_DAC:
	case REPLACE_CONTROLLER_DONE:
	case CNTRL_NOT_RESPOND:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                sprintf(buff, cat_ptr, dname, dnameloc);
                break;
	case uCODE_IN_PROGRESS:
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 128);
		if (sa_selected == SA_uCODE_PDISK) {
                	sprintf(buff, cat_ptr, ucode.download_fname, dev.location);
		} else {
                	sprintf(buff, cat_ptr, ucode.download_fname, dnameloc);
		}
                break;
        default: /* No substitution required. */
                cat_ptr = (char *)diag_cat_gets(catd, U7135_MSG_SET, cat_num);
                buff = (char *)calloc(1, strlen(cat_ptr) + 64);
                strcpy(buff, cat_ptr);
                break;
        }

        return((char *)buff);

} /* end alloc_msg() */

/*
 * NAME:  void check_asl()
 *
 * FUNCTION: If Cancel or Exit, clean up with No Trouble Found.
 *
 * NOTES: This will also check and clear the key board buffer.
 *
 * RETURNS: void
 *
 */

void check_asl() {
        int rc;

        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
        switch(rc) {
        case DIAG_ASL_CANCEL:
                rc = DM_USER_CANCEL;
                break;
        case DIAG_ASL_EXIT:
                rc = DM_USER_EXIT;
                break;
        default:
                rc = 0;
                break;
        }

        if (rc) {
                clean_up(rc);
        }

        return;

} /* end check_asl() */

/*
 *
 * NAME: str_parse()
 *
 * FUNCTION: Parse a string of items, separated by a delimeters, for a
 *           a specific item.
 *
 * NOTES:
 *
 * RETURNS: Pointer to the item.
 *          Null otherwise.
 */

char *str_parse( char *str_to_parse, char *delimeter, int item_number) {
        int count = 1;
        char *ptr = (char *)NULL;
        char buff[1024];
        char *ret_buff = (char *)NULL;

        strcpy(buff, str_to_parse);

        ptr = strtok(buff, delimeter);
        while (ptr != NULL && count != item_number) {
                ptr = strtok( NULL, delimeter);
                if (ptr != (char *)NULL) {
                        --item_number;
                }
        }

        if (count != item_number) {
                ptr = (char *)NULL;
        } else {
                ret_buff = (char *)malloc(strlen(ptr) + 1);
                strcpy(ret_buff, ptr);
        }

        return(ret_buff);

} /* end str_parse() */

/*
 *
 * NAME: clean_up()
 *
 * FUNCTION: Clean up before exiting back to the Diag Controller. Close
 *           open files, unconfig devices, etc.
 *
 * NOTES:
 *
 * RETURNS: Exit back to diag controller.
 *
 */

int clean_up(int sa_status) {
        int rc;
        static int cu_flag;

        ++cu_flag;

        /*
         * We are done testing. Perform the following:
         *     - If DA reserved a device, release it.
         *     - Logical close of device.
         *     - Return device to initial cfg state.
         *     - Close catalog file.
         *     - Quit ASL.
         *     - Exit back to the Diag Controller.
         */

        dt(25,"In clean_up()");
        dt(16,1,"---->    Status",sa_status);
        dt(10,1,"---->      step",sa.seq->step);

        switch(sa_status) {
        case DM_ERROR_OPEN:
                display_screen("3065");
                break;
        case DM_USER_EXIT:
        case DM_USER_CANCEL:
        case DM_STATUS_GOOD:
                break;
        case DM_ERROR_UNEXPECTED:
        case DM_ERROR_OTHER:
        case DM_ERROR_ODM:
        case DM_STATUS_BAD:
        default:
                if (cu_flag > 2) {
                        break;
                }
                display_screen("3069");
                break;
        }

        /* If a device has been resevered, release it before exiting. */
        if (((SA_RESERVED_LUN & sa_flags) || (SA_RESERVED_PDISK & sa_flags)) &&
            (cu_flag < 2)) {
                /* Release the unit before exiting. */
                init_tucb(TASK_SCATU_RELEASE_UNIT);
                rc = exectu(fdes, &tucb);
                if (rc != SCATU_GOOD) { /* Retry. */
                        rc = exectu(fdes, &tucb);
                        dt(10,1,"release exectu() rc",rc);
                }
        }

        if (fdes > -1) {
                dt(25,"Closing device in clean_up().");
                close(fdes);
        }

	if (sa_selected == SA_REPLACE_CNTRL) {
		/* Run the cfgdac and cfgdar methods to update ODM again. */
        	sprintf(attr, " -l %s", dname);
        	rc = run_method(dname, CFG_METHOD, attr);
        	dt(10,1,"dac ODM update cfg rc", rc);
        	sprintf(attr, " -l %s", arr.dar_name);
        	rc = run_method(arr.dar_name, CFG_METHOD, attr);
        	dt(10,1,"dar ODM update cfg rc", rc);
	}

        if (arr.lun_cfg_state == DEFINED) {
                /* Return device to the DEFINED state. */
                sprintf(attr, " -l %s", arr.lun_name);
                dt(20,1,"LUN name", arr.lun_name);
                rc = run_method(arr.lun_name, UCFG_METHOD, attr);
                dt(10,1,"LUN uncfg rc", rc);
        }
        if (arr.dar_cfg_state == DEFINED) {
                /* Return device to the DEFINED state. */
                sprintf(attr, " -l %s", arr.dar_name);
                rc = run_method(arr.dar_name, UCFG_METHOD, attr);
                dt(10,1,"dar uncfg rc", rc);
        }
        if (dev_cfg_state != -1) {
                /* Return device to initial config state. */
                rc = initial_state(dev_cfg_state, dname);
                dt(10,1,"dev initial_state rc", rc);
        }

        /* Remove any sequence structs remaining. */
        while (sa.seq != (struct sa_seq *)NULL)
                seq_mgr(SEQ_DELETE);

        if (SA_INIT_ODM & sa_flags)
                term_dgodm();
        if (SA_INIT_CAT & sa_flags)
                catclose(catd);
        if (SA_INIT_ASL & sa_flags)
                diag_asl_quit();

        dt(999);

	if ((sa_flags & NO_CONSOLE) && (sa_status != DM_STATUS_GOOD)) {
		exit(-1);
	}

        exit(0); /* Exit back to the Diag Controller */

} /* end clean_up() */

/*
 * NAME: int_handler()
 *
 * FUNCTION: Interrupt handler.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void int_handler(int sig)
{
        clean_up(DM_STATUS_GOOD);
        return;
} /* end int_handler() */

/*
 * NAME: dt()
 *
 * FUNCTION: Diagnostic Trace (dt) for low level software trace.
 *
 * NOTES:
 *      - The calls to dt() are for trace. If the trace file exists,
 *        trace information will be written. If the trace file
 *        does not exist, dt() will simply return. To turn on the trace,
 *        touch a file  called /tmp/.DIAG_DMEDIA_TRACE and that will
 *        cause the trace information to be saved in the trace file.
 *        To append to the trace file each time the DA/SA is run,
 *        export DIAG_DMEDIA_TRACE=APPEND before running diagnostics.
 *
 *      - The trace file name will be /tmp/.dt.u7135
 *
 *      - The function has variable arguments and will call va_start()
 *        and va_end() even if the trace file does not exist.
 *
 * RETURNS: void
 *
 */

void dt(int dt_num, ...)
{
        int           i, i1, i2, rc;
        char          *s1, *s2;
        char          fname[256];
        SCSI_TUTYPE   *dt_tucb;
        FILE          *dtfptr;
        struct stat   file_status;
        va_list       ag;

        va_start(ag, dt_num);

        if ((rc = stat("/tmp/.DIAG_DMEDIA_TRACE",&file_status)) == -1) {
                va_end(ag);
                return; /* Debug file not present. */
        }

        strcpy(fname, "/tmp/.dt.u7135");

        if (!strcmp((char *)getenv("DIAG_DMEDIA_TRACE"),"APPEND")) {
                dtfptr = fopen(fname,"a+");
        } else {
                if (dt_num == 1) {
                        dtfptr = fopen(fname,"w+");
                } else {
                        dtfptr = fopen(fname,"a+");
                }
        }

        switch(dt_num) {
        case 1: /* init trace file */
                fprintf(dtfptr,"============= start u7135 ============\n");
                break;
        case 2: /* print dt_tucb info before call to exectu(). */
                dt_tucb = va_arg(ag, SCSI_TUTYPE *);
                fprintf(dtfptr,"> SCSI CDB (Data Out)\n\t");
                p_data(dtfptr, dt_tucb->scsitu.scsi_cmd_blk,
                        dt_tucb->scsitu.command_length);
                if ((dt_tucb->scsitu.flags == B_WRITE) &&
                    (dt_tucb->scsitu.data_length)) {
                        fprintf(dtfptr,"> Param List (Data Out)\n\t");
                        p_data(dtfptr, dt_tucb->scsitu.data_buffer, 96);
                }
                break;
        case 3: /* print Sense Data */
                dt_tucb = va_arg(ag, SCSI_TUTYPE *);
                fprintf(dtfptr,
                        "> Sense Key %02.2X, ASC/ASCQ = %04.4X\n",
                        (dt_tucb->scsiret.sense_key & 0x0F),
                        dt_tucb->scsiret.sense_code);
                fprintf(dtfptr,"> Sense Data\n\t");
                p_data(dtfptr, dt_tucb->scsitu.data_buffer, 128);
                break;
        case 10: /* Print multiple integers (int_name = int_value). */
        case 16:
                i1 = va_arg(ag, int); /* Number of int name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        i2 = va_arg(ag, int);
                        if (dt_num == 16) /* Hex */
                                fprintf(dtfptr, "> %s = %X\n", s1, i2);
                        else /* Decimal */
                                fprintf(dtfptr, "> %s = %d\n", s1, i2);
                }
                break;
        case 20: /* print  multiple strings (ptr_name = ptr). */
                i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        s2 = va_arg(ag, char *);
                        fprintf(dtfptr, "> %s = %s\n", s1, s2);
                }
                break;
        case 25:
                /* Print a simple string. */
                s1 = va_arg(ag, char *);
                fprintf(dtfptr, "%s\n",s1);
                break;
        case 30: /* Display buffer data. */
                i1 = va_arg(ag, int);
                s2 = va_arg(ag, char *);
                fprintf(dtfptr, "> Data Buffer\n\t");
                p_data(dtfptr, s2, i1);
                break;
        case 999:
                fprintf(dtfptr,"======== end ========\n");
                break;
        default:
                break;
        }
        /* fflush(dtfptr); */
        if (dtfptr != NULL)
                fclose(dtfptr);
        va_end(ag);

        return;

} /* end dt */

/*
 * NAME: p_data()
 *
 * FUNCTION: Print hex bytes from a data buffer.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void p_data(FILE *dtfptr, char *ptr, int len) {
        int count = 0;

        if (len > 255) {
                len = 255;
        }
        /* Print data buffer, 16 bytes per line. */
        while ((len--) > 0) {
                if (count == 8) {
                        fprintf(dtfptr,"    ");
                } else if (count == 16) {
                        fprintf(dtfptr,"\n\t");
                        count = 0;
                }
                fprintf(dtfptr,"%02.2X ", *(ptr++));
                ++count;
        }
        fprintf(dtfptr,"\n");

        return;
} /* end p_data */

/* end d7135m.c */

