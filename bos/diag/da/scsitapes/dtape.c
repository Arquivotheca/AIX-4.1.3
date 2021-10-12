static char sccsid[] = "@(#)98  1.22.2.15  src/bos/diag/da/scsitapes/dtape.c, datape, bos412, 9445C412a 11/9/94 09:13:46";
/*
 * COMPONENT_NAME: datape
 *
 * FUNCTIONS: main()
 *              get_dev_type()       disp_mng()
 *              display_first_msg()  clean_up()
 *              chk_ela,             int_handler()
 *              exit_with_frub()     ela_3490()
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */
#include "dtape.h"

/* Function Prototypes. */
int     get_dev_type(void);
void    display_first_msg(void);
void    chk_ela(void);
int     ela_3490(void);
int     build_3490_msg(struct errdata);
void    exit_with_frub(int);
void    disp_mng(int,int);
void    clean_up(int);
void    int_handler(int);
int     de_adapter(char *);

void db(int, ...);
void p_data(FILE *, char *, int);

extern int disp_menu(int);
extern void chk_kb_buff(void);

/* Global Variables */
DA_DATA		da;
struct CuDv     *cudv;		/* ODM data struct */
struct listinfo	obj_info;
struct tm_input tm_input;	/* Test Mode Input structure */
int		fdes = -1;	/* Device file descriptor */
uchar		lun_id;		/* Device Logical Unit ID */
uchar		scsi_id;	/* Device SCSI ID */
int		step = 1;	/* 1 based test step counter.  */

/* Catalog file descriptor and diag catalog routines. */
nl_catd catd;
nl_catd diag_catopen();

/* Config state flags */
int    cfgpar_state = -1; /* Config state of parent (scsi adapter). */
int    cfgdev_state = -1; /* Config state of device.                */

/* Initialization flags used in clean_up(). */
int        asl_flag  =  0; /* 1 if asl initialized.                  */
int       catd_flag  =  0; /* 1 if catalog file open.                */
int        odm_flag  =  0; /* 1 if odm initialized                   */
int      ttape_flag  = NO; /* NO if no tape (init),                  */
	                   /* YES catalog value if tape avail.,      */
	                   /* USERS_TAPE if users tape in drive.     */
int       load_flag  = ENABLE;
int       exit_flag  = EXIT_NTF; /* Exit status flag. */

char  test_sequence[128];     /* Array to hold TU sequence string.   */
int       test_seq = ST1;     /* Type of test sequence indicator.    */
	                      /* ST1, ST2_NO_WP or ST2_WP.           */
/*
 *
 * NAME: main()
 *
 * FUNCTION: Diagnostics for SCSI Tape Drives.
 *
 * NOTES:
 *        -  Init ASL and Diag ODM
 *        - Open catalog file
 *        - Run Diag tests.
 *        - Perform Error Log Analysis (ELA).
 *
 * RETURNS: DA_EXIT() to diag controller,
 *          DA_ERROR_OTHER (software error)
 *          DA_ERROR_OPEN  (device is busy)
 *          See the Diag CAS for more info.
 */

main(argc, argv, envp)
	int             argc;   /* argument list count           */
	char          **argv;   /* argument list vector          */
	char          **envp;   /* environment list vector       */
{
	long asl_rc;
	int     rc;             /* return code                   */
	int     fdes = -1;      /* file descriptor of device     */
	struct  sigaction act;
	int ipl_flag;

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

	/* Initialize Diag ODM. */
	if (init_dgodm() == -1) {
	        clean_up(EXIT_SW_ERR);
	}
	odm_flag = 1;

	/* Get our test mode input parameters. */
	if (getdainput(&tm_input) == -1) {
	        clean_up(EXIT_SW_ERR);
	}
	if(SYSX && EXITLM) {
	        clrdainput();
		clean_up(EXIT_NTF);  /* DOES NOT RETURN */
	}

db(1);

	if (CONSOLE) {
	        /* Initialize ASL with type ahead param. */
	        if (INLM || SYSTEM) {
	                /* Allow type ahead to catch Cancel or Exit. */
	                rc = diag_asl_init("DEFAULT");
	        } else {
	                /* Dont allow any type ahead. */
	                rc = diag_asl_init("NO_TYPE_AHEAD");
	        }
	        if (rc == -1) {
	                /* ASL init error. */
	                clean_up(EXIT_SW_ERR);
	        }
	        asl_flag = 1;
	        /* Open catalog file for messages. */
	        catd = diag_catopen("datape.cat",0);
	        catd_flag = 1;
	}

	/* Get device type (dev struct array index) from the LED value. */
	if ((da.dev_type = get_dev_type()) == -1)
		clean_up(EXIT_SW_ERR);

db(10,1,"dev_type", da.dev_type);

	if (da.dev_type == T3490) {
	        ipl_mode(&ipl_flag);
	        if (ipl_flag != DIAG_TRUE) {
	                /*
	                 * Call the 3490 routine. This routine covers all
	                 * the support required for the 3490 tape drive.
	                 */
	                rc = ela_3490();
	                clean_up(rc);
	        } else {
	                /*
	                 * Display a menu goal to inform the user that
	                 * PD must be run with the error log available.
	                 */
	                disp_mng(MNG_3490_IPL_N,CLEAN_UP);
	        }
	}

	display_first_msg();

	/* Do the TU tests */
	if (!ELA) {
	        do_tests();
	}

	/* Check for CANCEL or EXIT */
	chk_kb_buff();

	/* Check if ELA needed. */
	if ((exit_flag == EXIT_NTF) && PD && (NOTLM || ENTERLM) ) {
	        chk_ela();
	}

	clean_up(EXIT_NTF);  /* DOES NOT RETURN */

	return (0);

} /* end main */
/*
 *
 * NAME: get_dev_type()
 *
 * FUNCTION: Determine the device type by its LED value in ODM database.
 *
 * NOTES:
 *         -  Set dev_type (array index) to a supported drive type
 *            or exit with a menu goal.
 *         -  Search CuVPD for known OST drives (4mm is the only one).
 *
 * RETURNS: dev_type or software error.
 *
 */

int get_dev_type(void)
{
	int     dev_type = 0;
	int     rc;
	struct  CuVPD *cuvpd;
	struct  listinfo cuvpd_info;
	char    criteria[512];
	extern	int diag_get_sid_lun(char *, uchar *, uchar *);

	/* Figure out what device we are testing. Get the FFC code */
	/*  from the LED value in the pre-defined object class.    */

	sprintf(criteria, "name = %s", tm_input.dname);
	cudv = get_CuDv_list(CuDv_CLASS, criteria, &obj_info, 1, 2);
	if ((cudv == (struct CuDv *)(-1)) ||
	    (cudv == (struct CuDv *)NULL) ||
	    (obj_info.num != 1)) {
	        /* Error getting device info. */
	        clean_up(EXIT_SW_ERR);
	}

	/* Get the device LED and LUN ID from CuDv. */
	if (diag_get_sid_lun(cudv->connwhere, &scsi_id, &lun_id) == -1)
		return (-1);

db(16,1,"Device LED", cudv->PdDvLn->led);

	switch(cudv->PdDvLn->led) {
	case 0x899:
	        dev_type = T3490;
	        break;
	case 0x970:
	        dev_type = T9348;     /* LED 970 1/2" Model 9348 */
	        break;
	case 0x971:
	        dev_type = T3600;     /* LED 971 1/4" 150MB */
	        break;
	case 0x972:
	        dev_type = T8200;     /* LED 972 8mm 2.3GB */
	        break;
	case 0x991:
	        dev_type = T3800;     /* LED 991 1/4" 525MB */
	        break;
	case 0x994:
	        /* Check to see if the device is an SE or DE device. */
	        /* The FFC value is different for SE and DE devices. */
	        if (!de_adapter(tm_input.parent) == DIAG_TRUE)
	                dev_type = T8505_SE;  /* LED 994 8mm 5GB SE */
	        else
	                dev_type = T8505_DE;  /* LED 914 8mm 5GB DE */
	        break;
	case 0x914:
	        dev_type = T8505_DE;  /* LED 914 8mm 5GB DE */
	        break;
	case 0x995:
	        dev_type = T4100;     /* LED 995 1/4" 1.2GB */
	        break;
	case 0x973:
	       dev_type = OST;        /* LED 973 (Other SCSI Tape) */
	       break;
	case 0x998:
	        dev_type = T4MM;      /* LED 998 4mm  */
	        break;
	case 0x915:
	        dev_type = T4MM4GB;   /* LED 915 4mm4gb  */
	        break;
	case 0x733:
	        dev_type = T8505XL;   /* LED 733 7 GB 8 mm  */
	        break;
	default:
	        /* Display "Device not supported..." Menu Goal. */
	        disp_mng(MNG_NOT_SUPPORTED,CLEAN_UP);
	        break;
	}

	if (dev_type == OST) {
	        /* If dev_type OST then check for 3490. */
	        /* This allows 3490 support as an OST.  */
	        sprintf(criteria,
	                CuVPD_3490_CRITERIA,
	                tm_input.dname);
	        cuvpd = get_CuVPD_list(CuVPD_CLASS,
	                               criteria, &cuvpd_info, 1, 2);
	        if ((cuvpd != (struct CuVPD *)(-1)) &&
	            (cuvpd != (struct CuVPD *) NULL)) {
	                dev_type = T3490;
	        }

	        /* If dev_type still OST then check for 4mm. */
	        if (dev_type == OST) {
	                /* Check VPD for 4mm drive type. */
	                sprintf(criteria,
	                        CuVPD_4mm_CRITERIA,
	                        tm_input.dname);
	                cuvpd = get_CuVPD_list(CuVPD_CLASS,
	                                       criteria, &cuvpd_info, 1, 2);
	                if ((cuvpd != (struct CuVPD *)(-1)) &&
	                    (cuvpd != (struct CuVPD *) NULL))
	                        dev_type = T4MM;
	        }

	        /* If dev_type still OST then check for 3800. */
	        /* This removes the coreq with diags and DD for 9/92. */
	        if (dev_type == OST) {
	                /* Check VPD for 525MB drive type as OST. */
	                sprintf(criteria,
	                        CuVPD_525MB_CRITERIA,
	                        tm_input.dname);
	                cuvpd = get_CuVPD_list(CuVPD_CLASS,
	                                       criteria, &cuvpd_info, 1, 2);
	                if ((cuvpd != (struct CuVPD *)(-1)) &&
	                    (cuvpd != (struct CuVPD *) NULL))
	                        dev_type = T3800;
	        }
	}

	return(dev_type);

} /* end get_device_type */
/*
 * NAME: display_first_msg()
 *
 * FUNCTION: Determine the test mode, if interactive mode, display first
 *           message(s) before any testing begins.
 *
 * NOTES: The test tape flag and Test Unit sequence string is set in this
 *        function.
 *
 * RETURNS: void
 *
 */

void display_first_msg(void)
{
	int rc;

	if ( INTERACTIVE ) {
	        /* Display Test Tape Requirements Menu. */
	        if (SYSX) {
	                if (da.dev_type != T4MM && da.dev_type != T4MM4GB &&
			tm_input.advanced == ADVANCED_TRUE) {
	                        ttape_flag = disp_menu(MSG_SYSX_REQS);
	                } else {
	                        ttape_flag = NO;
	                }
	        } else {
	                ttape_flag = disp_menu(MSG_TEST_REQS);
	        }

	        if (ttape_flag == NO) {
	                test_seq = ST1;
	                rc = disp_menu(REMOVE_TAPE);
	        } else {
	                if (NOTLM) {
	                        test_seq = ST2_WP;
	                } else {
	                        test_seq = ST2_NO_WP;
	                }
	        }
	        if (SYSX) {
                        diag_asl_quit();
                        asl_flag = 0;
                        clrdainput();
                }
	} else {
	        if (INLM || EXITLM) {
	                getdavar(tm_input.dname, "test_seq",
	                         DIAG_INT, &test_seq);
	                getdavar(tm_input.dname, "ttape_flag",
	                         DIAG_INT, &ttape_flag);
	        } else {
	                test_seq = ST1;
	        }
	}

db(10,2,"test_seq",test_seq,"ttape_flag",ttape_flag);

	/* Display Stand By Message. */
	if (NOTLM || (ENTERLM && !SYSTEM))
	        rc = disp_menu(STAND_BY);
	else
	        rc = disp_menu(LOOPMODE);

	/* Get the device specific TU sequence string. */
	/* If NULL, get the default TU string.         */
	switch(test_seq) {
	case ST1:
	        strcpy(test_sequence, dev[da.dev_type].st1);
	        if (!strlen(test_sequence))
	                strcpy(test_sequence, SELF_TEST1);
	        break;
	case ST2_NO_WP:
	        strcpy(test_sequence, dev[da.dev_type].st2_no_wp);
	        if (!strlen(test_sequence))
	                strcpy(test_sequence, SELF_TEST2_NO_WP);
	        break;
	case ST2_WP:
	        strcpy(test_sequence, dev[da.dev_type].st2_wp);
	        if (!strlen(test_sequence))
	                strcpy(test_sequence, SELF_TEST2_WP);
	        break;
	}

db(20,1,"test_sequence",test_sequence);

	return;

} /* end display_first_msg */
/*
 * NAME: chk_ela()
 *
 * FUNCTION: Perform Error Log Analysis
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */
void chk_ela(void)
{
	int     rc;
	int     error_found = 0;
	int     recovered_errs = 0;
	int     max_temp_errs = 1;
	struct  errdata err_data;
	int     op_flg;
	char srch_crit[128];   /* ELA search criteria. */

	sprintf( srch_crit, "%s -N %s", tm_input.date, tm_input.dname);
	op_flg = INIT;
	do {
	        rc = error_log_get( op_flg, srch_crit, &err_data );
	        if( rc > 0 ) {
	                switch( err_data.err_id ) {
	                case ERRID_TAPE_ERR1:  /* unrecoverable media error */
	                case ERRID_TAPE_ERR3:  /* recovered error threshold */
	                        ++recovered_errs;
	                        op_flg = SUBSEQ;
	                        break;
	                case ERRID_TAPE_ERR2:  /* hardware error  */
	                        error_found = 1;
	                        op_flg = TERMI;
	                        break;
	                default:
	                        op_flg = SUBSEQ;
	                        break;
	                }
	        } else {
	                op_flg = TERMI;
	        }
	} while( op_flg != TERMI );

	/* Close error log (op_flag == TERMI). */
	error_log_get( op_flg, srch_crit, &err_data);

	if( error_found ) {
	        /* Report ELA SRN. */
	        exit_with_frub(DTAPE_ELA_SRN);
	}

	if( (exit_flag == EXIT_NTF) &&
	    (recovered_errs >= max_temp_errs) ) {
	        /* Display ELA menu goal. */
	        disp_mng(MNG_ELA_MEDIA,CLEAN_UP);
	}

	return;

} /* end chk_ela */
/* *** FUNCTION PROLOG ***
 *
 * NAME:  ela_3490()
 *
 * DESCRIPTION:      Find error log entries for the 3490 1/2" tape
 *              drive. Display 32 bytes of sense data for each entry.
 *
 * INPUT PARAMETERS:  NONE
 *
 * CALLING FUNCTION(S):  get_dev_type() in the dtape.c module.
 *
 * FUNCTION(S) CALLED:  build_3490_msg()
 *                      disp_mn(), error_log_get()
 *
 * DATA STRUCTURES: struct errdata errlog_data
 *
 * GLOBAL VARIABLES AFFECTED: char msg_3490_buff[]
 *
 * ERROR RECOVERY: Return EXIT_SW_ERR for software errors.
 *
 * RETURNS:  EXIT_NTF or EXIT_SW_ERR
 *
 * NOTES:  - Refer to dtape.h for 3490 defines etc.
 *         -
 */

int ela_3490(void) {

	struct errdata errlog_data;  /* Populated by error_log_get func. */
	char criteria[NAMESIZE+1];   /* Error log search criteria.       */
	int errlog_rc = 0;           /* Used for error_log_get call.     */
	int mng_flag = 0;            /* True after nenu goal displayed.  */
	int num_entries = 0;         /* Error log entry counter.         */
	int rc = 0;                  /* Misc function return code.       */

	db(25,"In ela_3490()");
	/* Clear the message buffer for a new message. */
	memset(msg_3490_buff, 0, BUFF_SIZE_3490);

	/* Display the "Stand By" message. */
	if (NOTLM || (ENTERLM && !SYSTEM))
	        rc = disp_menu(STAND_BY);
	else
	        rc = disp_menu(LOOPMODE);

	sleep(1); /* Allow user time to see the "Stand by" message. */

	/* Search the system error log for 3490 entries. */
	sprintf(criteria," -N %s",tm_input.dname); /* No date used. */
	errlog_rc = error_log_get(INIT, criteria, &errlog_data);
	if (errlog_rc < 0) {
	        rc = error_log_get(TERMI, criteria, &errlog_data);
	        return(EXIT_SW_ERR);
	}

	if (errlog_rc) {
	        if ( INTERACTIVE && PD && !ENTERLM ) {
	                while(errlog_rc || num_entries) {
	                        ++num_entries;
	                        if (errlog_rc) {
	                                rc = build_3490_msg(errlog_data);
	                                if (rc == DA_ERROR_OTHER) {
	                                        error_log_get(TERMI, criteria,
	                                                      &errlog_data);
	                                        return(EXIT_SW_ERR);
	                                }
	                        }
	                        if ( (!errlog_rc && num_entries) ||
	                             (num_entries == MAX_3490_ENTRIES) ) {
	                                /* Display the sense data. */
	                                rc = disp_menu(SENSE_3490_N);
	                                if ( (rc == DIAG_ASL_CANCEL) ||
	                                     (rc == DIAG_ASL_EXIT) ) {
	                                        break;
	                                }
	                                /* Clear the message buffer */
	                                /* for a new message.       */
	                                memset(msg_3490_buff, 0,
	                                       BUFF_SIZE_3490);
	                                num_entries = 0;
	                        }
	                        errlog_rc = error_log_get(SUBSEQ,
	                                                  criteria,
	                                                  &errlog_data);
	                        if (errlog_rc < 0) {
	                                error_log_get(TERMI, criteria,
	                                              &errlog_data);
	                                return(EXIT_SW_ERR);
	                        }
	                } /* end while */
	        } else if (CONSOLE) {
	                /* If the mng flag is zero, populate a  */
	                /* menu goal and set the mng flag to 1. */
	                if (INLM || EXITLM) {
	                        rc = getdavar(tm_input.dname, "mng_flag",
	                                      DIAG_INT, &mng_flag);
	                }
	                if (!mng_flag) {
	                        disp_mng(MNG_3490_RUN_PD_N, RETURN);
	                        mng_flag = 1;
	                        if (ENTERLM || INLM) {
	                                rc = putdavar(tm_input.dname,
	                                              "mng_flag", DIAG_INT,
	                                              &mng_flag);
	                        }
	                }
	                if (SYSX) {
	                        diag_asl_quit();
	                        asl_flag = 0;
	                        clrdainput();
	                }
	        } /* CONSOLE */
	} /* rc */

	rc = error_log_get(TERMI, criteria, &errlog_data);

	return(EXIT_NTF);

} /* end ela_3490 */
/* *** FUNCTION PROLOG ***
 *
 * NAME:  build_3490_msg()
 *
 * DESCRIPTION:      Build a message buffer to hold the error log entry
 *              info. Uses the SENSE_3490_N catalog message to add the error
 *              label, date, time and 32 bytes of extended sense data
 *              from the error log detail data.
 *
 * INPUT PARAMETERS:  struct errdata errlog_data
 *
 * CALLING FUNCTION(S):  ela_3490()
 *
 * FUNCTION(S) CALLED:  diag_cat_gets(), malloc()
 *
 * DATA STRUCTURES:  struct errdata errlog_data
 *
 * GLOBAL VARIABLES AFFECTED:  char msg_3490_buff[]
 *
 * ERROR RECOVERY:  Return DA_ERROR_OTHER for software errors.
 *
 * RETURNS:  DA_STATUS_GOOD or DA_ERROR_OTHER
 *
 * NOTES:  - Coded to format 32 bytes of sense data into 2 lines of text
 *          (see SENSE_3490_N in datape.msg).
 */

int build_3490_msg(struct errdata errlog_data) {
	int i, offset;
	int id = 0;
	char *msg_ptr;
	char *temp_buff;
	char *timestamp;
	char sense_byte[4] = {'\0'};
	char sense_line1[64] = {'\0'};
	char sense_line2[64] = {'\0'};
	char sense_line3[64] = {'\0'};
	char sense_line4[64] = {'\0'};

	db(25,"In build_3490_msg()");
	db(16,1,"ERROR ID",errlog_data.err_id);
	msg_ptr = (char *)diag_cat_gets(catd, DTAPE_SET, SENSE_3490_N);

	/* Use err_id to fill in the "X" in the */
	/* error label (TAPE_ERRX).             */
	switch(errlog_data.err_id) {
	case ERRID_TAPE_ERR1:
	        id = 1;
	        break;
	case ERRID_TAPE_ERR2:
	        id = 2;
	        break;
	case ERRID_TAPE_ERR3:
	        id = 3;
	        break;
	case ERRID_TAPE_ERR4:
	        id = 4;
	        break;
	case ERRID_TAPE_ERR5:
	        id = 5;
	        break;
	default:
	        /* Leave ID at zero. */
	        break;
	}

	/* Get the timestamp from the errlog struct. */
	timestamp = (char *)ctime(&errlog_data.time_stamp);

	/* Build the two lines of sense data. */
	if (errlog_data.detail_data_len >=
	    (EL_SENSE_OFFSET + MAX_3490_SENSE_BYTES)) {
	        /* Format first line of 3490 sense data (16 bytes). */
	        offset = EL_3490_OFFSET;
	        for (i = 0; i < 16; i++) {
	                sprintf(sense_byte, "%02.2X ",
	                        errlog_data.detail_data[offset++]);
	                strcat(sense_line1, sense_byte);
	                if (i == 7)
	                        strcat(sense_line1, " ");
	        }
	        /* Format second line of 3490 sense data (16 bytes). */
	        for (i = 0; i < 16; i++) {
	                sprintf(sense_byte, "%02.2X ",
	                        errlog_data.detail_data[offset++]);
	                strcat(sense_line2, sense_byte);
	                if (i == 7)
	                        strcat(sense_line2, " ");
	        }
	        /* Format first line of SCSI sense data (16 bytes). */
	        offset = EL_SENSE_OFFSET;
	        for (i = 0; i < 16; i++) {
	                sprintf(sense_byte, "%02.2X ",
	                        errlog_data.detail_data[offset++]);
	                strcat(sense_line3, sense_byte);
	                if (i == 7)
	                        strcat(sense_line3, " ");
	        }
	        /* Format second line of SCSI sense data (6 bytes). */
	        for (i = 0; i < 6; i++) {
	                sprintf(sense_byte, "%02.2X ",
	                        errlog_data.detail_data[offset++]);
	                strcat(sense_line4, sense_byte);
	        }
	}

	temp_buff = (char *)malloc(BUFF_SIZE_3490);
	if (temp_buff == (char *)NULL) {
	        return(DA_ERROR_OTHER);
	} else {
	        sprintf(temp_buff, msg_ptr, id,
	                                    errlog_data.err_id,
	                                    timestamp,
	                                    sense_line1,
	                                    sense_line2,
	                                    sense_line3,
	                                    sense_line4);
	}

	strcat(msg_3490_buff, temp_buff);
	free(temp_buff);

	return(DA_STATUS_GOOD);

} /* end build_3490_msg */
/*
 * NAME: exit_with_frub()
 *
 * FUNCTION: Populate a FRU Buucket to be displayed by the Diag Controller.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void exit_with_frub(int frub_num)
{
	int     rc, conf = 0;
	int     index1 = 0, index2 = 0;
	int     rcode = 0, rcode_save = 0;
	int     serv_hints = 0;

	/* Populate a FRU Bucket and clean up. */
	/* FRUB = Field Replaceable Unit, Bucket (struct) of data. */

	/* If CANCEL or EXIT, clean up. */
	chk_kb_buff();

	/* Map the failing TU number with to a reason code in the */
	/* da_frub struct. the rcode may be added for non check   */
	/* condition TU return codes.                             */
	switch(frub_num) {
	case SCATU_RESERVE_UNIT:
	        rcode = 0x110;
	        break;
	case SCATU_INQUIRY:
	        rcode = 0x120;
	        break;
	case SCATU_LOAD_UNLOAD_UNIT:
	case SCATU_MODE_SENSE:
	case LOAD:
	case UNLOAD:
	        if (load_flag == ENABLE)
	                rcode = 0x130; /* Before self tests. */
	        else
	                rcode = 0x180; /* After self tests. */
	        rcode += (frub_num == UNLOAD) ? 0x05 : 0;
	        break;
	case SCATU_MODE_SELECT:
	        if (load_flag == ENABLE)
	                rcode = 0x140; /* Before self tests. */
	        else
	                rcode = 0x190; /* After self tests. */
	        break;
	case SCATU_TEST_UNIT_READY:
	        if (load_flag == ENABLE)
	                rcode = 0x150; /* Before self tests. */
	        else

	                rcode = 0x200; /* After self tests. */
	        break;
	case TUR_4MM4GB:
	case TUR_4MM4GB1:
	        rcode = 0x200;
	        break;
	case SEND_DIAG_MEDIA_ERROR:
	        rcode = 0x169;
	        break;
	case SCATU_SEND_DIAGNOSTIC:
	case SCATU_RECEIVE_DIAGNOSTIC_RESULTS:
	case SDTU_ST1:
	case SDTU_ST2:
	        rcode = 0x160;
	        break;
	case DA_W_R_C_TEST:
	        rcode = 0x170;
	        break;
	case SCATU_WRITE:
	        rcode = 0x210;
	        break;
	case SCATU_RELEASE_UNIT:
	        rcode = 0x220;
	        break;
	case SCATU_REQUEST_SENSE:
	        rcode = 0x230;
	        break;
	case DTAPE_OPENX_FAILED:
		DA_SETRC_MORE(DA_MORE_CONT);
	        rcode = 0x240;
	        break;
	case DTAPE_CFG_DEVICE:
		DA_SETRC_MORE(DA_MORE_CONT);
	        rcode = 0x300;
	        break;
	case DTAPE_CFG_PARENT:
		DA_SETRC_MORE(DA_MORE_CONT);
	        rcode = 0x310;
	        break;
	case DTAPE_ELA_SRN:
	        rcode = 0x320;
	        break;
	default:
	        break;
	} /* end switch */

	if (!CHECK_CONDITION(da.tu_rc)) {
	        switch(da.tu_rc) {
	        case SCATU_RESERVATION_CONFLICT:
	                rcode_save = rcode;
	                rcode = 0x400;
	                break;
	        case SCATU_BAD_REQUEST_SENSE_DATA:
	        case SCATU_NONEXTENDED_SENSE:
	                rcode_save = rcode;
	                rcode = 0x500;
	                break;
	        case SCATU_COMMAND_ERROR:
	        case SCATU_IO_BUS_ERROR:
	        case SCATU_ADAPTER_FAILURE:
	                rcode_save = rcode;
	                rcode = 0x600;
	                break;
	        case SCATU_TIMEOUT:
	                rcode_save = rcode;
	                rcode = 0x700;
	                break;
	        case SCATU_BAD_PARAMETER:
	        default:
	                break;
	        }
	} /* end if */

	/* Search for rcode in the FRU Bucket array (da_frub). */
	while (da_frub[index1].rcode != rcode)  {
	        if (da_frub[index1].rcode == 0) { /* Reached end of array. */
db(10,1,"!!!! SW ERROR",1);
	                clean_up(EXIT_SW_ERR);
	        }
	        ++index1;
	}

	if (!CHECK_CONDITION(da.tu_rc))
	        rcode += (rcode_save / 0x10); /* Map TU # to rcode. */

	da_frub[index1].rcode = rcode;

	/* Fill in the information for each FFC.   */
	/* While our confidence is less than 100% (more FFC's), */
	while ((conf += da_frub[index1].frus[index2].conf) <= 100) {
	        /* fill in location info for this FFC, or */
	        switch (da_frub[index1].frus[index2].fru_flag) {
	        case DA_NAME:
	                strncpy(da_frub[index1].frus[index2].floc,
	                       tm_input.dnameloc, (LOCSIZE - 1));
	                break;
	        case PARENT_NAME:
	                ++serv_hints;
	                strncpy(da_frub[index1].frus[index2].floc,
	                       tm_input.parentloc, (LOCSIZE - 1));
	                break;
	        default:
	                break;
	        } /* end switch */
	        ++index2;
	} /* end while */

	/* Fill the the device name. */
	strcpy(da_frub[index1].dname,tm_input.dname);

	/* Assign correct FFC (led) value to the FRU Bucket. */
	da_frub[index1].sn = dev[da.dev_type].led;

	/* Insert the FRU Bucket, the FFC (led) may change. */
	rc = insert_frub(&tm_input, &da_frub[index1]);
	if (rc == -1) {
db(10,1,"!!!! SW ERROR",2);
	        clean_up(EXIT_SW_ERR);
	}

	/* Reassign correct FFC (led) to FRU Bucket. */
	da_frub[index1].sn = dev[da.dev_type].led;

	/* Add FRUB. */
	rc = addfrub(&da_frub[index1]);
	if (rc == -1) {
db(10,1,"!!!! SW ERROR",3);
	        clean_up(EXIT_SW_ERR);
	}

	DA_SETRC_STATUS(DA_STATUS_BAD);
	exit_flag = EXIT_SRN;

db(16,1," **** SRN",rcode);

	if (dev[da.dev_type].led == 0x973) /* OST */
	        disp_mng(MNG_UNKNOWN_DEVICE,CLEAN_UP);

	/* The following Menu Goal calls will return to this */
	/* function. This will allow multiple Menu Goals to  */
	/* be displayed to help the CE better understand the */
	/* problem with the tape drive. This data will also  */
	/* be returned from the field through the Product    */
	/* Topology Service Aid.                             */

	if (ADVANCED && CHECK_CONDITION(da.tu_rc))
	        disp_mng(MNG_DISP_SENSE_DATA,RETURN);

	clean_up(EXIT_SRN);

} /* end exit_with_frub */
/*
 * NAME: disp_mng()
 *
 * FUNCTION: Populate a FRU Buucket to be displayed by the Diag Controller
 *           to inform the user of additional recommended service procedures.
 *
 * NOTES:
 *
 * RETURNS: void
 *
 */

void disp_mng(int mng, int return_flag)
{
	char    *criteria;
	char    *sense_buff;
	char    sense_byte[16];
	char    *mng_ptr;
	long    mng_num = 0;
	int     rc, i, j, offset, size;
	extern int retry_flag;
	extern int step;
	extern int display_menu(int);

	/* Populate a Menu Goal and clean up. */

	/* If CANCEL or EXIT, clean up. */
	chk_kb_buff();

        /* Check for media error menugoal. */
        if ((mng == MNG_CLEAN_NEW_TTAPE) && (INTERACTIVE) && !(SYSX)) {
                /* Ask user questions about cleaning and test tape for */
                /* media errors. Return so tests can be run again.     */
                if ((rc = disp_menu(DID_YOU_CLEAN_DRIVE)) == YES) {
                        if ((rc = disp_menu(DID_YOU_TRY_NEW_TTAPE)) == YES) {
                                exit_with_frub(SEND_DIAG_MEDIA_ERROR);
                        } else {
                                rc = disp_menu(GET_NEW_TTAPE_NOW);
                        }
                } else {
                        rc = disp_menu(CLEAN_DRIVE_NOW);
                }
                step = retry_flag = 0;
                return;
        }

	/* Set up menu goal number. */
	mng_num = dev[da.dev_type].led * 0x1000;

	/* Add all the parts to determine allocation size. */
	size =  strlen((char *)diag_cat_gets(catd, DTAPE_SET, mng)) +
	        6 + NAMESIZE + LOCSIZE;

	/* Get the menu goal text. */
	mng_ptr = (char *)diag_cat_gets(catd, DTAPE_SET, mng);

	/* Set up menu goal substitutions. */
	switch(mng) {
	case MNG_DISP_SENSE_DATA:
	        return_flag = 1;
	        mng_num += 0x10;
	        /* Put the sense data in a displayable format.        */
	        /* Display 40 bytes, 8 bytes per line. Separated each */
	        /* byte with two spaces.                              */
	        /* Allocate and zero the buff. */
	        sense_buff = (char *)calloc(1,255);
	        offset = 0;

	        strcpy(sense_buff, "\t\t");
	        for (i = 0; i < 5; i++) {
	                /* 8 bytes, each separated by a space. */
	                for (j = 0; j < 8; j++) {
	                        sprintf(sense_byte, "%02.2X  ",
	                                da.sdata[offset++]);
	                        strcat(sense_buff, sense_byte);
	                }
	                strcat(sense_buff,"\n\t\t");
	        }
	        size += strlen(sense_buff) + 64;
	        criteria = (char *)malloc(size + 1);
	        sprintf(criteria, mng_ptr, mng_num, tm_input.dname,
	                tm_input.dnameloc, da.skey, da.scode, sense_buff);
	        break;
	/* Standard substitutions, add new ones here. */
	case MNG_SCSI_BUS_RESET:      /* Menu Goal XXX019 */
	        mng_num += 1; /* NO BREAK */
	case MNG_3490_IPL_N:            /* Menu Goal 899018 */
	        mng_num += 1; /* NO BREAK */
	case MNG_3490_RUN_PD_N:         /* Menu Goal 899017 */
	        mng_num += 1; /* NO BREAK */
	case MNG_UNKNOWN_DEVICE:       /* Menu Goal XXX016 */
	        mng_num += 1; /* NO BREAK */
	case MNG_TAPE_SERVICE_HINTS:   /* Menu Goal XXX015 */
	        mng_num += 1; /* NO BREAK */
	case MNG_ELA_MEDIA:            /* Menu goal XXX014 */
	        mng_num += 1; /* NO BREAK */
	case MNG_CLEAN_DRIVE:          /* Menu Goal XXX013 */
	        mng_num += 1; /* NO BREAK */
	case MNG_NOT_SUPPORTED:        /* Menu Goal XXX012 */
	        mng_num += 1; /* NO BREAK */
	case MNG_CLEAN_NEW_TTAPE:      /* Menu Goal XXX011 */
	        mng_num += 0x11;
	        criteria = (char *)malloc(size + 1);
	        sprintf(criteria, mng_ptr, mng_num, tm_input.dname,
	                tm_input.dnameloc);
	        break;
	default:
db(10,1,"!!!! SW ERROR",4);
	        clean_up(EXIT_SW_ERR);
	        break;
	} /* end switch */

	/* Populate the Menu Goal (DMenu) object class. */
	menugoal(criteria);

db(16,1," ** MNG",mng_num);

	/* This function returns to allow multiple Menu Goals. */
	if (return_flag)
	        return;

	clean_up(EXIT_MNG);

} /* end disp_mng */
/*
 * NAME: clean_up()
 *
 * FUNCTION: Perform what is necessary for an orderly exit from this DA.
 *
 * NOTES:
 *        - Will unload tape and release the reservation on the drive.
 *        - Unconfig the device if this DA configed it.
 *        - Close files, etc.
 *
 * RETURNS: Return to the Diag controller.
 *
 */

void clean_up(int status)
{
	int i, rc;

db(10,1,"In clean_up(), exit_flag",exit_flag);

	if (!asl_flag)
	        tm_input.console = CONSOLE_FALSE;

	/* Check for CANCEL or EXIT. */
	if (CONSOLE) {
	        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
	        if (rc == DIAG_ASL_CANCEL) {
	                DA_SETRC_USER(DA_USER_QUIT);
	                status = EXIT_NTF;
	        }
	        if(rc == DIAG_ASL_EXIT) {
	                DA_SETRC_USER(DA_USER_EXIT);
	                status = EXIT_NTF;
	        }
	}

	switch(status) {
	case EXIT_SW_ERR:
	        DA_SETRC_ERROR(DA_ERROR_OTHER);
	        break;
	case EXIT_ERR_OPEN:
	        DA_SETRC_ERROR(DA_ERROR_OPEN);
	        break;
	case EXIT_SRN:
	case EXIT_MNG:
	case EXIT_NTF:
	        exit_flag = status;
	        break;
	case EXIT_SUB:
	        DA_SETRC_TESTS(DA_TEST_SUB);
	        break;
	default:
	        break;
	}

	if (ENTERLM) {
	        /* Put away vars for INLM and EXITLM. */
	        putdavar(tm_input.dname, "ttape_flag",
	                 DIAG_INT, &ttape_flag);
	        putdavar(tm_input.dname, "test_seq",
	                 DIAG_INT, &test_seq);
	}

	if (!SYSTEM && (EXITLM || NOTLM) && (ttape_flag == YES)) {
		/* Check if diags booted from tape. */
		if (!strcmp((char *)getenv("DIAG_IPL_SOURCE"),"2") &&
		    !strcmp((char *)getenv("BOOTDEV"),tm_input.dname)) {
			/* Remove test tape, re-insert diag boot tape. */
	        	rc = disp_menu(DIAG_BOOT_COMPLETE); 
		} else {
	        	rc = disp_menu(COMPLETE); /* Remove test tape. */
		}
	}

	/* Release the device reservation before close. */
	/* Get the RELEASE UNIT TU data.                */
	if (fdes != -1) {
	        i = 0;
	        while (tus[i].tu_num != SCATU_RELEASE_UNIT) {
	                if (tus[i++].tu_num == 0) {
db(10,1,"!!!! SW ERROR",5);
	                        DA_SETRC_ERROR(DA_ERROR_OTHER);
	                }
	        }
	        da.tu = &tus[i];

	        rc = tu_test();
	        if (rc != SCATU_GOOD)
	                rc = tu_test(); /* Retry */
	}

	if (fdes > -1)  /* Close the device file descriptor. */
	        close(fdes);

	if (cfgdev_state != -1) {
	        /* Return device to initial config state. */
	        rc = initial_state(cfgdev_state, tm_input.dname);
	        if ( (rc == -1) && (exit_flag == EXIT_NTF) )
	                DA_SETRC_ERROR(DA_ERROR_OTHER);
	}

	if (cfgpar_state != -1) {
	        /* Return parent to initial config state. */
	        rc = initial_state(cfgpar_state, tm_input.parent);
	        if ( (rc == -1) && (exit_flag == EXIT_NTF) )
	                DA_SETRC_ERROR(DA_ERROR_OTHER);
	}

	if (catd_flag)  /* Close catalog file. */
	        catclose(catd);

	if (asl_flag)   /* Quit ASL. */
	        diag_asl_quit();

	if (cudv)
		odm_free_list(cudv, &obj_info);

	if (odm_flag) /* Terminate Diag ODM. */
	        term_dgodm();

db(999);

	DA_EXIT();  /* Exit the DA and return to the Diagnostic Controller. */

} /* end clean_up */
/*
 * NAME:  int_handler()
 *
 * FUNCTION: Perform clean up on receipt of an interrupt.
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */

void int_handler(int sig)
{
	ttape_flag = NO;
	clean_up(EXIT_NTF);

	return;
} /* end int_handler */
/*
 * NAME:
 *
 * FUNCTION:
 *
 * NOTES:
 *
 * RETURNS:
 *
 */

int de_adapter(char *parent) {
	int rc = DIAG_FALSE;
	struct  CuVPD *cuvpd;
	struct  listinfo cuvpd_info;
	char    criteria1[40] = {"name=%s and vpd like *SCSI-2DIFF*"};
	char    criteria2[40] = {"name=%s and vpd like *SCSI-2DE*"};
	char    criteria[128];

	sprintf(criteria, criteria1, parent);
	cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria, &cuvpd_info, 1, 2);
	if ((cuvpd != (struct CuVPD *)(-1)) &&
	    (cuvpd != (struct CuVPD *) NULL)) {
	        rc = DIAG_TRUE;
	} else {
	        sprintf(criteria, criteria2, parent);
	        cuvpd = get_CuVPD_list(CuVPD_CLASS, criteria,
	                               &cuvpd_info, 1, 2);
	        if ((cuvpd != (struct CuVPD *)(-1)) &&
	            (cuvpd != (struct CuVPD *) NULL)) {
	                rc = DIAG_TRUE;
	        }

	}

	return(rc);

} /* end da_adapter() */
/*
 * NAME:
 *
 * FUNCTION: Print debug data.
 *
 * NOTES: Called only if 1. Create a file called .DIAG_DTAPE_DBUG in /tmp.
 *
 * RETURNS: void
 *
 */

void db(int dbug_num, ...)
{
	int           i, i1, i2, rc;
	char          *s1, *s2;
	char          fname[256];
	SCSI_TUTYPE   *db_tucb;
	FILE          *dbfptr;
	struct stat   file_status;
	va_list       ag;


	va_start(ag, dbug_num);
	sprintf(fname,"%s%s","/tmp/dbug.",tm_input.dname);

	if ((rc = stat("/tmp/.DIAG_DTAPE_DBUG",&file_status)) == -1) {
	        va_end(ag);
	        return; /* Debug file not present. */
	}

	if (!strcmp((char *)getenv("DIAG_DTAPE_DBUG"),"APPEND")) {
	        dbfptr = fopen(fname,"a+");
	} else {
	        if (dbug_num == 1) {
	                dbfptr = fopen(fname,"w+");
	        } else {
	                dbfptr = fopen(fname,"a+");
	        }
	}

	switch(dbug_num) {
	case 1: /* init dbug file */
	        fprintf(dbfptr,"============= start ============\n");
	        break;
	case 2: /* print db_tucb info before call to exectu(). */
	        db_tucb = va_arg(ag, SCSI_TUTYPE *);
	        fprintf(dbfptr,"> SCSI CDB (Data Out)\n\t");
	        p_data(dbfptr, db_tucb->scsitu.scsi_cmd_blk, 6);
	        if (db_tucb->scsitu.data_length) {
	                fprintf(dbfptr,"> Param List (Data Out)\n\t");
	                p_data(dbfptr, db_tucb->scsitu.data_buffer, 64);
	        }
	        break;
	case 3: /* print Sense Data */
	        fprintf(dbfptr, "> Sense Key %02.2X, ASC/ASCQ = %04.4X\n",
	                da.skey, da.scode);
	        fprintf(dbfptr,"> Sense Data\n\t");
	        p_data(dbfptr, da.sdata, 128);
	        break;
	case 10: /* Print multiple integers (int_name = int_value). */
	case 16:
	        i1 = va_arg(ag, int); /* Number of int name/value pairs. */
	        for (i = 0; i < i1; i++) {
	                s1 = va_arg(ag, char *);
	                i2 = va_arg(ag, int);
	                if (dbug_num == 16) /* Hex */
	                        fprintf(dbfptr, "> %s = %X\n", s1, i2);
	                else /* Decimal */
	                        fprintf(dbfptr, "> %s = %d\n", s1, i2);
	        }
	        break;
	case 20: /* print  multiple strings (ptr_name = ptr). */
	        i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
	        for (i = 0; i < i1; i++) {
	                s1 = va_arg(ag, char *);
	                s2 = va_arg(ag, char *);
	                fprintf(dbfptr, "> %s = %s\n", s1, s2);
	        }
	        break;
	case 25:
	        /* Print a simple string. */
	        s1 = va_arg(ag, char *);
	        fprintf(dbfptr, "%s\n",s1);
	        break;
	case 30: /* Display buffer data. */
	        s1 = va_arg(ag, int);
	        s2 = va_arg(ag, char *);
	        fprintf(dbfptr, "> Data Buffer\n\t");
	        p_data(dbfptr, s2, s1);
	        break;
	case 999:
	        fprintf(dbfptr,"======== end ========\n");
	        break;
	default:
	        break;
	}

	if (dbfptr != NULL)
	        fclose(dbfptr);
	va_end(ag);

	return;

} /* end db */
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

void p_data(FILE *dbfptr, char *ptr, int len) {
	int count = 0;

	/* Print data buffer, 16 bytes per line. */
	while ((len--) > 0) {
	        if (count == 8) {
	                fprintf(dbfptr,"    ");
	        } else if (count == 16) {
	                fprintf(dbfptr,"\n\t");
	                count = 0;
	        }
	        fprintf(dbfptr,"%02.2X ", *(ptr++));
	        ++count;
	}
	fprintf(dbfptr,"\n");

	return;
} /* end p_data */

/* end dtape.c */
