static char sccsid[] = "@(#)39  1.6  src/bos/diag/da/d7135/d7135m.c, da7135, bos41J, 9519A_all 5/4/95 17:09:04";
/*
 *   COMPONENT_NAME: DA7135
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "d7135m.h"

long da_flags = 0;              /* Flags for init status, etc.              */
int dev_lun = 0;                /*                                          */
int fdes = -1;                  /* Device file descriptor                   */
int dev_cfg_state = -1;         /* Device config state flag.                */
int par_cfg_state = -1;         /* Parent config state flag.                */

/*
 *
 * NAME: main()
 *
 * FUNCTION: Call initialization routines for a Diagnostic Application (DA).
 *           Get the initial sequence of tasks for a 7135 DA and call the
 *           do_seq() function to execute the sequence of tasks.
 *
 * NOTES:
 *
 * RETURNS:  Return to the Diag Controller through clean_up() with the rc
 *           from the do_seq() call.
 *
 */

main(int argc, char *argv, char **envp) {
        int i,j,rc;
        struct sigaction act;

        /* set initial return codes */
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_USER(DA_USER_NOKEY);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);

        setlocale(LC_ALL,"");

        /* Initialize the interrupt handler. */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /* Initialize ODM for Diag calls. */
        rc = init_dgodm();
        if (rc == -1) {
                dt(25,"ERROR: INIT_ODM");
                clean_up(DM_ERROR_OTHER);
        }
        da_flags |= DA_INIT_ODM;

        /* Get our test mode input parameters. */
        if (getdainput(&tm_input) == -1) {
                dt(25,"ERROR: TM_INPUT");
                clean_up(DM_ERROR_OTHER);
        }

        dt(1);

        ipl_mode(&iplmode);

        if (CONSOLE) {
                /* Initialize ASL. */
                if (INLM || SYSTEM) {
                        /* Allow type ahead to catch Cancel or Exit. */
                        rc = diag_asl_init("DEFAULT");
                } else {
                        /* Dont allow any type ahead. */
                        rc = diag_asl_init("NO_TYPE_AHEAD");
                }
                if (rc == -1) {
                        dt(25,"ERROR: ASL_INIT");
                        clean_up(DM_ERROR_OTHER);
                }
                da_flags |= DA_INIT_ASL;
        }
        /* Open catalog file for messages. */
        catd = diag_catopen("d7135.cat",0);
        da_flags |= DA_INIT_CAT;

        seq_mgr(SEQ_CREATE); /* Create the main sequence struct. */
        main_seq = da.seq;   /* Save a pointer to it (head ptr). */
        retry = (struct retry *)malloc(sizeof(struct retry));
        retry->step = -1;
        retry->next = (struct retry *)NULL;

        rc = do_seq(Main_DA_Seq);

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

        da.seq->task_str = get_data(SEQUENCE_DATA, seq);

        if (da.seq->task_str == (char *)NULL) {
                dt(10,1,"ERROR: DA SEQ",seq);
                return(DM_ERROR_ODM);
        }
        dt(20,1,"Sequence",da.seq->task_str);
        da.seq->step = 1;
        da.task = str_parse(da.seq->task_str, " ", da.seq->step);

        do {
                check_asl();
                dt(20,1,"Task",da.task);
                da.task_rc = do_task();
                dt(10,1,"do_task() rc",da.task_rc);

                /* The da.task_rc will decide which ERP to perform. */
                rc = do_erp();
                dt(16,1,"do_erp() rc",rc);

                /* The rc from do_erp() will be added to the step   */
                /* counter to determine the next task to perform.   */
                da.seq->step += rc;
                da.task = str_parse(da.seq->task_str, " ", da.seq->step);

                if (!strncmp(da.task, TASK_END_OF_SEQ, 4)) {
                        /* No more tasks in this sequence. */
                        seq_mgr(SEQ_DELETE);
                        if (da.seq == (struct da_seq *)NULL) {
                                done = TRUE;
                        } else {
                                /* A task sequence finished, increment the */
                                /* previous step counter for next task.    */
                                ++da.seq->step;
                                da.task = str_parse(da.seq->task_str, " ",
                                                    da.seq->step);
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
        struct da_seq *seq, *back;

        if (action == SEQ_CREATE) {
                seq = (struct da_seq *)calloc(1,sizeof(struct da_seq));
                if (da.seq == (struct da_seq *)NULL) {
                        seq->next = (struct da_seq *)NULL;
                } else {
                        seq->next = da.seq->next;
                        da.seq->next = seq;
                }
                da.seq = seq;
                da.seq->step = 0;
                dt(25,"Create Sequence");
        } else if (action == SEQ_DELETE) {
                /* Find the previous sequence. */
                back = seq = main_seq;
                while((seq != da.seq) && (seq != (struct da_seq *)NULL)) {
                        back = seq;
                        seq = seq->next;
                }
                if (back == seq) {
                        da.seq = main_seq = (struct da_seq *)NULL;
                        dt(25,"Delete Main Sequence");
                } else {
                        back->next = seq->next;
                        da.seq = back;
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
 * FUNCTION: Call the routine for the task type.
 *
 * NOTES:
 *
 * RETURNS: Return code from the task called.
 *
 */

int do_task(void) {
        int get_misc_erp = 0;
        int my_errno;
        int rc = DM_STATUS_GOOD;
        int rs_rc;
        int task_num;
        char *ptr;

        task_num = atoi(da.task);

        /* Determine the task type and call the appropriate routine. */
        if (TASK_IS(CONFIG_TASK,da.task)) {
                rc = config(da.task);
        } else if (TASK_IS(OPEN_TASK,da.task)) {
                rc = dev_open(da.task);
        } else if (TASK_IS(SCREEN_TASK,da.task)) {
                rc = display_screen(da.task);
        } else if (TASK_IS(SCARRY_TASK,da.task)) {
                /* Execute the DKIOCMD_RS ioctl(). */
                init_tucb(da.task);
                dt(2,&tucb);
                if (arr.spt_flag) {
                        rc = ptrs_ioctl(da.task);
                } else {
                        rc = rs_ioctl(da.task);
                }
                dt(30,tucb.scsitu.data_length,
                      tucb.scsitu.data_buffer);
        } else if (TASK_IS(GET_ROUTER_TASK,da.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(da.task));
                da.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
                rc = ioctl(fdes, RT_GET_ROUTER_STATE, &router.ioctl);
                my_errno = errno;
                if (rc != 0) {
                        dt(10,1,"GET ROUTER errno", my_errno);
                        rc = my_errno;
                }
        } else if (TASK_IS(SET_ROUTER_TASK,da.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(da.task));
                da.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
                rc = ioctl(fdes, RT_SET_ROUTER_STATE, &router.ioctl);
                my_errno = errno;
                if (rc != 0) {
                        dt(10,1,"SET ROUTER errno", my_errno);
                        rc = my_errno;
                }
        } else if (TASK_IS(SCATU_TASK,da.task)) {
                /* Execute an SCATU (LUN Certify). */
                init_tucb(da.task);
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
        } else if (TASK_IS(MENUGOAL_TASK,da.task)) {
                rc = display_menugoal(da.task);
        } else if (TASK_IS(ELA_TASK,da.task)) {
                rc = raid_ela(da.task);
        } else if (TASK_IS(MISC_TASK,da.task)) {
                /* Get the ERP data for this task. */
                ptr = get_data(MISC_TASK_DATA, atoi(da.task));
                da.erp_str = cat_data(ERP_DATA, " ", ";", ptr);
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
        char *c_code, *r_code;

        /* Search the ERP data for a true condition. */
        do {
                /* Get the ERP condition code for this task. */
                c_code = get_erp_code(++erp_count, CONDITION);

                /* Test if the condition code is true. */
                found = test_condition(c_code);

        } while ((c_code != (char *)NULL) && !found);

        if ((c_code == (char *)NULL) || (!found)) {
                dt(25,"ERROR: COND CODE");
                return(ERP_FAIL);
        }

        dt(20,1,"In do_erp(), c_code",c_code);

        /* Get the ERP reaction code for this task. */
        r_code = get_erp_code(erp_count, REACTION);

        dt(20,1,"In do_erp(), r_code",r_code);

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
                if (CHECK_CONDITION(da.task_rc)) {
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
                                da.skey, da.scode);
                        for(i=0;i<strlen(c_code);i++) {
                                if (c_code[i] == 'X')
                                        buff[i] = 'X';
                        }
                        if (!strcmp(buff, c_code)) {
                                found = TRUE;
                        }
                }
        } else {
                condition_code = atoi(c_code);
                /****************************************************/
                /* This switch statement is used for the 4 digit    */
                /* condition codes (refer to the CIS).              */
                /****************************************************/
                switch(condition_code) {
                case 1: /* DM_STATUS_GOOD */
                        if (da.task_rc == DM_STATUS_GOOD)
                                found = TRUE;
                        break;
                case 2:
                        if (da.task_rc == DM_STATUS_BAD)
                                found = TRUE;
                        break;
                case 5: /* User selected item #1. */
                        if (da.task_rc == 1)
                                found = TRUE;
                        break;
                case 6: /* User selected item #2. */
                        if (da.task_rc == 2)
                                found = TRUE;
                        break;
                case 7: /* User selected item #3. */
                        if (da.task_rc == 3)
                                found = TRUE;
                        break;
                case 8: /* User selected item #4. */
                        if (da.task_rc == 4)
                                found = TRUE;
                        break;
                case 9:
                        if ((da.task_rc == DIAG_ASL_OK) ||
                            (da.task_rc == DIAG_ASL_ENTER))
                                found = TRUE;
                        break;
                case 10:
                        if ((da.task_rc == DM_USER_CANCEL) ||
                            (da.task_rc == DM_USER_EXIT))
                                found = TRUE;
                        break;
                case 11:
                        if (da.task_rc == NO_DEVICES)
                                found = TRUE;
                        break;
                case 12:
                        if ((da.task_rc == SCATU_GOOD) && CNTRL_IS_ACTIVE)
                                found = TRUE;
                        break;
                case 13:
                        if ((da.task_rc == SCATU_GOOD) && CNTRL_IS_PASSIVE)
                                found = TRUE;
                        break;
                case 14:
                        if ((da.task_rc == SCATU_GOOD) && ADVANCED)
                                found = TRUE;
                        break;
                case 15:
                        if ((da.task_rc == SCATU_GOOD) && ADVANCED &&
                            (iplmode != DIAG_TRUE) &&
                            CNTRL_IS_PASSIVE && INTERACTIVE_TEST_MODE)
                                found = TRUE;
                        break;
                case 16:
                        if ((da.task_rc == SCATU_GOOD) &&
                            CNTRL_IS_PASSIVE && INTERACTIVE_TEST_MODE)
                                found = TRUE;
                        break;
                case 17:
                        if ((da.task_rc == DM_STATUS_GOOD) &&
                            (arr.lun_counter < arr.linfo.num))
                                found = TRUE;
                        break;
                case 18:
                        if (!INTERACTIVE_TEST_MODE &&
                            (arr.pdisk_counter >= arr.num_pdisks))
                                found = TRUE;
                        break;
                case 19:
                        if (arr.pdisk_counter < arr.num_pdisks)
                                found = TRUE;
                        break;
                case 20:
                        if (arr.pdisk_counter >= arr.num_pdisks)
                                found = TRUE;
                        break;
                case 21:
                        if (da.task_rc == DM_ERROR_EINVAL)
                                found = TRUE;
                        break;
                case 22:
                        if (EXITLM)
                                found = TRUE;
                        break;
                case 23:
                        if (da.task_rc == EICON)
                                found = TRUE;
                        break;
                case 24:
                        if (da.task_rc == EDRV)
                                found = TRUE;
                        break;
                case 25:
                        if (da.task_rc == EIO)
                                found = TRUE;
                        break;
                case 27:
                        if (!router.single_ctrl)
                                found = TRUE;
                        break;
                case 28:
                        if ((router.single_ctrl == 0) && (router.state_change))
                                found = TRUE;
                        break;
                case 29:
                        if ((DAC_ID_ODD(dac_scsi_id) &&
                            (router.ioctl.state == ACTIVE_RESET)) ||
                           ((!DAC_ID_ODD(dac_scsi_id)) &&
                            (router.ioctl.state == RESET_ACTIVE)))
                                found = TRUE;
                        break;
                case 30:
                        if (NON_ZERO_FRUC)
                                found = TRUE;
                        break;
                case 31:
                        if (NOT_READY_NO_uCODE)
                                found = TRUE;
                        break;
                case 45:
                        if ((rc = retry_mgr(CHECK_RETRY, 0)) == ERP_GOOD)
                                found = TRUE;
                        break;
                case 46:
                        if (da.task_rc == DEV_CFG_FAILED)
                                found = TRUE;
                        break;
                case 47:
                        if (da.task_rc == PAR_CFG_FAILED)
                                found = TRUE;
                        break;
                case 49:
                        if (da.task_rc == DM_ERROR_OPEN)
                                found = TRUE;
                        break;
                case 51:
                        if (da.task_rc == SCATU_TIMEOUT)
                                found = TRUE;
                        break;
                case 52:
                        if (da.task_rc == SCATU_RESERVATION_CONFLICT)
                                found = TRUE;
                        break;
                case 53:
                        if (CHECK_CONDITION(da.task_rc))
                                found = TRUE;
                        break;
                case 54: /* Also covers SCATU_BUSY. */
                        if (da.task_rc == SCATU_COMMAND_ERROR)
                                found = TRUE;
                        break;
                case 55:
                        if (da.task_rc == SCATU_IO_BUS_ERROR)
                                found = TRUE;
                        break;
                case 56:
                        if (da.task_rc == SCATU_ADAPTER_FAILURE)
                                found = TRUE;
                        break;
                case 58: /* Previous task sequence not complete. */
                        if (main_seq != da.seq)
                                found = TRUE;
                        break;
                case 59:
                        if (ELA_MODE_ONLY)
                                found = TRUE;
                        break;
                case 60:
                        found = TRUE; /* ANY CONDITION */
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
        char *ptr;
        char *temp_ptr;

        /* Get an ERP (condition_code:reaction_code) from the ERP string. */
        ptr = str_parse(da.erp_str, " ", erp_count);

        /* Get the condition or reaction code from the ERP. */
        temp_ptr = str_parse(ptr, ":", (type & 0x0F));

        if (temp_ptr == (char *)NULL) {
                dt(25,"ERROR: ERP CODE");
                clean_up(DM_ERROR_OTHER);
        }

        return(temp_ptr);

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
        char *ptr;
        char *r_ptr;
        int i,j,rc;
        int erp_rc = ERP_FAIL;
        int reaction_code;
        int mult_reactions = FALSE;
        int count = 1;
        char *ms_buffer;
        struct CuAt cuat;
        struct CuVPD *cuvpd;
        struct listinfo list_info;
        struct lun *lun_ptr;
        char *temp_ptr;

        r_ptr = str_parse(r_code_str, ",", count++);
        if (r_ptr == (char *)NULL) {
                dt(25,"r_code == NULL");
                return(ERP_FAIL);
        }

        do {
                reaction_code = atoi(r_ptr);
                dt(10,1,"reaction code", reaction_code);
                /****************************************************/
                /* This switch statement is used for all reactions. */
                /****************************************************/
                switch(reaction_code) {
        /* General DA */
                case 1: /* Continue testing. */
                        erp_rc = retry_mgr(CLEAR_RETRY, 0);
                        break;
                case 2: /* Retry task. */
                        erp_rc = ERP_RETRY;
                        break;
                case 3: /* Restart DA */
                        while (da.seq != (struct da_seq *)NULL)
                                seq_mgr(SEQ_DELETE);
                        seq_mgr(SEQ_CREATE);
                        main_seq = da.seq;   /* Save the pointer (head ptr). */
                        da.seq->task_str = get_data(SEQUENCE_DATA,Main_DA_Seq);
                        if (da.seq->task_str == (char *)NULL) {
                                dt(25,"ERROR: TASK");
                                return(DM_ERROR_ODM);
                        }
                        da.seq->step = 0;
                        erp_rc = ERP_GOOD;
                        dt(25,"Restart da.");
                        break;
                case 4: /* Set device reserved flag. This will cause  */
                             /* a RELEASE_UNIT to be issued in clean_up(). */
                        da_flags |= DA_RESERVED_DEV;
                        erp_rc = ERP_GOOD;
                        break;
                case 5: /* Reset step count to begining of currnet */
                             /* task sequence.                          */
                        da.seq->step = 0;
                        erp_rc = ERP_GOOD;
                        break;
        /* ASL type reaction codes (message, menu goal, etc). */
               case 6:
                        if (INTERACTIVE_TEST_MODE) {
                                rc = display_screen("3004");
                                if ((rc == DM_USER_CANCEL) ||
                                    (rc == DM_USER_EXIT)) {
                                        clean_up(rc);
                                }
                        } else {
                                clean_up(DM_STATUS_GOOD);
                        }
                        erp_rc = ERP_GOOD;
                        break;
        /* 7135 Array reaction codes. */
                case 15:
                        dev_lun = dac_lun_id;
                        erp_rc = ERP_GOOD;
                        break;
                case 16:
                        dev_lun = atoi((arr.cudv+arr.lun_counter)->connwhere);
                        erp_rc = ERP_GOOD;
                        break;
                case 17: /* Get dac type from CuVPD. */
                        sprintf(attr, "name = %s", tm_input.dname);
                        cuvpd = get_CuVPD_list(CuVPD_CLASS, attr,
                                                &list_info, 1, 2 );
                        ptr = strstr(cuvpd->vpd,"7135");
                        if (!strncmp(ptr, INF_VPD_STR, strlen(INF_VPD_STR))) {
                                /* dac is Infiniti, change FFC. */
                                dac_ffc = INF_DAC_FFC;
                        }
                        dt(20,1,"dac CuVPD",ptr);
                        dt(16,1,"dac FFC", dac_ffc);
                        erp_rc = ERP_GOOD;
                        break;
                case 18: /* Add dac device(s) to array status. */
                        erp_rc = raid_array_status(CNTRL);
                        erp_rc = ERP_GOOD;
                        break;
                case 19: /* Build the array status secreen. */
                        raid_build_msg();
                        erp_rc = ERP_GOOD;
                        break;
                case 20: /* Get the dar name from CuAt for this dac device. */
                        sprintf(attr, "name LIKE dar* and value LIKE '*%s*'",
                                tm_input.dname);
                        if (odm_get_first(CuAt_CLASS, attr, &cuat)) {
                                strcpy(arr.dar_name, cuat.name);
                        } else {
                                dt(25,"ERROR: dar not found in CuAt.");
                                /* Continue on to determine the problem. */
                                /* May need to load ucode to controller. */
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 21: /* Add hdisk to array status. */
                        erp_rc = raid_array_status(LUN);
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
                        ++arr.lun_counter;
                        dt(10,2,"lun counter",arr.lun_counter,"linfo",
                           arr.linfo.num);
                        erp_rc = ERP_GOOD;
                        break;
                case 22: /* Get the name of the next LUN. */
                        strcpy(arr.lun_name,
                               (arr.cudv + arr.lun_counter)->name);
                        dt(20,1,"LUN name",arr.lun_name);
                        erp_rc = ERP_GOOD;
                        break;
                case 23: /* Add dead hdisk to array status. */
                        erp_rc = raid_array_status(LUN | DEAD_LUN);
                        dt(25,"Dead LUN");
                        if (fdes != -1) {
                                close(fdes);
                        }
                        if (arr.lun_cfg_state == DEFINED) {
                                arr.lun_cfg_state = -1;
                                sprintf(attr, " -l %s",
                                        (arr.cudv + arr.lun_counter)->name);
                                rc = run_method(arr.lun_name, UCFG_METHOD,
                                                attr);
                        }
                        ++arr.lun_counter;
                        dt(10,2,"lun counter",arr.lun_counter,"linfo",
                           arr.linfo.num);
                        erp_rc = ERP_GOOD;
                        break;
                case 24: /* Add hdisk open error to array status. */
                        rc = raid_array_status(LUN | OPEN);
                        erp_rc = ERP_GOOD;
                        break;
                case 25:
                        ++arr.lun_counter;
                        erp_rc = ERP_GOOD;
                        break;
                case 26: /* Configure the dar. */
                        arr.dar_cfg_state = get_cfg_state(arr.dar_name);
                        sprintf(attr, " -l %s", arr.dar_name);
                        rc = run_method(arr.dar_name, CFG_METHOD, attr);
                        dt(10,1,"cfgdar rc", rc);
                        erp_rc = ERP_GOOD;
                        break;
                case 27: /* Call cfgdac to update ODM information. */
                        sprintf(attr, " -l %s", tm_input.dname);
                        erp_rc = run_method(tm_input.dname, CFG_METHOD, attr);
                        dt(10,1,"  cfgdac rc", erp_rc);
                        erp_rc = ERP_GOOD;
                        break;
                case 28: /* Get the CH/ID of the next pdisk. */
                        lun_ptr = raid_hptr;
                        arr.pdisk_ch = 0;
                        while (lun_ptr != (struct lun *)NULL) {
                                arr.pdisk_ptr = lun_ptr->pdisk;
                                while (arr.pdisk_ptr != (struct pdisk *)NULL) {
                                        if (!(arr.pdisk_ptr->flags & PDISK_DIAGNOSED) &&
                                            (arr.pdisk_ptr->status_msg == FORMAT_INIT)) {
                                                /* Dont test this drive, it will */
                                                /* timeout on the TUR command.   */
                                                ++arr.pdisk_counter;
                                                arr.pdisk_ptr->flags |=
                                                         PDISK_DIAGNOSED;
                                        }
                                        if (!(arr.pdisk_ptr->flags & PDISK_DIAGNOSED)) {
                                               arr.pdisk_ch = arr.pdisk_ptr->ch;
                                               arr.pdisk_id = arr.pdisk_ptr->id;
                                               ++arr.pdisk_counter;
                                               arr.pdisk_ptr->flags |=
                                                        PDISK_DIAGNOSED;
                                               break;
                                        } else {
                                               arr.pdisk_ptr =
                                                    arr.pdisk_ptr->next;
                                        }
                                }
                                if (arr.pdisk_ch) {
                                        /* Found it, get out. */
                                        break;
                                } else {
                                        /* Try next LUN. */
                                        lun_ptr = lun_ptr->next;
                                }
                        }
                        if (!arr.pdisk_ch) {
                                dt(25,"ERROR: PDISK CH");
                                clean_up(DM_ERROR_OTHER);
                        }
                        erp_rc = ERP_GOOD;
                        dt(10,2,"CH",arr.pdisk_ch,"ID",arr.pdisk_id);
                        break;
                case 29: /* Get 7 bytes of the Inquiry data product ID. */
                        strncpy(arr.pdisk_ptr->inq_pid,
                                &tucb.scsitu.data_buffer[16],
                                (INQ_PID_SIZE - 1));
                        arr.pdisk_ptr->inq_pid[(INQ_PID_SIZE - 1)] = '\0';
                        erp_rc = ERP_GOOD;
                        break;
                case 30:
                        da.seq->step = 0;
                        erp_rc = ERP_GOOD;
                        break;
                case 31: /* Configure the LUN. */
                        dt(20,1,"arr.lun_name",
                           (arr.cudv+arr.lun_counter)->name);
                        sprintf(attr, " -l %s",
                                (arr.cudv + arr.lun_counter)->name);
                        arr.lun_cfg_state = get_cfg_state(
                                            (arr.cudv + arr.lun_counter)->name);
                        dt(10,1,"LUN cfg state", arr.lun_cfg_state);
                        rc = run_method((arr.cudv + arr.lun_counter)->name,
                                        CFG_METHOD, attr);
                        dt(10,1,"LUN cfg rc", rc);
                        if (rc != 0) {
                                rc = raid_array_status(LUN | DEAD_LUN);
                                ++arr.lun_counter;
                                /* Return to previous sequence. */
                                seq_mgr(SEQ_DELETE);
                                erp_rc = ERP_RETRY;
                        } else {
                                erp_rc = ERP_GOOD;
                        }
                        break;
                case 32: /* Get all the array LUN's (hdisks) from CuDv. */
                        sprintf(attr, "parent = %s AND chgstatus != 3",
                                arr.dar_name);
                        dt(20,1,"attr",attr);
                        arr.cudv = get_CuDv_list(CuDv_CLASS, attr,
                                                 &arr.linfo, 1, 2);
                        if ((arr.cudv == (struct CuDv *)(-1)) ||
                            (arr.cudv == (struct CuDv *)NULL)) {
                                /* May be a valid condition. */
                                dt(25,"No LUN's found in CuDv.");
                        }
                        dt(10,1,"arr.linfo", arr.linfo.num);
                        erp_rc = ERP_GOOD;
                        break;
                case 33:
                        /* Call raid fru for multiple subsystem failures. */
                        for (i = 200; i < 215; i++) {
                                sprintf(arr.fru_info, "%X:%X:%d", dac_ffc, 06, i);
                                rc = raid_fru(FRUC_FRU_TYPE);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 34:
                        memcpy(ms_2A_buff, tucb.scsitu.data_buffer,
                               (unsigned)tucb.scsitu.data_length);
                        erp_rc = ERP_GOOD;
                        break;
                case 35: /* Add spares to array status. */
                        erp_rc = raid_array_status(SPARE);
                        erp_rc = ERP_GOOD;
                        break;
                case 37: /* Initialize. */
                        dac_ffc = 0;
                        arr.dar_cfg_state = -1;
                        arr.lun_counter = 0;
                        arr.linfo.num = 0;
                        arr.pdisk_counter = 0;
                        arr.num_pdisks = 0;
                        arr.spt_flag = 0;
                        router.single_ctrl = 0;
                        router.prev_state = -1;
                        router.state_change = 0;
			router.ioctl.ctrl_path[0].ctrl_name[0] = '\0';
			router.ioctl.ctrl_path[1].ctrl_name[0] = '\0';
                        for (i = 0; i < MAX_SUPPORTED_CHs; i++) {
                                for (j = 0; j < MAX_SUPPORTED_IDs; j++) {
                                        lun_owns_pdisk[i][j] = 0;
                                }
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 38: /* Get the device CuDv information from ODM. */
                        sprintf(attr, "name = %s", tm_input.dname);
                        cudv = get_CuDv_list(CuDv_CLASS, attr, &linfo, 1, 2);
                        if ((cudv == (struct CuDv *)(-1)) ||
                            (cudv == (struct CuDv *)NULL)) {
                                /* Error getting device info. */
                                dt(25,"ERROR: CUDV");
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 39: /* Get the SCSI ID of the dac. */
                        if (diag_get_sid_lun(cudv->connwhere,
                            &dac_scsi_id, &dac_lun_id) == -1) {
                                dt(25,"ERROR: DAC SCSI ID");
                                clean_up(DM_ERROR_UNEXPECTED);
                        }
                        erp_rc = ERP_GOOD;
                        break;
                case 40:
                        dac_ffc = cudv->PdDvLn->led;
                        erp_rc = ERP_GOOD;
                        break;
                case 41: /* Check for hdisk device type for ELA. */
        		if (!strncmp(tm_input.dname, "hdisk", 5)) {
                		if (ELA_MODE_ONLY) {
                        		strcpy(arr.dar_name, tm_input.parent);
                		} else {
                        		clean_up(DM_STATUS_GOOD);
                		}
        		}
                        erp_rc = ERP_GOOD;
                        break;
                case 44: /* Save current router state. */
                        router.prev_state = router.ioctl.state;
                        dt(16,1,"router.prev_state",router.prev_state);
                        if (router.ioctl.state != DUAL_ACTIVE) {
                                dt(25,"Router state changed.");
                                router.state_change = 1;
                        }
                        if((strlen(router.ioctl.ctrl_path[0].ctrl_name) == 0)||
                           (strlen(router.ioctl.ctrl_path[1].ctrl_name) == 0)){
                                router.single_ctrl = 1;
                        }
                        dt(10,1,"single_ctrl",router.single_ctrl);
			dt(20,1,"path 0, ctrl_name",
			   router.ioctl.ctrl_path[0].ctrl_name);
			dt(20,1,"path 1, ctrl_name",
			   router.ioctl.ctrl_path[1].ctrl_name);
                        erp_rc = ERP_GOOD;
                        break;
                case 45: /* Change router state to make this dac active. */
                        router.ioctl.state = DUAL_ACTIVE;
                        erp_rc = ERP_GOOD;
                        break;
                case 46: /* Restore previous router state. */
                        if (DAC_ID_ODD(dac_scsi_id)) {
                                router.ioctl.state =
                                        ((router.prev_state & 0x0F) << 4);
                                router.ioctl.state |=
                                        ((router.prev_state & 0xF0) >> 4);
                        } else {
                                router.ioctl.state = router.prev_state;
                        }
                        erp_rc = ERP_GOOD;
                        break;
		case 47: /* Get microcode level from Inquiry data. */
			if (ADVANCED) {
                        	sprintf(arr.pdisk_ptr->ucode_level,
                                	"(%02.2X%02.2X)",
                                	(unsigned)tucb.scsitu.data_buffer[14],
                                	(unsigned)tucb.scsitu.data_buffer[15]);
			} else {
				strcpy(arr.pdisk_ptr->ucode_level, " ");
			}
                        erp_rc = ERP_GOOD;
                        break;
                case 55: /* Go ahead two steps (1 + GOOD). */
                        da.seq->step += 2;
                        erp_rc = ERP_GOOD;
                        break;
                case 56: /* Go ahead three steps (2 + GOOD). */
                        da.seq->step += 3;
                        erp_rc = ERP_GOOD;
                        break;
                case 57: /* Set SPT flag. */
                        arr.spt_flag = 1;
                        erp_rc = ERP_GOOD;
                        break;
                case 58: /* Clear SPT flag. */
                        arr.spt_flag = 0;
                        erp_rc = ERP_GOOD;
                        break;
                case 67: /* Clear retry entry for this step. */
                        erp_rc = retry_mgr(CLEAR_RETRY, 0);
                        break;
                case 68: /* Go ahead 1 step (1 + RETRY). */
                        da.seq->step += 1;
                        erp_rc = ERP_GOOD;
                        break;
                case 69:
                        da.seq->step -= 1;
                        erp_rc = ERP_RETRY;
                        break;
                case 70: /* Exit DA with good status. */
                        clean_up(DM_STATUS_GOOD);
                        break;
                case 71: /* Exit DA with bad status. */
                        clean_up(DM_STATUS_BAD);
                        break;
                case 72: /* Exit DA with more continue. */
                        clean_up(DM_MORE_CONT);
                        break;
                case 73: /* Exit DA with error open. */
                        clean_up(DM_ERROR_OPEN);
                        break;
                case 74: /* Exit the DA with a Software Error. */
                        dt(25,"ERROR: DAEE");
                        clean_up(DM_ERROR_OTHER);
                        break;
                case 75: /* Exit DA with unexpected error. */
                        clean_up(DM_ERROR_UNEXPECTED);
                        break;
                case 76: /* Exit DA with ODM error. */
                        clean_up(DM_ERROR_ODM);
                        break;
                case 90: /* Unknown Disk FRU. */
                        sprintf(arr.fru_info, "%X:%X:%d:%d", dac_ffc, 0x310,
                                arr.pdisk_ch, arr.pdisk_id);
                        erp_rc = raid_fru(UNKNOWN_PDISK_FRU_TYPE);
                        /* Continue on. */
                        erp_rc = ERP_GOOD;
                        break;
                case 91: /* Format pdisk. */
                        rc = display_menugoal("9042");
                        erp_rc = ERP_GOOD;
                        break;
                case 100: /* Callout FRU according to task number. */
                        sprintf(arr.fru_info, "%X:%d", dac_ffc, atoi(da.task));
                        erp_rc = raid_fru(TASK_FRU_TYPE);
                        clean_up(DM_STATUS_BAD);
                        break;
                case 102: /* Callout FRU for a non-zero Sense Data FRUC. */
                        if (SENSE_DATA_BYTE_14 >= 0x10) {
                                /* Call out an SRN for the physical disk. */
                                rc = get_pdisk_ffc(&da.sdata[83]);
                                sprintf(arr.fru_info, "%X:%X:%X:%d",
                                        0x845, 0x200, (SENSE_DATA_BYTE_14 > 4),
                                        (SENSE_DATA_BYTE_14 & 0x0F), rc);
                                rc = raid_fru(PDISK_FRUC_FRU_TYPE);
                        } else {
                                sprintf(arr.fru_info, "%X:%X", dac_ffc,
                                        SENSE_DATA_BYTE_14);
                                erp_rc = raid_fru(FRUC_FRU_TYPE);
                        }
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
                case 105: /* End of sequence, exit DA with good status. */
                        clean_up(DM_STATUS_GOOD);
                        break;
                case 108: /* Get disk capacity from CuAt. */
                        strcpy(arr.pdisk_ptr->capacity,
                               get_pdisk_capacity(arr.dar_name,
                                                  arr.pdisk_ptr->ch,
                                                  arr.pdisk_ptr->id));
                        erp_rc = ERP_GOOD;
                        break;
                case 109: /* Get disk type from CuAt. */
                        strcpy(arr.pdisk_ptr->inq_pid,
                               get_pdisk_pid(arr.dar_name,
                                                  arr.pdisk_ptr->ch,
                                                  arr.pdisk_ptr->id));
                        erp_rc = ERP_GOOD;
                        break;
                case 110: /* Get pdisk FFC from PDiagAtt. */
                        arr.pdisk_ptr->ffc =
                                get_pdisk_ffc(arr.pdisk_ptr->inq_pid);
                        erp_rc = ERP_GOOD;
                        break;
                default:  /* Check for special reaction codes. */
                        if (REACTION_IS(SEQUENCE_REACTION, r_ptr)) {
                                /* Perform a Test Sequence. */
                                seq_mgr(SEQ_CREATE);
                                da.seq->task_str =
                                             get_data(SEQUENCE_DATA,
                                      atoi(&r_ptr[strlen(SEQUENCE_REACTION)]));
                                if (da.seq->task_str == (char *)NULL) {
                                        dt(10,1,"ERROR: TASK SEQ",erp_rc);
                                        return(DM_ERROR_ODM);
                                }
                                erp_rc = ERP_GOOD;
                        } else if (REACTION_IS(MENUGOAL_REACTION, r_ptr)) {
                                rc = display_menugoal(r_ptr);
                                erp_rc = ERP_GOOD;
                        } else if (REACTION_IS(SRN_REACTION, r_ptr)) {
                                sprintf(arr.fru_info, "%X:%X", dac_ffc,
                                        strtol(&r_ptr[strlen(SRN_REACTION)],ep,16));
                                erp_rc = raid_fru(SPECIFIC_FRU_TYPE);
                                clean_up(DM_STATUS_BAD);
                        } else if (REACTION_IS(RETRY_REACTION, r_ptr)) {
                                /* set max retry count to last two digits. */
                                erp_rc = retry_mgr(RETRY_RETRY,
                                     atoi(&r_ptr[strlen(RETRY_REACTION)]));
                        } else {
                                erp_rc = ERP_FAIL;
                        }
                        break; /* default */
                } /* end switch */

                if (erp_rc == ERP_GOOD) {
                        /* Check for multiple reaction codes. */
                        r_ptr = str_parse(r_code_str, ",", count++);
                }
        } while ((r_ptr != (char *)NULL) && (erp_rc == ERP_GOOD));

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
        int i, rc, ic = 0; /* ic = item counter */
        char *cmdblk;
        char *erp;
        char *paramlist;
        char *data_ptr;
        char *temp_ptr;

        /* Get the SCATU TUCB data. */
        data_ptr = get_data(SCATU_TUCB_DATA, atoi(&task[1]));
        if (data_ptr == (char *)NULL) {
                return(DM_ERROR_ODM);
        }

        /* The first TUCB item is the SCATU number */
        /* from the scsi_atu.h header file.        */
        tucb.header.tu = strtol(str_parse(data_ptr, ",", ++ic), ep, 16);

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
        tucb.scsitu.flags =
                tucb_flags[atoi(str_parse(data_ptr,",",++ic))];
        tucb.scsitu.ioctl_pass_param =
                tucb_passthru[atoi(str_parse(data_ptr,",",++ic))];
        /* SET PASSTHROUGH direction bit. Used in ptrs_ioctl(). */
        tucb.header.r1 = atoi(str_parse(data_ptr,",",++ic));

        /* Get the CDB, PL and ERP data. */
        temp_ptr = str_parse(data_ptr,",", ++ic);

        cmdblk = str_parse(temp_ptr,":", 1);
        paramlist = str_parse(temp_ptr,":", 2);
        erp = str_parse(temp_ptr,":", 3);

        /* Get the SCATU Command Block data. */
        data_ptr = get_data(SC_CDB_DATA, atoi(cmdblk));
        if (data_ptr == (char *)NULL) {
                dt(25,"ERROR: TUCB CDB");
                return(DM_ERROR_ODM);
        }

        /* Fill in the SCSI Command Block */
        for (i = 0; i < tucb.scsitu.command_length; i++) {
                tucb.scsitu.scsi_cmd_blk[i] =
                        strtol(str_parse(data_ptr, " ", i+1), ep, 16);
        }

        /* Add the LUN value to the CDB. */
        if (arr.spt_flag) {
                tucb.scsitu.scsi_cmd_blk[1] |= arr.pdisk_ptr->lun_id << 5;
        } else {
                tucb.scsitu.scsi_cmd_blk[1] |= dev_lun << 5;
        }

        /* Check for Parameter List Data (if any). */
        if (tucb.scsitu.data_length) {
                if (strcmp(paramlist, "n/a")) {
                        /* Call cat_data() to get the SCATU Parameter */
                        /* List data and convert it to a data buffer. */
                        data_ptr = cat_data(SC_PL_DATA, " ", ";",
                                            paramlist);
                        if (data_ptr == (char *)NULL) {
                                dt(20,1,"ERROR: PL", paramlist);
                                return(DM_ERROR_ODM);
                        }
                        for (i = 0; i < tucb.scsitu.data_length; i++) {
                                tu_buffer[i] =
                                        strtol(str_parse(data_ptr, " ", i+1),
                                               ep, 16);
                        }
                }
        }
        tucb.scsitu.data_buffer = tu_buffer;

        /* Get the ERP(s) for this SCATU. */
        da.erp_str = cat_data(ERP_DATA, " ", ";", erp);
        if (da.erp_str == (char *)NULL) {
                dt(25,"ERROR: TUCB ERP");
                return(DM_ERROR_ODM);
        }

        return(DM_STATUS_GOOD);

} /* end init_tucb() */

/*
 *
 * NAME: rs_ioctl()
 *
 * FUNCTION: Call the MODES SENSE ioctl() for a LUN.
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

        dt(25,"In lun_ioctl().");

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

        dt(30,iocmd_rs.command_length, iocmd_rs.scsi_cdb);
        dt(25,"Call the ioctl().");

        rc = ioctl(fdes, DKIOCMD_RS, &iocmd_rs);

        errno_rs = errno;
        dt(10,1,"  DKIOCMD_RS rc", rc);

        if (rc == 0) {
                return(SCATU_GOOD);
        }
        dt(10,1,"  DKIOCMD_RS errno", errno_rs);
        dt(16,1,"  DKIOCMD_RS status validity", iocmd_rs.status_validity);

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
                        da.skey  = iocmd_rs.request_sense_ptr[2] & 0x0F;
                        da.scode = (uchar)iocmd_rs.request_sense_ptr[13] |
                           ((uchar)iocmd_rs.request_sense_ptr[12] << 8);
                        memcpy(tucb.scsitu.data_buffer,
                               iocmd_rs.request_sense_ptr,
                               iocmd_rs.req_sense_length);
                        tucb.scsitu.data_length = iocmd_rs.req_sense_length;
                        da.sdata = tucb.scsitu.data_buffer;
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

        /* Fill in the tucb info. */
        iocmd_ptrs.data_length = tucb.scsitu.data_length;
        iocmd_ptrs.flags = tucb.scsitu.flags;
        iocmd_ptrs.buffer = tucb.scsitu.data_buffer;
        iocmd_ptrs.timeout_value = tucb.scsitu.cmd_timeout;
        iocmd_ptrs.q_tag_msg = SC_SIMPLE_Q;
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

        rc = ioctl(fdes, DKIOCMD_PTRS2, &iocmd_ptrs);
        errno_ptrs = errno;

        dt(10,1,"  DKIOCMD_PTRS2 rc", rc);

        if (rc == 0) {
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
                        da.skey  = iocmd_ptrs.request_sense_ptr[2] & 0x0F;
                        da.scode = (uchar)iocmd_ptrs.request_sense_ptr[13] |
                           ((uchar)iocmd_ptrs.request_sense_ptr[12] << 8);
                        memcpy(tucb.scsitu.data_buffer,
                               iocmd_ptrs.request_sense_ptr,
                               iocmd_ptrs.req_sense_length);
                        tucb.scsitu.data_length = iocmd_ptrs.req_sense_length;
                        da.sdata = tucb.scsitu.data_buffer;
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
        int rc = 0;
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
        switch(method) {
        case CFG_METHOD:
                rc = odm_run_method(MKDEV, method_criteria, &outbuf,&errbuf);
                        break;
        case CHG_METHOD:
                rc = odm_run_method(CHDEV, method_criteria, &outbuf,&errbuf);
                break;
        case UCFG_METHOD:
                rc = odm_run_method(RMDEV, method_criteria, &outbuf,&errbuf);
                break;
        default:
                dt(10,1,"ERROR: run_method", method);
                rc = -3;
                break;
        }
        dt(10,1,"run_method() rc", rc);
        if (outbuf != (char *)NULL)
                free(outbuf);
        if (errbuf != (char *)NULL)
                free(errbuf);

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
        char *temp;
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

        dt(20,1,"In cat_data, ptr", ptr);
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
        int rc, rc1;


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

        tucb.scsitu.scsi_cmd_blk[1] |= dev_lun << 5;
        tucb.scsitu.scsi_cmd_blk[2] = 0;
        tucb.scsitu.scsi_cmd_blk[3] = 0;
        tucb.scsitu.scsi_cmd_blk[4] = 0xFF;
        tucb.scsitu.scsi_cmd_blk[5] = 0;
        tucb.scsitu.data_length = 0xFF;
        tucb.scsitu.data_buffer = tu_buffer;

        /* dt(2,&tucb); */
        rc = exectu(fdes, &tucb);
        dt(10,1,"Request Sense rc",rc);
        if (rc != SCATU_GOOD) {
                /* Retry */
                dt(10,1,"WARNING: REQUEST SENSE rc",rc);
                rc = exectu(fdes, &tucb);
        }
        if (rc == SCATU_GOOD) {
                da.skey  = tucb.scsiret.sense_key;
                da.scode = tucb.scsiret.sense_code;
                da.sdata = tucb.scsitu.data_buffer;
                dt(3,&tucb);
        } else {
                da.skey  = 0;
                da.scode = 0;
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
        int dev_selected;
        int i, rc;
        int item;
        int scr_type;
        int scr_alloc;
        long scr_num;
        char *cat_nums;
        char *odm_scr_data;
        char *scr_info;
        ASL_SCR_INFO *menuinfo;
        ASL_SCR_TYPE *menutype;

        dt(20,1, "In display_screen(), screen", screen);

        scr_num = atoi(&screen[1]);

        /* Get the screen data. */
        odm_scr_data = get_data(ASL_SCREEN_DATA, scr_num);
        if (odm_scr_data == (char *)NULL) {
                dt(25,"ERROR: ODM SCR DATA.");
                return(DM_ERROR_ODM);
        }

        /* Get the ERP for this screen. */
        scr_info = str_parse(odm_scr_data, ":", 3);
        da.erp_str = cat_data(ERP_DATA, " ", ";", scr_info);
        if (da.erp_str == (char *)NULL) {
                dt(20,1,"ERROR: SCR ERP");
                return(DM_ERROR_ODM);
        }

        if (!CONSOLE) {
                dt(25,"No console mode.");
                return(DIAG_ASL_OK);
        }

        /* Get the screen info from scr_data. */
        scr_info = str_parse(odm_scr_data, ":", 1);

        /* Get the screen type from scr_info. */
        scr_type  = atoi(str_parse(scr_info, ",", 1));

        if ((SCREEN_IS_INTERACTIVE(scr_type)) && (!INTERACTIVE_TEST_MODE)) {
                dt(25,"Not iteractive test mode.");
                return(DIAG_ASL_OK);
        }

        /* Get the allocation size of the ASL_INFO srtuct. */
        scr_alloc = atoi(str_parse(scr_info, ",", 2));

        /* Allocate for the ASL_INFO struct. */
        menuinfo = (ASL_SCR_INFO *)calloc(scr_alloc, sizeof(ASL_SCR_INFO));

        /* For each message, call alloc_msg() to allocate */
        /* for the message and do any sustitutions.       */
        item = 1;
        for (i = 0; i < scr_alloc; i++) {
                /* Allocate for the message and populate substitutions. */
                menuinfo[i].text = alloc_msg(odm_scr_data, &item);
                ++item;
        }

        menutype = (ASL_SCR_TYPE *)calloc(1, sizeof(ASL_SCR_TYPE));
        menutype->screen_code = asl_scr_types[scr_type];
        menutype->max_index = scr_alloc - 1; /* Zero based */
        menutype->cur_index = 1;

        /* Put the screen number in HEX for diag_display(). */
        scr_num = (dac_ffc * 0x1000) + strtol(&screen[1], ep, 16);
        /* Display the message or selection menu.  */
        rc = diag_display(scr_num, catd, NULL, DIAG_IO,
                          asl_scr_types[scr_type], menutype, menuinfo);

        if (rc == DIAG_ASL_CANCEL) {
                return(DM_USER_CANCEL);
        }
        if (rc == DIAG_ASL_EXIT) {
                return(DM_USER_EXIT);
        }

        if (scr_type <= 3) {
                rc = DIAG_ITEM_SELECTED(*menutype);
        }

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

char *alloc_msg(char *scr_data, int *item) {
        int i, rc;
        int cat_num;
        int more_devs = FALSE;
        int found_one = FALSE;
        int test_time;
        char *cat_ptr;
        char *buff;
        char *ptr;

        /* The message numbers start after the screen info. */
        /* Get the catalog numbers. */
        ptr = str_parse(scr_data, ":", 2);

        cat_num = atoi(str_parse(ptr, " ", *item));
        dt(10,1,"cat_num",cat_num);

        /* Take care of common substitutions. */
        switch(cat_num) {
        case TITLE:
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET, cat_num);
                ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET,
                                            ADVANCED_MODE);
                buff = (char *)malloc(strlen(cat_ptr) + NAMESIZE + LOCSIZE +
                                      strlen(ptr) + 5);
                if (ADVANCED) {
                        sprintf(buff, cat_ptr, tm_input.dname,
                                tm_input.dnameloc, ptr);
                } else {
                        sprintf(buff, cat_ptr, tm_input.dname,
                                tm_input.dnameloc, " ");
                }
                break;
        case STAND_BY:
                /* Get the test time (in minutes) for the message. */
                ptr = str_parse(scr_data, ":", 1);
                test_time = atoi(str_parse(ptr, ",", 3));
                /* If testing multiple times, display the loopmode message. */
                if (INLM || EXITLM || (SYSTEM && ENTERLM)) {
                        cat_num = LOOPMODE;
                }
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + 1);
                if (cat_num == LOOPMODE) {
                        sprintf(buff, cat_ptr, test_time,
                                tm_input.lcount, tm_input.lerrors);
                } else {
                        sprintf(buff, cat_ptr, test_time);
                }
                break;
        /* 7135 Array messages. */
        case ARRAY_STATUS_ITEM:
                raid_build_msg();
                buff = (char *)malloc(strlen(array_status) + 1);
                strcpy(buff, array_status);
                break;
        case GET_LUN_STATUS:
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + (NAMESIZE * 2) + 2);
                sprintf(buff, cat_ptr,
                       (arr.cudv + arr.lun_counter)->name,
                       (arr.cudv + arr.lun_counter)->location);
                break;
        case DETECT_SPARES:
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + NAMESIZE);
                sprintf(buff, cat_ptr,
                        pdisk_locs[arr.pdisk_ch][arr.pdisk_id]);
                break;
        default:
                /* No substitution required. */
                cat_ptr = (char *)diag_cat_gets(catd, D7135_MSG_SET, cat_num);
                buff = (char *)malloc(strlen(cat_ptr) + 1);
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

        if (!CONSOLE) {
                return;
        }
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
        char *ptr;
        char *r_ptr = (char *)NULL;
        char buff[1024];
        int count = 1;

        strcpy(buff, str_to_parse);

        ptr = strtok(buff, delimeter);
        while (ptr != NULL && count != item_number) {
                ptr = strtok( NULL, delimeter);
                if (ptr != (char *)NULL) {
                        --item_number;
                }
        }

        if (count == item_number) {
                r_ptr = (char *)malloc(strlen(ptr) + 1);
                strcpy(r_ptr, ptr);
        }

        return(r_ptr);

} /* end str_parse() */

/*
 *
 * NAME: display_menugoal()
 *
 * FUNCTION: Populate a menugoal to be displayed by the Diag Controller
 *           after the DA exits.
 *
 * NOTES: - For each menugoal, the size (mng_size) must be adjusted for any
 *          substitutions.
 *
 * RETURNS: DM_STATUS_GOOD after sucessfully populating a menugoal.
 *          DM_ERROR_OTHER with appropriate debug message.
 *
 */

int display_menugoal(char *task) {
        int rc;
        int cat_num = UNEXPECTED_ERROR;
        int mng_num;
        int scr_num;
        int alloc_size;
        int std_subs = 1;
        char *mng_ptr;

        dt(25,"In display_menugoal()");
        if (!CONSOLE) {
                return(DM_STATUS_GOOD);
        }

        /* Put in HEX for call to menugoal(). */
        mng_num = strtol(&task[1], ep, 16);

        check_asl();

        dt(10,1,"Menugoal",mng_num);

        /* Get the screen number by adding the menugoal */
        /* catalog number and the device led value.     */
        scr_num = mng_num + (dac_ffc * 0x1000);

        /* Add the standard title line substitutions (type, */
        /* dname and location) for the initial alloc size.  */
        alloc_size = (NAMESIZE * 2) + LOCSIZE;

        switch(mng_num) {
        case 0x042:
                cat_num = PDISK_FORMAT_PENDING;
                alloc_size += (NAMESIZE * 2) + LOCSIZE; /* Add more for pdisk subs. */
                alloc_size += strlen((char *)diag_cat_gets(catd,
                                                   D7135_MNG_SET,cat_num));
                mng_ptr = (char *)calloc(1, alloc_size + 1);
                sprintf(mng_ptr,
                        (char *)diag_cat_gets(catd, D7135_MNG_SET, cat_num),
                        scr_num, tm_input.dname, tm_input.dnameloc,
                        arr.pdisk_ptr->capacity,
                        pdisk_locs[arr.pdisk_ptr->ch][arr.pdisk_ptr->id]);
                std_subs = 0;
                break;
        case 0x043:
                cat_num = DAC_IN_RESET;
                break;
        case 0x044:
                cat_num = CHECK_CABLES;
                break;
        case 0x045:
                cat_num = UNKNOWN_LUN;
                break;
        case 0x046:
                cat_num = RUN_RAID_MANAGER;
                break;
        case 0x047:
                cat_num = NO_uCODE;
                break;
        default:
                dt(16,1,"Default menugoal",scr_num);
                break;
        }
        if (std_subs) {
                alloc_size += strlen((char *)diag_cat_gets(catd,
                                                   D7135_MNG_SET,cat_num));
                mng_ptr = (char *)calloc(1, alloc_size + 1);
                sprintf(mng_ptr,
                        (char *)diag_cat_gets(catd, D7135_MNG_SET, cat_num),
                        scr_num, tm_input.dname, tm_input.dnameloc);
        }
        rc = menugoal(mng_ptr);
        if (rc == -1) {
                dt(16,1,"ERROR: MNG", mng_num);
                return(DM_ERROR_OTHER);
        }

        return(DM_STATUS_GOOD);

} /* end display_menugoal() */

/*
 *
 * NAME: clean_up()
 *
 * FUNCTION: Clean up before exiting back to the Diag Controller. Close
 *           open files, unconfig devices, etc.
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_GOOD, DA_STATUS_BAD
 *
 */

int clean_up(int da_status) {
        int rc;

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
        dt(16,1,"---->    Status",da_status);
        dt(10,1,"---->      step",da.seq->step);

        switch(da_status) {
        case DM_ERROR_ODM:
        case DM_ERROR_UNEXPECTED:
                rc = display_menugoal("9049");
                break;
        case DM_ERROR_OTHER:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                break;
        case DM_ERROR_OPEN:
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                break;
        case DM_STATUS_BAD:
                DA_SETRC_STATUS(DA_STATUS_BAD);
                break;
        case DM_USER_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                break;
        case DM_USER_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                break;
        case DM_STATUS_GOOD:
                break;
        case ERP_FAIL:
                sprintf(arr.fru_info, "%X:%d", dac_ffc, atoi(da.task));
                rc = raid_fru(TASK_FRU_TYPE);
                break;
        default:
                break;
        }

        if (fdes > -1) {
                dt(25,"Closing device in clean_up().");
                close(fdes);
        }

        /* Run the cfgdac and cfgdar methods to update ODM again. */
	sprintf(attr, " -l %s", tm_input.dname);
	rc = run_method(tm_input.dname, CFG_METHOD, attr);
	dt(10,1,"dac ODM update cfg rc", rc);
	sprintf(attr, " -l %s", arr.dar_name);
	rc = run_method(arr.dar_name, CFG_METHOD, attr);
	dt(10,1,"dar ODM update cfg rc", rc);

        if (arr.dar_cfg_state == DEFINED) {
                /* Return device to the DEFINED state. */
                sprintf(attr, " -l %s", arr.dar_name);
                rc = run_method(arr.dar_name, UCFG_METHOD, attr);
                dt(10,1,"dar uncfg rc", rc);
        }
        if (dev_cfg_state != -1) {
                /* Return device to initial config state. */
                rc = initial_state(dev_cfg_state, tm_input.dname);
                dt(10,1,"dev initial_state rc", rc);
        }

        if (par_cfg_state != -1) {
                /* Return parent to initial config state. */
                rc = initial_state(par_cfg_state, tm_input.parent);
                dt(10,1,"par initial_state rc", rc);
        }

        /* Remove any sequence structs remaining. */
        while (da.seq != (struct da_seq *)NULL)
                seq_mgr(SEQ_DELETE);

        if (DA_INIT_ODM & da_flags)
                term_dgodm();
        if (DA_INIT_CAT & da_flags)
                catclose(catd);
        if (DA_INIT_ASL & da_flags)
                diag_asl_quit();

        dt(999);

        DA_EXIT(); /* Exit back to the Diag Controller */

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
 *      - The trace file name will be /tmp/.dt.device_logical_name
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

        sprintf(fname, "/tmp/.dt.%s", tm_input.dname);

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
                fprintf(dtfptr,"============= start %s ============\n",
                        tm_input.dname);
                fprintf(dtfptr,"ADV=%d SYS=%d SYSX=%d CON=%d (1=T 0=F)\n",
                        ADVANCED, SYSTEM, SYSX, CONSOLE);
                fprintf(dtfptr,"LM=%d (1=N 2=E 4=I 8=X)", tm_input.loopmode);
                fprintf(dtfptr," DMODE=%d (1=ELA 2=PD 4=RP)\n",
                        tm_input.dmode);
                break;
        case 2: /* print dt_tucb info before call to exectu(). */
                dt_tucb = va_arg(ag, SCSI_TUTYPE *);
                fprintf(dtfptr,"> SCSI CDB (Data Out)\n\t");
                p_data(dtfptr, dt_tucb->scsitu.scsi_cmd_blk,
                        dt_tucb->scsitu.command_length);
                if ((dt_tucb->scsitu.flags == B_WRITE) &&
                    (dt_tucb->scsitu.data_length)) {
                        fprintf(dtfptr,"> Param List (Data Out)\n\t");
                        p_data(dtfptr, dt_tucb->scsitu.data_buffer, 64);
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

