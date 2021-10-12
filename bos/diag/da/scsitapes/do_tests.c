static char sccsid[] = "@(#)19  1.14  src/bos/diag/da/scsitapes/do_tests.c, datape, bos411, 9428A410j 5/31/94 17:53:34";
/*
 * COMPONENT_NAME: datape
 *
 * FUNCTIONS:	do_tests()
 * 		dev_openx()
 *              get_tu_data()
 *		tu_test()
 *		da_tucb_init()
 *		str_to_buff()
 *		w_r_c_test()
 *		do_erp()
 *		get_erp_data()
 *		do_erp_reaction()
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

/* Function Prototypes */
int     do_tests(void);
void    dev_openx(void);
int     get_TU_data(void);
int     tu_test(void);
int     da_tucb_init(SCSI_TUTYPE *);
void    str_to_buff(char[], char *);
int     w_r_c_test(void);
int     do_erp(void);
int     get_erp_data(int, int *, int *);
int     do_erp_reaction(int);

extern int disp_menu(int);
extern void db(int, ...);

int 	pf_bit = -1;
int     ga_level_8200 = 0;
int     load_failed = ERP_GOOD;
int     retry_flag; /* True if retrying a Test Unit.          */
long    stop_tstamp, start_tstamp;
char    tu_buffer[1024];

extern DA_DATA         da; /* DA info struct */
extern int           fdes; /* Device file descriptor */
extern int         lun_id; /* Device Logical Unit ID */
extern int           step; /* Test step counter.     */
extern int   cfgpar_state; /* Config state of parent (scsi adapter). */
extern int   cfgdev_state; /* Config state of device.                */
extern int     ttape_flag; /* NO if no tape (init),                  */
	                   /* YES catalog value if tape avail.,      */
	                   /* USERS_TAPE if users tape in drive.     */
extern int      exit_flag; /* True if exiting.                       */
extern int       asl_flag; /* True if ASL init.                      */
extern int       test_seq; /* Type of test sequence indicator.       */
extern int      load_flag; /* First load is for a write ENABLE tape, */
	                   /* second is for a write PROTECTed tape.  */

extern struct tm_input tm_input;       /* Test Mode Input structure */

extern char test_sequence[]; /* Array to hold TU sequence string. */
/*
 * NAME: do_tests()
 *
 * FUNCTION: Configure and openx the device.
 *           Execute TU's in the test_sequence string.
 *
 * NOTES:
 *        - do_erp will increment the step counter.
 *
 *        - If interactive mode and Test Unit Ready,
 *          display the insert media message.
 *
 * RETURNS: 0 or will exit through do_erp().
 *
 */

int do_tests(void)
{
	int done = FALSE;
	int erp = FALSE;
	int my_errno;
	int max_step;
	int rc;

	/* Configure the parent of this device. */
	cfgpar_state = configure_device(tm_input.parent);
	if (cfgpar_state == -1) {
	        chk_ela();
	        exit_with_frub(DTAPE_CFG_PARENT);
	}

	/* Configure the device itself. */
	cfgdev_state = configure_device(tm_input.dname);
	if (cfgdev_state == -1) {
	        chk_ela();
	        exit_with_frub(DTAPE_CFG_DEVICE);
	}

	/* Open the device for Diagnostic testing. */
	dev_openx();

	if (EXITLM) {
	        sleep(1);
	        clean_up(EXIT_NTF);
	}

	/* Execute TU's in sequence. */
	step = 1;

	/* Get max TU's and the TU data for this step. */
	/* Max steps future ERP's change.              */
	max_step = get_TU_data();

	while(step <= max_step) {

db(10,1,"==> In do_tests(), step",step);

	        if (INTERACTIVE && (da.tu->tu_num == LOAD) && asl_flag &&
	            !UNIT_ATTENTION(da.tu_rc,da.skey)) {
	                if (ttape_flag == YES) {
	                        rc = disp_menu(INSERT_TAPE);
	                        rc = disp_menu(STAND_BY);
	                        sleep(2); /* Let tape start to load. */
	                }
	                if (SYSX) {
	                        diag_asl_quit();
	                        asl_flag = 0;
	                        clrdainput();
	                }
	        }

	        if ((da.tu->tu_num == SDTU_ST1) &&
	            (ttape_flag == USERS_TAPE)) {
	                /* Leave screen up 2 seconds. */
	                sleep(2);
db(10,1,"Users tape",1);
	                clean_up(EXIT_NTF);
	        }

		if(ENTERLM && SYSX ) {	/* do not perform the test */
			step++;
			continue;
		}

	        /* Set up to execute a Test Unit. */
	        rc = tu_test();

	        /* If the TU did not return the expected rc, */
	        if (rc != SCATU_GOOD) {
	                /* do the ERP's for the TU. */
	                rc = do_erp();
db(10,1,"ERP rc",rc);
	                step += rc;
	        } else {
	                ++step;
	        }

	        /* Get max TU's and the TU data for this step. */
	        /* Max steps future ERP's change.              */
	        max_step = get_TU_data();

	} /* end while */

	return(0);

} /* end do_tests */
/*
 * NAME: dev_openx()
 *
 * FUNCTION: Open the device for Diagnostic testing.
 *
 * NOTES: Returns if openx OK, else clean_up with either ERRO OPEN or FRU.
 *
 * RETURNS: void
 *
 */

void dev_openx(void)
{
	char devname[40];
	int my_errno;
	int rc;

	/* Use openx() to open the device in Diagnostic mode.  */
	/* If openx OK, then send a BDR to the device before   */
	/* testing begins.                                     */

	if (fdes != -1)
	        close(fdes);

	sprintf(devname, "/dev/%s", tm_input.dname);
	fdes = openx(devname, O_RDWR, NULL, SC_DIAGNOSTIC);
	my_errno = errno;
	if (fdes < 0) {

db(16,1,"openx(DIAG) failed, errno",my_errno);

	        if (my_errno == EAGAIN || my_errno == EACCES || my_errno == EBUSY) {
	                /* Set status to display the      */
	                /* "device reports busy" message. */
	                clean_up(EXIT_ERR_OPEN);
	        }
	        chk_ela();
	        exit_with_frub(DTAPE_OPENX_FAILED);

	} else {
	        /* Close the device, then do a FORCED OPEN  */
	        /* to reset the drive (BDR) before testing. */
	        close(fdes);
	        fdes = openx(devname, O_RDWR, NULL,
	                     SC_FORCED_OPEN | SC_DIAGNOSTIC);
	        my_errno = errno;
	        if (fdes < 0) {
db(16,1,"openx(FORCED) failed, errno",my_errno);
	                if (my_errno == EAGAIN || my_errno == EACCES || my_errno == EBUSY) {
	                        clean_up(EXIT_ERR_OPEN);
	                }
	                chk_ela();
	                exit_with_frub(DTAPE_OPENX_FAILED);
	        }
	}
	sleep(10); /* Give device time to recover from BDR. */
	return;

} /* end dev_openx */
/*
 * NAME: get_TU_data()
 *
 * FUNCTION: Determine the current TU number from the
 *           test_sequence string using step as an index.
 *           Get either the device specific TU data, if any,
 *           or the default TU data.
 *
 * NOTES:
 *
 * RETURNS: 0 if good, -1 on error.
 *
 */

int get_TU_data(void)
{
	int i , index;
	int max_step;
	int seq_len;
	int tu_num;
	char tu[8];
	char *tu_seq;

	tu_seq = test_sequence;
	seq_len = strlen(tu_seq);

	/* Determine the TU number for this step. */
	index = (step - 1) * 3;
	if ( (seq_len < (index + 1)) || (index < 0) )
	        return(0);

	/* Get the current TU num. */
	strncpy(tu, &tu_seq[index], 2);
	tu[2] = '\0'; /* tu_seq is longer, so terminate string. */

	tu_num = strtol(tu,(char **)NULL,16);

	/* Check dev struct first for specific TU data. */
	i = 0;
	while (dev_tus[i].led != 0) {
	        if ((dev_tus[i].led == dev[da.dev_type].led) &&
	            (dev_tus[i].tu.tu_num == tu_num) )
	                break;
	        else
	                ++i;
	}

	if (dev_tus[i].led != 0) {
	        da.tu = &dev_tus[i].tu;
	} else {
	        /* Get the default TU data. */
	        i = 0;
	        while (tus[i].tu_num != tu_num) {
	                if (tus[i++].tu_num == 0) {
db(10,1,"SW ERROR",7);
db(16,1,"  tu_num",tu_num);
	                        clean_up(EXIT_SW_ERR);
	                }
	        }
	        da.tu = &tus[i];
	} /* end if */

	/* Return max number of TU's in tu_seq.                 */
	/* example: "01 02" --> (5 + 1) / 3 = 2 TU's in tu_seq. */

	return( (seq_len + 1) / 3 );

} /* end get_TU_data */
/*
 * NAME: tu_test()
 *
 * FUNCTION: Set up the tucb struct and Execute a particular test unit.
 *
 * NOTES:
 *         - Init tucb struct.
 *         - Call exectu().
 *         - If a check condition is returned,
 *             - Leave da.tu_rc equal to the TU that returned the
 *               check condition.
 *             - Init tucb for Request Sense call exectu() again
 *               to get the sense data.
 *             - Put sense data in da struct.
 *
 * RETURNS: 0 if good, -1
 *
 */

int tu_test(void)
{
	SCSI_TUTYPE     tucb;
	TU_DATA         *tu_save;
	int             i, rc;
	int             str_len;

	/* Check for the Write, Read, and Compare test. */
	if (da.tu->tu_num == DA_W_R_C_TEST) {

	        /* Close device so tar can open. */
	        if (fdes > -1)
	                close(fdes);
	        da.tu_rc = w_r_c_test();

db(10,1,"Write, Read, Compare Test rc",da.tu_rc);

	        /* Reopen the device to continue testing. */
	        dev_openx();

	        /* Set flag to insert write protected tape */
	        /* on next load command.                   */
	        load_flag = PROTECT;
	        return(da.tu_rc);
	}

	/* Dont unload the tape in non-interactive modes. */
	if (!INTERACTIVE && (da.tu->tu_num == UNLOAD)) {
	        da.tu_rc = SCATU_GOOD;
	        return(da.tu_rc);
	}

	/* Set up tucb for the exectu() call. */
	tucb.header.tu = da.tu->tu_num;

	/* For certain TU's, set the TU number to the correct TU define. */
	/* The CDB and data buffer will be set for correct operation.    */
	switch(da.tu->tu_num) {
	case SDTU_ST2:
	        if (ga_level_8200) /* Hangs after Send Diag. */
	                return(SCATU_GOOD);
	        /* NO BREAK */
	case SDTU_ST1:
	        tucb.header.tu = SCATU_SEND_DIAGNOSTIC;
	        break;
	case LOAD:
	        if (da.dev_type == T4MM || da.dev_type == T4MM4GB) {
	                /* Set a time stamp to determine if ttape was */
	                /* inserted into the drive.                   */
	                start_tstamp = time(0);
	                /* Allow device time to disconnect. */
	                sleep(30);
	                return(SCATU_GOOD);
	        }
	        /* NO BREAK */
	case UNLOAD:
	        tucb.header.tu = SCATU_LOAD_UNLOAD_UNIT;
	        break;
	case TUR_4MM4GB:
	case TUR_4MM4GB1:
	        tucb.header.tu = SCATU_TEST_UNIT_READY;
	        break;
	default:
	        break;
	}

	/* Initialize the tucb struct with da struct data. */
	rc = da_tucb_init(&tucb);
	if (rc == -1)
	        return(-1);

        if ((pf_bit > 0) && (tucb.header.tu == SCATU_MODE_SELECT)) {
		/* Toggle the PF bit and try the command again. */
		tucb.scsitu.scsi_cmd_blk[1] ^= 0x10;
	}

db(2,&tucb);

	/*****************************************/
	/* Execute the Test Unit.                */
	da.tu_rc = exectu(fdes, &tucb);

db(16,1,"Test Unit",da.tu->tu_num);
db(10,1,"  rc",da.tu_rc);

	/* If a check condition is returned, issue a REQUEST_SENSE  */
	/* command to get the sense data.                           */

	if ( CHECK_CONDITION(da.tu_rc) && !exit_flag ) {
	        /* Get the REQUEST_SENSE tus struct. */
	        tu_save = da.tu;
	        i = 0;
	        while (tus[i].tu_num != SCATU_REQUEST_SENSE) {
	                if (tus[i++].tu_num == 0)
	                        return(-1);
	        }
	        da.tu = &tus[i];
	        tucb.header.tu = da.tu->tu_num;
	        rc = da_tucb_init(&tucb);
	        if (rc == -1)
	                return(-1);
	        /* Execute the Request Sense Test Unit.  */
	        rc = exectu(fdes, &tucb);

	        if (rc != SCATU_GOOD) {
	                rc = exectu(fdes, &tucb); /* Retry TU. */
	                if (rc != SCATU_GOOD)
	                        exit_with_frub(SCATU_REQUEST_SENSE);
	        }
	        da.skey  = tucb.scsiret.sense_key;
	        da.scode = tucb.scsiret.sense_code;
	        da.sdata = tucb.scsitu.data_buffer;
	        da.tu = tu_save;
db(3);
	} else {
	        da.skey = 0;
	        da.scode = 0;
	}

	if ((da.tu->tu_num == SCATU_INQUIRY) ||
	    (da.tu->tu_num == SCATU_MODE_SENSE) ||
	    (da.tu->tu_num == SCATU_RECEIVE_DIAGNOSTIC_RESULTS)) {
	        memcpy(tu_buffer,
	               tucb.scsitu.data_buffer,
	               tucb.scsitu.data_length);
	}

	/* Force some TU's into their ERP's (by returning 1). */
	if ( (da.tu_rc == SCATU_GOOD) &&
	     ((da.tu->tu_num == SCATU_MODE_SENSE) ||
	      (da.tu->tu_num == SCATU_INQUIRY) ||
	      (da.tu->tu_num == SCATU_TEST_UNIT_READY) ||
	      (da.tu->tu_num == SCATU_RECEIVE_DIAGNOSTIC_RESULTS)) ) {
	        return(1);
	}

	/* Return exectu status. */
	return(da.tu_rc);

} /* end of tu_test() */
/*
 * NAME: da_itucb_init()
 *
 * FUNCTION: Initialize the tucb struct prior to calling exectu().
 *
 * NOTES:
 *
 * RETURNS: 0 if good, -1 on error.
 *
 */

int da_tucb_init(SCSI_TUTYPE *tucb)
{
	int rc;

	tucb->header.mfg = 0;
	tucb->header.loop = 1;

	/* Initialize the tucb struct. */
	if (scsitu_init(tucb) != SCATU_GOOD)
	        return(-1);

	memset(tu_buffer, 0, 1024);
	tucb->scsitu.data_buffer = tu_buffer;

	/* Copy the CDB info from tu struct to tucb struct. */
	str_to_buff(tucb->scsitu.scsi_cmd_blk, da.tu->scsi_cmd_blk);

	/* If parameter list data, */
	if (da.tu->data_length) {
	        if (da.tu->tu_num == SCATU_WRITE)
	                memset(tucb->scsitu.data_buffer, 0xA5,
	                       da.tu->data_length);

	        if (strlen(da.tu->data_buffer)) {
	                str_to_buff(tucb->scsitu.data_buffer,
	                            da.tu->data_buffer);
	        }
	} /* end if */

	/* Set the correct "flags" value. */
	switch(tucb->header.tu) {
	case SCATU_REQUEST_SENSE:
	case SCATU_MODE_SENSE:
	case SCATU_INQUIRY:
	        tucb->scsitu.flags |= B_READ;
	        break;
	case SCATU_MODE_SELECT:
	case SCATU_SEND_DIAGNOSTIC:
	case SCATU_WRITE:
	        tucb->scsitu.flags |= B_WRITE;
	        break;
	default:
	        break;
	} /* end switch */


	/* Common tucb assignments. */
	tucb->scsitu.ioctl_pass_param = PASS_PARAM;
	if (lun_id < 8)
		tucb->scsitu.scsi_cmd_blk[1] |= lun_id << 5;
	else
		tucb->scsitu.lun = lun_id;
	tucb->scsitu.data_length = da.tu->data_length;
	tucb->scsitu.command_length = 6;
	switch (tucb->header.tu) {
	case SCATU_SEND_DIAGNOSTIC:
	        tucb->scsitu.cmd_timeout =
	                           60 * atoi(dev[da.dev_type].test_time);
	        break;
	case SCATU_RECEIVE_DIAGNOSTIC_RESULTS:
	        /* 4MM4GB auto diags take about 1.5 mins */
	        /* sleep for 2 min before next command   */
	        /* 4MM auto diags take about 4.5 mins.   */
	        /* Allow 6 min for the command timeout.  */
	        switch(da.dev_type) {
	        case T4MM4GB:
	                sleep (30);
	                break;
	        default:
	                tucb->scsitu.cmd_timeout = 60 * 6;
	                break;
	        }
	        break;
	case SCATU_LOAD_UNLOAD_UNIT:
	        tucb->scsitu.cmd_timeout =
	                           60 * atoi(dev[da.dev_type].load_time);
	        break;
	default:
	        tucb->scsitu.cmd_timeout = 60;
	        break;
	}

	return(0);

} /* end da_tucb_init */
/*
 * NAME: str_to_buff()
 *
 * FUNCTION: Convert the contents of a data byte string to a
 *           data buffer (buff).
 *
 * NOTES: Expected str2 syntax:  "01 0A 00 C0 0F FF"
 *        One or more data buffer byte values (in hex) separated by a space.
 *
 *        Result: buff[0] = 0x01
 *                buff[1] = 0x0A
 *                buff[2] = 0x00
 *                buff[3] = 0xC0
 *                buff[4] = 0x0F
 *                buff[5] = 0xFF
 *
 * RETURNS: void
 *
 */

void str_to_buff(char buff[], char *str2)
{
	int  i, max_bytes;
	char byte[4];
	char *ptr;

	/* Example: strlen("01 0A 00 C0 0F FF") = 17 */
	/*    (17 + 1) / 3 = 6 max_bytes in str2     */

	max_bytes = (strlen(str2) + 1) / 3;

	ptr = str2;
	for (i = 0; i < max_bytes; i++) {
	        strncpy(byte, ptr, 2);
	        byte[2] = '\0';
	        buff[i] = strtol(byte, (char *)NULL, 16);
	        ptr += 3;
	}

	return;

} /* end str_to_buff */
/*
 * NAME: w_r_c_test
 *
 * FUNCTION: Set up and Execute a write, read, and compare test.
 *
 * NOTES:
 *         - Use the tar command to write data to the tape. This
 *           is needted to test the SCSI interface to the drive.
 *
 * RETURNS: 0 if good (ignore system errors), error count or -1 on
 *          error (device).
 *
 */

int w_r_c_test(void)
{
	int             i,rc;
	int             w_r_c_err;      /* error counter */
	char            w_r_c_buff[W_R_C_SIZE+1]; /* tmp buff for data */
	char            wrc_file[128];  /* will be the wrc file name */
	FILE            *fptr;          /* ptr to wrc file */
	char            sys_command[128];

	/* file name should be $DIAGDATADIR/dt_XXXX"   */
	/* where XXXX will be tm_input.dname (rmt0, rmt1, etc.).     */
	/* This name will allow multiple tape drives to be tested    */
	/* with the system exerciser.                                */

	strcpy(wrc_file,(char *)getenv("DIAGDATADIR"));
	if ( strlen(wrc_file) == 0 )
	        return(0);
	strcat(wrc_file,"/dt_");
	strcat(wrc_file,tm_input.dname);

	/* try to open (create) the file */

	if ((fptr = fopen(wrc_file,"w")) == NULL) {
	        return(0);
	}

	/* write 0xAA 0x55 0xAA ... to the file. */

	for(i = 0;i < W_R_C_SIZE;i++) {
	        if(i % 2) {
	                fputc(0xAA,fptr);
	        } else {
	                fputc(0x55,fptr);
	        }
	}
	fclose(fptr);

	/* Use tar to write the file to the tape drive. */

	sprintf(sys_command,
	        "/bin/tar -cf/dev/%s %s 1>/dev/null 2>&1 ",
	        tm_input.dname,
	        wrc_file);
	rc = system(sys_command);
	if (rc != 0) {
	        /* Error in calling the tar command. Remove the file */
	        /* and return good status.                           */
	        unlink(wrc_file);
	        db(10,1,"ERROR: tar -c, rc",rc);
	        return(0);
	}

	/* Unlink (remove) the file */
	unlink(wrc_file);

	/* Use tar to read the file from the tape drive. */

	sprintf(sys_command,
	        "/bin/tar -xf/dev/%s %s 1>/dev/null 2>&1 ",
	        tm_input.dname,
	        wrc_file);
	rc = system(sys_command);
	if (rc != 0) {
	        /* Error in calling the tar command. Remove the file */
	        /* and return good status.                           */
	        db(10,1,"ERROR: tar -x, rc",rc);
	        unlink(wrc_file);
	        return(0);
	}

	/* try to open the file. */

	if ((fptr = fopen(wrc_file,"r")) == NULL) {
	        unlink(wrc_file);
	        return(0);
	}

	/* Check if the data is the same as it was before the */
	/* write to and read from the tape drive.             */

	w_r_c_err = 0;
	for(i = 0;i < W_R_C_SIZE;i++) {
	        if(i % 2) {
	                if( fgetc(fptr) != 0xAA)
	                        ++w_r_c_err;
	        } else {
	                if( fgetc(fptr) != 0x55)
	                        ++w_r_c_err;
	        }
	}

	fclose(fptr);

	/* Unlink (remove) the file */
	unlink(wrc_file);

	/* Return the error count. */

	return(w_r_c_err);

}  /* end w_r_c_test */
/*
 * NAME: do_erp()
 *
 * FUNCTION: Do the ERP (Error Recovery Path) for the TU that failed.
 *
 * NOTES:       Example erp attribute = "001:002 001:006 002:007"
 *
 *              i.e. erp1 = if condition 01, take reaction 002
 *                   erp2 = if condition 01, take reaction 006
 *                   erp3 = if condition 02, take reaction 007
 *
 * RETURNS: ERP rc,   ERP_GOOD (1) to incremnet step,
 *                    ERP_PREV_LOAD (-2) to decremnet step to the
 *                                       previous load.
 *                    ERP_FAIL
 *                    ERP_RETRY (0) retry current step (TU).
 */

int do_erp(void)
{
	int cond_found = FALSE; /* ERP condition found flag       */
	int cond;               /* Holds current condition value. */
	int react;              /* Holds current reaction value.  */
	int erp_rc = ERP_FAIL;
	int index;
	int erp_count = 1;
	int max_erps = 1;

	/* Test ERP conditions. When a condition is found to be true, */
	/* set a flag and perform the reaction.                       */

	while(!cond_found && (erp_count <= max_erps)) {

	        /* Get the max ERP's and the current condition */
	        /* and reaction codes from the ERP string.     */
	        max_erps = get_erp_data(erp_count, &cond, &react);


	        switch(cond) {
	        /* Check if the condition code is equal to a    */
	        /* sense key code and a check condition exists. */
	        case 0: case 1: case 2: case 3: case 4: case 5:
	        case 6: case 7: case 8: case 9: case 0xA: case 0xB:
	        case 0xC: case 0xD: case 0xE: case 0xF:
	                if (CHECK_CONDITION(da.tu_rc) && (da.skey == cond))
	                        cond_found = 1; /* cond code is the skey. */
	                break;
	        case 16: /* TU passed. */
	                if (da.tu_rc == SCATU_GOOD)
	                        cond_found = 1;
	                break;
	        case 17:
	                /* 1/2" 9348 will return 04 8900 if no tape in */
	                /* the drive on a LOAD command.                */
	                if (CHECK_CONDITION(da.tu_rc) && (da.skey == 0x04) &&
	                    (da.scode == 0x8900))
	                        cond_found = 1; /* cond code is the skey. */
	                break;
	        case 18:
	                /* 1/2" 9348 will return 04 4000 if WP tape in */
	                /* the drive on a SEND DIAG command.           */
	                if (CHECK_CONDITION(da.tu_rc) && (da.skey == 0x04) &&
	                    (da.scode == 0x4000))
	                        cond_found = 1; /* cond code is the skey. */
	                break;
	        case 19: /* TIMEOUT */
	                if (da.tu_rc == SCATU_TIMEOUT) {
	                        cond_found = 1;
	                }
	                break;
	        case 90: /* Used to react to any condition */
	                cond_found = 1;
	                /* Check for misc conditions. */

	                if ((da.tu_rc != SCATU_GOOD) &&
	                    (!CHECK_CONDITION(da.tu_rc))) {
	                        /* Something bad happened, dont retry. */
	                        exit_with_frub(da.tu->tu_num);
	                }

	                /* Model 8505 CLEAN/CLEANED bit set (sk=0). */
	                if ((da.dev_type == T8505_SE) ||
	                    (da.dev_type == T8505_DE)) {

	                        if (CLN_BIT_8505(da.tu_rc,
	                                         da.scode,
	                                         da.sdata[FSC_8505]))
	                                disp_mng(MNG_CLEAN_DRIVE,CLEAN_UP);

	                        if (CLND_BIT_8505(da.tu_rc,
	                                          da.scode,
	                                          da.sdata[FSC_8505]))
	                                return(ERP_RETRY);

	                } /* end if 8505 */
	                if (da.dev_type == T4MM) {
	                        if (NO_SENSE(da.tu_rc,da.skey) &&
	                            (da.scode == CLN_ME_4MM)) {
	                                disp_mng(MNG_CLEAN_DRIVE,CLEAN_UP);
	                        }
	                }
	                break;
	        case 98:
	                /* Check for a possible SCSI Bus reset. */
	                if (da.tu_rc == SCATU_ADAPTER_FAILURE)
	                        cond_found = 1;
	                break;
	        case 128:
	                /* If the tar command (system call) returns */
	                /* 128, then the root volume may be full.   */
	                if (da.tu_rc == 128)
	                        cond_found = 1;
	                break;
	        /* Model 3600  1/4" 150MB *
	        case 200:
	                break;            */
	        /* Model 3800  1/4" 500MB *
	        case 250:
	                break;            */
	        /* Model 4100  1/4" 1.2GB *
	        case 300:
	                break;            */
	        /* Model 8200  8mm  2.3GB *
	        case 350:
	                break;            */
	        /* Model 8505  8mm  5GB   *
	        case 400:
	                break;            */
	        /* Model SUITE  4mm       */
	        case 450: /* Diag tape inserted. */
	                if (CHECK_CONDITION(da.tu_rc) && (da.skey == 0x02) &&
	                    (da.scode == 0x3004))
	                        cond_found = 1;
	                break;
	        case 451: /* No media present. */
	                if (CHECK_CONDITION(da.tu_rc) && (da.skey == 0x02) &&
	                    (da.scode == 0x3A00))
	                        cond_found = 1;
	                break;
	        /* Model 9348  1/2"       *
	        case 500:
	                break;            */
	        /* OST                    *
	        case 550:
	                break;            */
	        default:
	                break; /* Try next ERP (if any). */
	        } /* end switch */

	        ++erp_count;

	} /* end while */

db(10,2,"ERP cond",cond, "ERP react",react);

	if (cond_found)
	        erp_rc = do_erp_reaction(react);

	if ((erp_rc == ERP_RETRY) || (erp_rc == ERP_PREV_LOAD)) {
	        ++retry_flag;
db(10,1,"retry_flag",retry_flag);
	} else {
	        retry_flag = 0;
	}

	if ((erp_rc == ERP_FAIL) || (retry_flag > MAX_RETRYS))
	        exit_with_frub(da.tu->tu_num);

	/* Return the proper value to for next step. */
	return(erp_rc);

} /* end do_erp */
/*
 * NAME: get_erp_data()
 *
 * FUNCTION: Process ERP string and set condition and reaction code values.
 *           Return max ERP's in the ERP string.
 *
 * NOTES:
 *
 * RETURNS: Max ERP's in ERP string, or -1 on error.
 *
 */

int get_erp_data(int erp_count, int *cond, int *react)
{
	int  index;
	char temp[4];

	/* ERP string will be in the form "000:000 000:000 ... 000:000"     */
	/*                                  ERP 1   ERP 2       ERP n       */
	/* An ERP has a strlen of 7, followed by a space, that makes 8.     */
	/* The (erp_count - 1) * 8 sets index to point to the first char of */
	/* the current ERP in the string.                                   */
	/* Also, (strlen + 1) / 8 tells how many ERP's in the string.       */
	/* All ERP's are strlen 8 (including the space) except the last     */
	/* one. Add one for this and divide by 8 for max ERP's.             */

	index = (erp_count - 1) * 8;
	/* If strlen less than one ERP or index less than 0, return error.  */
	if ( (strlen(da.tu->erps) < (index + 6)) || (index < 0) )
	        return(-1);

	/* Get current condition and reaction from the ERP string. */
	strncpy(temp, &da.tu->erps[index], 3);
	temp[3] = '\0';
	*cond = atoi(temp);
	strncpy(temp, &da.tu->erps[index + 4], 3);
	temp[3] = '\0';
	*react = atoi(temp);

	/* Return the max number of ERP's. */
	return( (strlen(da.tu->erps) + 1) / 8 );

} /* end get_erps */
/*
 * NAME: do_erp_reaction()
 *
 * FUNCTION: Peform the ERP reaction to recover from an error condition.
 *
 * NOTES:
 *
 * RETURNS: Max ERP's in ERP string, or -1 on error.
 *
 */

int do_erp_reaction(int react)
{
	int i;
	int r_rc;       /* Reaction return code */
	int rc;

	/* If SRN callout, exit with FRU. */
	if ( (react >= 70) && (react <= 85) )
	        exit_with_frub(da.tu->tu_num);

	/* Refer to the DA CIS for Reaction Code definitions. */
	/* Do the ERP reaction.                               */
	switch(react) {
	case 0: /* Return good status. */
	        r_rc = ERP_GOOD;
	        break;
	case 1: /* Retry Failing TU. */
	        r_rc = ERP_RETRY;
	        break;
	case 3: /* Display CHECK TAPE WP / WE message. */
	        if (INTERACTIVE && ttape_flag == YES) {
	                if (load_failed != react) {
	                        rc = disp_menu(CHECK_TAPE);
	                        load_failed = react;
	                        r_rc = ERP_PREV_LOAD;
	                } else {
	                        r_rc = ERP_FAIL;
	                }
	        } else {
	                r_rc = ERP_RETRY;
	        }
	        break;
	case 4: /* Sleep 1 minute, and retry. */
	        sleep(60);
	        r_rc = ERP_RETRY;
	        break;
	case 5: /* Exit with the bad test tape menu goal. */
	        r_rc = disp_mng(MNG_ELA_MEDIA,CLEAN_UP);
	        break;
        case 6: /* Set the PF bit flag to retry the command. */
		if (pf_bit < 0) {
			/* First time, set the flag. */
			pf_bit = 1;
		} else if (pf_bit > 0) {
			/* Flag already set once, set it to zero. */
			pf_bit = 0;
		}
		r_rc = ERP_RETRY;
		break;
	case 7: /* LOAD UNIT returned NOT READY. */
	        if (INTERACTIVE && ttape_flag == YES) {
	                if (load_failed != react) {
	                        rc = disp_menu(NO_TAPE);
	                        load_failed = react;
	                        r_rc = ERP_RETRY;
	                } else {
	                        r_rc = ERP_FAIL;
	                }
	        } else {
	                r_rc = ERP_GOOD;
	        }
	        break;
	case 8: /* TU READY returned GOOD status. */
	        /* Check first TU READY for users tape in drive. */
	        if ((step == 1) && (ttape_flag == NO)) {
	                ttape_flag = USERS_TAPE;
	        }
	        r_rc = ERP_GOOD;
	        break;
	case 9: /* TU READY returned NOT READY. */
	        if ((ttape_flag == YES) && (step != 1))
	                r_rc = ERP_RETRY;
	        else
	                r_rc = ERP_GOOD;
	        break;
	case 10: /* Mode Sense check for Write Protect bit. */
db(16,1,"WP bit",(tu_buffer[2] & 0x80));
	        if (((load_flag == ENABLE) && (tu_buffer[2] & 0x80)) ||
	            ((load_flag != ENABLE) && !(tu_buffer[2] & 0x80))) {
	                if (load_failed != react) {
	                        rc = disp_menu(CHECK_TAPE);
	                        load_failed = react;
	                        r_rc = ERP_PREV_STEP;
	                } else {
	                        r_rc = ERP_FAIL;
	                }
	        } else {
	                r_rc = ERP_GOOD;
	        }
	        break;
	case 11: /* Callout SRN for curren TU. */
	        r_rc = ERP_FAIL;
	        break;
	case 50: /* Clean the drive menu goal. */
	case 51:
	        disp_mng(MNG_CLEAN_DRIVE,CLEAN_UP);
	        break;
	case 52: /* Media error, clean drive and try new test tape. */
	        disp_mng(MNG_CLEAN_NEW_TTAPE,CLEAN_UP);
		r_rc = ERP_GOOD;
	        break;
	case 53: /* Display Servie Hints for SCSI Devices. */
	        disp_mng(MNG_TAPE_SERVICE_HINTS,CLEAN_UP);
	        break;
	case 60: /* Debug */
	        r_rc = ERP_GOOD;
	        break;
	case 98:
	        /* React to a SCATU_ADAPTER_FAILURE (possible SCSI */
	        /* Bus reset).                                     */
	        /* Exit the DA with a menu goal. */
	        disp_mng(MNG_SCSI_BUS_RESET,CLEAN_UP);
	        break;
	case 128:
	        clean_up(EXIT_SUB);
	        break;
	case 198: /* Search Std. Inquiry data for ... */
db(30,40,tu_buffer);
	        /* Check for GA level 8mm 8200. */
	        if (da.dev_type == T8200) {
	        /* Check for 8mm GA level drive type. */
	                if (!strncmp("21F8605", &tu_buffer[36],7) ||
	                    !strncmp("21F8610", &tu_buffer[36],7)) {
	                        /* Drives hang for some reason. */
	                        /* Dont run WRC test.           */
	                        ga_level_8200 = 1;
db(10,1,"GA level 8200",ga_level_8200);
	                }
	        }
	        r_rc = ERP_GOOD;
	        break;
	case 199: /* Sense key 04 (HW error), check sense code for */
	          /* possible media error for certain drive types. */
	        switch(da.dev_type) {
	        case T3600:
	                switch(da.scode & 0x00ff) {
	                case 0x06:
	                case 0x0C:
	                case 0x1A:
	                case 0x1B:
	                case 0x2F:
	                case 0x32:
	                case 0x33:
	                        disp_mng(MNG_CLEAN_NEW_TTAPE,CLEAN_UP);
				r_rc = ERP_GOOD;
	                        break;
	                default:
	                        r_rc = ERP_FAIL;
	                        break;
	                }
	                break;
	        case T3800:
	        case T4100:
	                switch(da.scode & 0x00ff) {
	                case 0x06:
	                case 0x0C:
	                case 0x1A:
	                case 0x1B:
	                        disp_mng(MNG_CLEAN_NEW_TTAPE,CLEAN_UP);
				r_rc = ERP_GOOD;
	                        break;
	                default:
	                        r_rc = ERP_FAIL;
	                        break;
	                }
	                break;
	        default:
	                r_rc = ERP_FAIL;
	                break;
	        }
	        break;
	/* Model 35480 4mm       */
	case 450:
	        stop_tstamp = time(0);
db(16,2,"  4mm error code",tu_buffer[1],"  4mm FRA",tu_buffer[2]);
db(10,1,"  4mm test time",(stop_tstamp - start_tstamp));
	        /* First look at the time stamp.                 */
	        if ( (stop_tstamp - start_tstamp) < 120 ) {
	                /* If not enough time went by, the tape must not */
	                /* have been inserted into the drive.            */
	                if (load_failed != react) {
	                        rc = disp_menu(NO_TAPE);
	                        load_failed = react;
	                        r_rc = ERP_PREV_STEP;
	                } else {
	                        r_rc = ERP_FAIL;
	                }
	                return(r_rc);
	        }
	        if (tu_buffer[1] != 0) {
	                /* Look at the error code. */
	                switch(tu_buffer[2]) {
	                case 2:
	                case 4:
	                        disp_mng(MNG_CLEAN_NEW_TTAPE,CLEAN_UP);
				r_rc = ERP_GOOD;
	                        break;
	                default:
	                        r_rc = ERP_FAIL;
	                        break;
	                }
	        } else {
	                /* No error. */
	                r_rc = ERP_GOOD;
	        }
	        break;
	case 451:
db(16,2,"  4mm error code",tu_buffer[5],"  4mm FRA",tu_buffer[6]);
	        if (tu_buffer[5] != 0) {
	                /* Look at the error code. */
	                switch(tu_buffer[6]) {
	                case 2:
	                case 4:
	                        disp_mng(MNG_CLEAN_NEW_TTAPE,CLEAN_UP);
				r_rc = ERP_GOOD;
	                        break;
	                default:
	                        r_rc = ERP_FAIL;
	                        break;
	                }
	        } else {
	                /* No error. */
	                r_rc = ERP_GOOD;
	        }
	        break;
	case 452: /* Sleep 3 minutes, and retry. */
	        sleep(60);
	        r_rc = ERP_RETRY;
	        break;
	case 803:
	        /* Return a SOFTWARE ERROR to the Diag Controller. */
	        clean_up(EXIT_SW_ERR);
	        break;
	default:
	        r_rc = ERP_RETRY;
db(10,1,"Invalid reaction code",react);
	        break;
	}

	return(r_rc);

} /* end do_erp_reaction */

/**** end do_tests.c ****/
