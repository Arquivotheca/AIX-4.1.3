static char sccsid[] = "@(#)19  1.4  src/bos/diag/da/pciscsi/pciscsi.c, dapciscsi, bos41J, 9522A_all 5/30/95 %%";
/*
 *   COMPONENT_NAME: DAPCISCSI
 *
 *   FUNCTIONS:         main()
 *                      display_screen()
 *                      test_device()
 *                      check_INIT_TU_rc()
 *                      check_DIAG_TU_rc()
 *                      check_SCSI_TU_rc()
 *                      check_TERM_TU_rc()
 *                      do_ela()
 *                      callout_srn()
 *                      display_menugoal()
 *                      clean_up()
 *                      check_kb()
 *                      sig_handler()
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include        "pciscsi.h"

/* Function prototypes. */
int  display_screen(int);
int  test_device(void);
int  check_INIT_TU_rc(int);
int  check_DIAG_TU_rc(int);
int  check_SCSI_TU_rc(int);
int  check_TERM_TU_rc(int);
void do_ela(void);
int  callout_srn(int);
void display_menugoal(int);
void clean_up(void);
int  check_kb(void);
void sig_handler(int);
extern char *diag_cat_gets(nl_catd, int, int);
extern int exectu(tucb_t *);
extern void dt(char *, int, ...);

struct da_struct da;    /* Struct for DA variables.      */
char DT_ID[16] = {"pci"};

/*
 *
 * NAME: main()
 *
 * FUNCTION: Initialize variables, call routines to test the device
 *           and/or do ELA.
 *
 * NOTES:
 *
 * RETURNS: Does not return, the clean_up() routine returns to the
 *          Diag Controller.
 *
 */

main(int argc, char **argv, char **envp) {
        int rc = -1;
	int resid_flags = 0;
        struct sigaction invec;  /* Signal handler structure      */

        da.flags = 0;

        /* Set Locale */
        setlocale(LC_ALL,"");

        /* Set up signal handler */
        invec.sa_handler = sig_handler;
        sigaction(SIGINT,  &invec, (struct sigaction *)NULL);
        sigaction(SIGTERM, &invec, (struct sigaction *)NULL);
        sigaction(SIGQUIT, &invec, (struct sigaction *)NULL);
        sigaction(SIGKILL, &invec, (struct sigaction *)NULL);

        /* Init DA_SETRC's */
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_USER(DA_USER_NOKEY);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);

	/* Initialize ODM. */
        if ((rc = init_dgodm()) < 0) {
		dt(DT_ID,DT_MSG,"ERROR: ODM Init failed.");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        da.flags |= DA_ODM_INIT;

	/* Get test mode information. */
        if ((rc = getdainput(&da.tm_input)) < 0) {
		dt(DT_ID,DT_MSG,"ERROR: Get DA input failed.");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

	strcpy(DT_ID, da.tm_input.dname);

	dt(DT_ID,DT_TMI,da.tm_input);

        /* Get info (integrated/failed_POST flags) from residual data. */

	rc = diag_get_device_flag(da.tm_input.dname, &resid_flags);
	if (rc == -1) {
		dt(DT_ID,DT_MSG,"ERROR: Get residual data failed.");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
	}

        /* Determine if device type is integrated. */
        if (resid_flags & INTEGRATED) {
                da.ffc = FFC_INT;   
        } else {
                /* Set a flag to indicate the SEDIFF TU is needed. */ 
		da.flags |= DETECT_DEV_TYPE;
        	da.ffc = FFC_DIFF; /* Default to DIFF */
        }
	/* If POST failed, output trace info. */
	if (resid_flags & FAILEDPOST) {
		dt(DT_ID,DT_MSG,"POST Failed bit is set.");
	}

	/* If console true, init ASL and open catalog file. */
        if (da.tm_input.console == CONSOLE_TRUE) {
                if (da.tm_input.loopmode == LOOPMODE_INLM) {
                        rc = diag_asl_init("DEFAULT");
                } else {
                        rc = diag_asl_init("NO_TYPE_AHEAD");
                }
                if (rc == -1) {
			dt(DT_ID,DT_MSG,"ERROR: ASL init failed.");
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                }
                da.flags |= DA_ASL_INIT;

                da.catd = (nl_catd)diag_catopen(MF_DPCISCSI, 0);
                da.flags |= DA_CAT_OPEN;

                if (da.tm_input.loopmode == LOOPMODE_INLM) {
                        rc = display_screen(LOOPMODE_TESTING_MENU);
                } else if (da.tm_input.advanced == ADVANCED_TRUE) {
                        rc = display_screen(ADVANCED_TESTING_MENU);
                } else {
                        rc = display_screen(CUSTOMER_TESTING_MENU);
                }
        }


        if (da.tm_input.dmode == DMODE_ELA) {
                do_ela();
        } else {
                rc = test_device();
                if ((da.tm_input.dmode == DMODE_PD) &&
                    ((da.tu_rc == BLOOM_DEVICE_BUSY)||(rc == BLOOM_SUCCESS))) {
                        do_ela();
                }
        }

        clean_up();

} /* end main() */

/*
 *
 * NAME: display_screen()
 *
 * FUNCTION: Display the proper "Stand By" screen for the test mode.
 *
 * NOTES: - The generic catalog messages for the three msg_type's are in the
 *          dcda.cat message file.
 *        - If -1 is returned, this function will set DA_ERROR_OTHER.
 *
 * RETURNS: rc from diag_display_menu(), or -1 on error.
 *
 */

int display_screen(int msg_type) {
        int menu_number = 0;
        int rc = -1;
        char *sub[3];

        if (da.tm_input.console == CONSOLE_FALSE) {
                return(DIAG_ASL_OK);
        }

        menu_number = da.ffc * 0x1000;
        sub[0] = (char *)diag_cat_gets(da.catd, PCISCSI, PCI_SCSI_ADAPTER);
        sub[1] = da.tm_input.dname;
        sub[2] = da.tm_input.dnameloc;

        switch(msg_type) {
        case CUSTOMER_TESTING_MENU:
                menu_number += 1;
                break;
        case ADVANCED_TESTING_MENU:
                menu_number += 2;
                break;
        case LOOPMODE_TESTING_MENU:
                menu_number += 3;
                break;
        default:
                break;
        }

        rc = diag_display_menu(msg_type, menu_number, sub,
                                da.tm_input.lcount, da.tm_input.lerrors);
        sleep(1); /* Leave screen up at least 1 second. */

        switch(rc) {
        case DIAG_ASL_OK:
        case DIAG_ASL_ENTER:
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up();
                break;
        case -1:
        default:
		dt(DT_ID,DT_DEC,"ERROR: ASL rc", rc);
                break;
        }

        return(rc);

} /* end display_screen() */

/*
 *
 * NAME: test_device()
 *
 * FUNCTION: Call each of the BLOOMER TU's. After each TU, a call
 *           is made to the check_XXXX_TU_rc() routines.
 *
 * NOTES: - If BLOOM_DEVICE_BUSY is returned, this function will
 *          set DA_ERROR_OPEN.
 *
 * RETURNS: Return code from Test Unit or BLOOM_DEVICE_BUSY.
 *
 */

int test_device(void) {
        int rc = -1;

        strcpy(da.tucb.name, da.tm_input.dname);
        da.tucb.tu = BLOOM_INIT_ATU;
        da.tu_rc = exectu(&da.tucb);

        /* Check for CANCEL/EXIT. */
        if (check_kb()) {
                clean_up();
        }

        da.srn_rcode = check_INIT_TU_rc(da.tu_rc);

        if (da.srn_rcode == BLOOM_DEVICE_BUSY) {
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                return(da.tu_rc);
        } else if (da.srn_rcode != BLOOM_SUCCESS) {
                rc = callout_srn(da.srn_rcode);
                if (rc == -1) {
                        clean_up();
                }
		/* Init TU failed, test parent device. */
                DA_SETRC_MORE(DA_MORE_CONT);
                return(da.tu_rc);
        }

	if (da.flags & DETECT_DEV_TYPE) {
		/* Call the detect device type TU. */
        	da.tucb.tu = BLOOM_SEDIFF_ATU;
        	da.tu_rc = exectu(&da.tucb);
        	/* Check for CANCEL/EXIT. */
        	if (check_kb()) {
                	clean_up();
        	}
		if (da.tu_rc == BLOOM_DEVICE_SE) {
			da.ffc = FFC_SE;
		} else if (da.tu_rc == BLOOM_DEVICE_DIFF) {
			da.ffc = FFC_DIFF;
		} else {
			dt(DT_ID,DT_HEX,"ERROR: Device type rc",da.tu_rc);
			da.flags |= DETECT_DEV_ERROR;
		}
	}

        da.tucb.tu = BLOOM_DIAG_ATU;
        da.tu_rc = exectu(&da.tucb);

        /* Check for CANCEL/EXIT. */
        if (check_kb()) {
                clean_up();
        }

        da.srn_rcode = check_DIAG_TU_rc(da.tu_rc);
        if (da.srn_rcode != BLOOM_SUCCESS) {
                rc = callout_srn(da.srn_rcode);
                if (rc == -1) {
                        clean_up();
                }
                return(da.tu_rc);
        }

        da.tucb.tu = BLOOM_SCSI_ATU;
        da.tu_rc = exectu(&da.tucb);

        /* Check for CANCEL/EXIT. */
        if (check_kb()) {
                clean_up();
        }

        da.srn_rcode = check_SCSI_TU_rc(da.tu_rc);
        if (da.srn_rcode != BLOOM_SUCCESS) {
                rc = callout_srn(da.srn_rcode);
                if (rc == -1) {
                        clean_up();
                }
        }

        return(da.tu_rc);

} /* end test_device() */

/*
 *
 * NAME: check_INIT_TU_rc()
 *
 * FUNCTION: Check the INIT TU RC to determine if an SRN and or menugoal
 *           is required.
 *
 * NOTES: The DEVICE OPEN init_flag will be set if the TU returns
 *        good status. This will indicate that the TERM TU is required
 *        in clean_up() after the DA is finished testing.
 *
 * RETURNS: BLOOM_SUCCESS, SRN reason code for failing tu_rci, or SW_ERROR.
 *
 */

int check_INIT_TU_rc(int tu_rc) {
        int rc;

        switch (tu_rc) {
        case BLOOM_SUCCESS:
                da.flags |= DA_DEV_OPEN;
                rc = BLOOM_SUCCESS;
                break;
        case BLOOM_CFGOPEN_E:
                rc = 0x301;
                break;
        case BLOOM_CHILD_E:
        case BLOOM_DEVICE_E:
                rc = BLOOM_DEVICE_BUSY;
                break;
        case BLOOM_PCFGRD_E:
        case BLOOM_PCFGWR_E:
                rc = 0x224;
                break;
        case BLOOM_DIAGNOSE_E:
        default:
                rc = SW_ERROR;
                break;
        }

        return(rc);

} /* end check_INIT_TU_rc() */

/*
 *
 * NAME: check_DIAG_TU_rc()
 *
 * FUNCTION: Check the DIAG TU RC to determine if an SRN and or menugoal
 *           is required.
 *
 * NOTES:
 *
 * RETURNS: BLOOM_SUCCESS, SRN reason code for failing tu_rci, or SW_ERROR.
 *
 */

int check_DIAG_TU_rc(int tu_rc) {
        int rc;

        switch (tu_rc) {
        case BLOOM_SUCCESS:
                rc = BLOOM_SUCCESS;
                break;
        case BLOOM_FIFOES_E:
                rc = 0x212;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_FIFOEC_E:
                rc = 0x213;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_FIFOFS_E:
                rc = 0x214;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_FIFOFC_E:
                rc = 0x215;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_FIFOMIS_E:
                rc = 0x216;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSIFIFO_MIS_E:
                rc = 0x217;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSIFIFO_UNDF_E:
                rc = 0x218;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSIFIFO_PAR_E:
                rc = 0x219;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSIFIFO_FLAGS_E:
                rc = 0x220;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_CFGMIS_E:
                rc = 0x221;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_IOREGS_E:
                rc = 0x222;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_MEMIOMIS_E:
                rc = 0x223;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_PCFGRD_E:
        case BLOOM_PCFGWR_E:
                rc = 0x224;
                break;
        default:
                rc = SW_ERROR;
                break;
        }

        return(rc);

} /* end check_DIAG_TU_rc() */

/*
 *
 * NAME: check_SCSI_TU_rc()
 *
 * FUNCTION: Check the SCSI TU RC to determine if an SRN and or menugoal
 *           is required.
 *
 * NOTES:
 *
 * RETURNS: BLOOM_SUCCESS, SRN reason code for failing tu_rci, or SW_ERROR.
 *
 */

int check_SCSI_TU_rc(int tu_rc) {
        int rc;

        switch (tu_rc) {
        case BLOOM_SUCCESS:
                rc = BLOOM_SUCCESS;
                break;
        case BLOOM_SCSIARB_E:
                rc = 0x230;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSFCMP_E:
                rc = 0x231;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_SCSIDATA_E:
                rc = 0x232;
                display_menugoal( USE_SCSI_PROCEDURE );
                break;
        case BLOOM_TERMPOWER_E:
                rc = 0x240;
                display_menugoal( USE_PTC_PROCEDURE );
                break;
        case BLOOM_PCFGRD_E:
        case BLOOM_PCFGWR_E:
                rc = 0x224;
                break;
        default:
                rc = SW_ERROR;
                break;
        }

        return(rc);

} /* check_SCSI_TU_rc() */

/*
 *
 * NAME: check_TERM_TU_rc()
 *
 * FUNCTION: Check the TERM TU RC to determine if an SRN and or menugoal
 *           is required.
 *
 * NOTES: Displays ERROR_INITIAL_STATE menugoal for TERM_TU failures. 
 *
 * RETURNS: BLOOM_SUCCESS or SRN reason code for failing tu_rc.
 *
 */

int check_TERM_TU_rc(int tu_rc) {
        int rc;

        switch (tu_rc) {
        case BLOOM_SUCCESS:
                rc = BLOOM_SUCCESS;
                break;
        case BLOOM_PCFGRD_E:
        case BLOOM_PCFGWR_E:
                rc = 0x224;
                break;
        default:
                rc = BLOOM_SUCCESS;
                display_menugoal( ERROR_INITIAL_STATE );
                break;
        }

        return(rc);

} /* end check_TERM_TU_rc() */

/*
 *
 * NAME: do_ela()
 *
 * FUNCTION: Perform Error Log Analysis for the device.
 *
 * NOTES: Refer to the header file (pciscsi.h) for the MAX thresholds.
 *
 * RETURNS: Void
 *
 */

void do_ela(void) {

        int i, rc;
        int op_flag = INIT;
        int err_found = 0;
        int errnum;
        enum { EN_85,  EN_110, EN_115,  EN_127,
               EN_128, EN_160, EN_170,  ERRID_10, AH_STAT_PTC };
        int thresholds[AH_STAT_PTC + 1];
        char criteria[64];
        struct errdata err_data;

        for (i = EN_85; i <= AH_STAT_PTC; i++) {
                thresholds[i] = 0;
        }

        sprintf(criteria, "%s -N %s", da.tm_input.date, da.tm_input.dname);

        while ((da.srn_rcode == 0) &&
               ((rc = error_log_get(op_flag, criteria, &err_data)) > 0)) {

                err_found = 1;

                switch(err_data.err_id) {
                case ERRID_SCSI_ERR2: /* TEMP Adapter HW */
                        errnum = 0;
                        errnum |=
                           ((int)err_data.detail_data[ERR_NUM_OFFSET]) << 24;
                        errnum |=
                           ((int)err_data.detail_data[ERR_NUM_OFFSET+1]) << 16;
                        errnum |=
                           ((int)err_data.detail_data[ERR_NUM_OFFSET+2]) << 8;
                        errnum |=
                           ((int)err_data.detail_data[ERR_NUM_OFFSET+3]);

                        switch (errnum) {
                        case 0x6E:
                                if (++thresholds[EN_110] >= MAX_EN_110) {
                                        da.srn_rcode = 0x700;
                                }
                                break;
                        case 0x73:
                                if (++thresholds[EN_115] >= MAX_EN_115) {
                                        da.srn_rcode = 0x701;
                                }
                                break;
                        case 0x7F:
                                if (++thresholds[EN_127] >= MAX_EN_127) {
                                        da.srn_rcode = 0x702;
                                }
                                break;
                        case 0x80:
                                if (++thresholds[EN_128] >= MAX_EN_128) {
                                        da.srn_rcode = 0x703;
                                }
                                break;
                        case 0xA0:
                                if (++thresholds[EN_160] >= MAX_EN_160) {
                                        da.srn_rcode = 0x704;
                                }
                                break;
                        case 0xAA:
                                if (++thresholds[EN_170] >= MAX_EN_170) {
                                        da.srn_rcode = 0x705;
                                }
                                break;
                        default:
                                if (err_data.detail_data[AH_STAT_OFFSET] ==
                                    TERM_PWR_FAIL) {
                                        da.srn_rcode = 0x800;
                                        display_menugoal( USE_PTC_PROCEDURE );
                                }
                                break;
                        }
                        break;
                case ERRID_SCSI_ERR10: /* TEMP SCSI Bus HW */
                        if (++thresholds[ERRID_10] >= MAX_ERRID_10) {
                                da.srn_rcode = 0x802;
                        }
                        break;
                default:
                        break;
                } /* end switch */

                op_flag = SUBSEQ;

        } /* end while */

        error_log_get(TERMI, criteria, &err_data);

        if (da.srn_rcode > 0) {
                rc = callout_srn(da.srn_rcode);
                if (rc == -1) {
                        clean_up();
                }
        } else if ((da.srn_rcode == 0) && (err_found)) {
                display_menugoal( USE_ERROR_REPORT_SA );
        }

        return;

} /* end do_ela() */

/*
 *
 * NAME: callout_srn()
 *
 * FUNCTION: Callout an SRN by populating a FRU bucket struct.
 *
 * NOTES: The frub array contains one of each type of FRU buckets used. 
 *        The rinfo array holds the reason message and FRU bucket index
 *        for each SRN reason code. 
 *
 * RETURNS: Void
 *
 */

int callout_srn(int srn_rcode) {
        int index = 0;
        int rc;
        struct fru_bucket frub[] = {
        /* 0 */ { "",   FRUB1, 0,  0,  0, {
                        { 70, "", "",  0,       DA_NAME,    EXEMPT  },
                        { 30, "", "",  0,   PARENT_NAME,    EXEMPT  }}},
        /* 1 */ { "",   FRUB1, 0,  0,  0, {
                        { 70, "", "", CAB_TERM_DEV, NO_FRU_LOCATION, NONEXEMPT},
                        { 30, "", "",  0,       DA_NAME,    EXEMPT  }}},
        /* 2 */ { "",   FRUB1, 0,  0,  0, {
                        { 80, "", "", CAB_TERM_DEV, NO_FRU_LOCATION, NONEXEMPT},
                        { 20, "", "",  0,       DA_NAME,    EXEMPT  }}},
        /* 3 */ { "",   FRUB1, 0,  0,  0, {
                        { 40, "", "",  0,       DA_NAME, NONEXEMPT  },
                        { 40, "", "", CAB_TERM_DEV, NO_FRU_LOCATION, NONEXEMPT},
                        { 20, "", "",  SW,    NOT_IN_DB, EXEMPT  }}},
        /* 4 */ { "",   FRUB1, 0,  0,  0, {
                        { 50, "", "",  0,       DA_NAME, NONEXEMPT  },
                        { 50, "", "",  SW,    NOT_IN_DB, EXEMPT  }}},
        /* 5 */ { "",   FRUB1, 0,  0,  0, {
                        { 70, "", "",  0,       DA_NAME, NONEXEMPT  },
                        { 30, "", "",  SW,    NOT_IN_DB, EXEMPT  }}},
        /* 6 */ { "",   FRUB1, 0,  0,  0, {
                        { 90, "", "",  0,       DA_NAME, NONEXEMPT  },
                        { 10, "", "",  SW,    NOT_IN_DB, EXEMPT  }}},
        /* 7 */ { "",   FRUB1, 0,  0,  0, {
                { 100, "", "", SCSI_SUBSYSTEM, NO_FRU_LOCATION, NONEXEMPT  }}}
        };
        struct reason_info rinfo[] = {
                { 0x212, RMSG_212, 0 } , { 0x213, RMSG_213, 0 } ,
                { 0x214, RMSG_214, 0 } , { 0x215, RMSG_215, 0 } ,
                { 0x216, RMSG_216, 0 } , { 0x217, RMSG_217, 0 } ,
                { 0x218, RMSG_218, 0 } , { 0x219, RMSG_219, 0 } ,
                { 0x220, RMSG_220, 0 } , { 0x221, RMSG_221, 0 } , 
		{ 0x222, RMSG_222, 0 } , { 0x223, RMSG_223, 0 } , 
		{ 0x224, RMSG_224, 0 } , { 0x225, RMSG_225, 0 } ,
                { 0x230, RMSG_230, 1 } , { 0x231, RMSG_231, 1 } ,
                { 0x232, RMSG_232, 1 } , { 0x240, RMSG_240, 2 } ,
                { 0x301, RMSG_301, 1 } , { 0x700, RMSG_700, 4 } ,
                { 0x701, RMSG_700, 4 } , { 0x702, RMSG_700, 4 } ,
                { 0x703, RMSG_700, 4 } , { 0x704, RMSG_700, 5 } ,
                { 0x705, RMSG_700, 6 } , { 0x800, RMSG_800, 7 } ,
                { 0x802, RMSG_802, 7 } ,
                        /* End of struct -> */ { 0 , 0 , 0 }
        };

        /* Search the rinfo struct for this SRN reason code. */
        while (srn_rcode != rinfo[index].rcode) {
                if (rinfo[index].rcode == 0) {
			dt(DT_ID,DT_HEX,"ERROR: SRN rcode",srn_rcode);
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        return(-1);
                }
                ++index;
        }

        strcpy(frub[rinfo[index].frub_index].dname, da.tm_input.dname);
        frub[rinfo[index].frub_index].sn = da.ffc;
        frub[rinfo[index].frub_index].rcode = rinfo[index].rcode;
        frub[rinfo[index].frub_index].rmsg = rinfo[index].rmsg;

        rc = insert_frub(&da.tm_input, &frub[rinfo[index].frub_index]);
        if (rc) {
		dt(DT_ID,DT_MSG,"ERROR: Insert Frub");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                return(-1);
        }

        frub[rinfo[index].frub_index].sn = da.ffc;

        rc = addfrub(&frub[rinfo[index].frub_index]);
        if (rc) {
		dt(DT_ID,DT_MSG,"ERROR: Add Frub");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                return(-1);
        }

        DA_SETRC_STATUS(DA_STATUS_BAD);

        return(0);

} /* end callout_srn() */

/*
 *
 * NAME: display_menugoal()
 *
 * FUNCTION: Display a menugoal.
 *
 * NOTES:
 *
 * RETURNS: Void
 *
 */

void display_menugoal(int menugoal_msg) {
        int menu_number;
        char text_buffer[1024];
	char *sub[3];

	/* If NO CONSOLE, open catalog file. */
        if (!(da.flags & DA_CAT_OPEN) && 
	    (da.tm_input.console != CONSOLE_TRUE)) {
                da.catd = (nl_catd)diag_catopen(MF_DPCISCSI, 0);
                da.flags |= DA_CAT_OPEN;
	}

        menu_number = da.ffc * 0x1000;

        switch(menugoal_msg) {
        case USE_PTC_PROCEDURE:
                menu_number += 0x10;
                break;
        case USE_SCSI_PROCEDURE:
                menu_number += 0x11;
                break;
        case USE_ERROR_REPORT_SA:
                menu_number += 0x12;
                break;
        case ERROR_INITIAL_STATE:
                menu_number += 0x13;
        	sub[0] = da.tm_input.dname;
		diag_display_menu(DEVICE_INITIAL_STATE_FAILURE,
				  menu_number, sub, 0, 0); 
		return;
        default:
		dt(DT_ID,DT_MSG,"ERROR: Invalid menugoal.");
                DA_SETRC_ERROR(DA_ERROR_OTHER);
		return;
        }

        sprintf(text_buffer,
                (char *)diag_cat_gets(da.catd, PCISCSI, menugoal_msg),
                menu_number,
                da.tm_input.dname,
                da.tm_input.dnameloc);

        menugoal(text_buffer);

        return;

} /* end display_menugoal() */

/*
 *
 * NAME: clean_up()
 *
 * FUNCTION: Prepare to return control back to the Diag Controller.
 *           Call the TERM TU to release the device from the diag state.
 *           Close all files and free any resources allocated by the DA.
 *
 * NOTES:
 *
 * RETURNS: Exit back to Diag Controller.
 *
 */

void clean_up(void) {
        int rc;

        if (da.flags & DA_DEV_OPEN) {
		/* Call the TERM (close) TU. */
                da.flags ^= DA_DEV_OPEN;
                da.tucb.tu = BLOOM_TERM_ATU;
                da.tu_rc = exectu(&da.tucb);
                da.srn_rcode = check_TERM_TU_rc(da.tu_rc);
                if ((da.srn_rcode != BLOOM_SUCCESS) && !(check_kb())) {
                        rc = callout_srn(da.srn_rcode);
                }
        }
        if (da.flags & DA_ASL_INIT) {
                diag_asl_quit();
        }
        if (da.flags & DA_CAT_OPEN) {
                catclose(da.catd);
        }
        if (da.flags & DA_ODM_INIT) {
                term_dgodm();
        }

        /* Exit back to the Diag Controller. */
        DA_EXIT();

} /* end clean_up() */

/*
 *
 * NAME: check_kb()
 *
 * FUNCTION: Check the keyboard input for CANCEL or EXIT.
 *
 * NOTES:
 *
 * RETURNS: Non-zero if user pressed CANCEL or EXIT, 0 otherwise.
 *
 */

int check_kb(void) {
        int kb_rc;

        if (da.tm_input.console == CONSOLE_FALSE) {
                return(0);
        }

        kb_rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,FALSE,NULL);

        if (kb_rc == DIAG_ASL_CANCEL) {
                DA_SETRC_USER(DA_USER_QUIT);
                return(3);
        } else if (kb_rc == DIAG_ASL_EXIT) {
                DA_SETRC_USER(DA_USER_EXIT);
                return(10);
        }

        return(0);

} /* end check_kb() */

/*
 *
 * NAME: sig_handler()
 *
 * FUNCTION: Call clean_up() when a signal is received.
 *
 * NOTES:
 *
 * RETURNS: Void
 *
 */

void sig_handler(int sig) {

        clean_up();

} /* end sig_handler() */
