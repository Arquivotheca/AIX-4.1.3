static char sccsid[] = "@(#)57  1.3  src/bos/diag/util/u7135/u7135t.c, dsau7135, bos41J, 9512A_all 3/19/95 11:04:30";
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
 * NAME: dev_open()
 *
 * FUNCTION: Open a device in SC_DIAGNOSTIC mode.
 *
 * NOTES: - Param dev_name is the device name to open.
 *	    Example: "/dev/hdisk0"
 *	  - LUN value NOT set for device.
 *
 * RETURNS: SA_STATUS_GOOD: fdes >= 0
 *	    DM_ERROR_OPEN: fdes < 0 && errno = EAGAIN, EACESS or EBUSY
 *	    DM_STATUS_BAD: fdes < 0
 */

int dev_open(char *task) {
	int my_errno;
	char dev_name[64];
	char *erp = (char *)NULL;
	char *temp_ptr = (char *)NULL;

	erp = get_data(MISC_TASK_DATA, atoi(task));
	sa.erp_str = cat_data(ERP_DATA, " ", ";", erp);

	if (fdes > -1) {
		dt(25,"  Closing device.");
		close(fdes);
	}


	if (TASK_IS(OPEN_DAC_TASK,sa.task)) {
		sprintf(dev_name, "/dev/%s", dname);
		fdes = open(dev_name, O_RDONLY, NULL);
	} else if (TASK_IS(OPEN_LUN_TASK,sa.task)) {
		sprintf(dev_name, "/dev/%s", arr.lun_name);
		fdes = open(dev_name, O_RDONLY, NULL);
	} else if (TASK_IS(OPENX_LUN_TASK,sa.task)) {
		sprintf(dev_name, "/dev/%s", arr.lun_name);
		fdes = openx(dev_name, O_RDWR, NULL, SC_DIAGNOSTIC);
	} else if (TASK_IS(OPENX_DAR_TASK,sa.task)) {
		sprintf(dev_name, "/dev/%s", arr.dar_name);
		fdes = openx(dev_name, O_RDWR, NULL, SC_DIAGNOSTIC);
	}
	my_errno = errno;

	dt(10,1,"  fdes",fdes);
	dt(20,1,"  Device name",dev_name);

	if (fdes < 0) {
		dt(10,1,"ERROR: OPENX, errno",my_errno);
		if ((my_errno == EAGAIN) /* for tapes */ ||
		    (my_errno == EACCES) || (my_errno == EBUSY)) {
			return(DM_ERROR_OPEN);
		}
		if (my_errno == EINVAL) {
			return(DM_ERROR_EINVAL);
		}
		return(DM_STATUS_BAD);
	}

	return(DM_STATUS_GOOD);

} /* end dev_openx() */

/*
 * NAME: config()
 *
 * FUNCTION: Config dac & parent, dar, or hdisk (LUN).
 *
 * NOTES: The task type determines dac/dar/LUN.
 *	  The configure_device() routine will also configure the parent
 *	  SCSI adapter of the dac if it's in the DEFINED state.
 *
 * RETURNS: SA_STATUS_GOOD, DEV_CFG_FAILED,
 *	    DM_ERROR_OTHER: Invalid task type.
 */

int config(char *task) {
	int rc;
	char *erp = (char *)NULL;
	char *temp_ptr = (char *)NULL;

	erp = get_data(MISC_TASK_DATA, atoi(task));
	sa.erp_str = cat_data(ERP_DATA, " ", ";", erp);

	if (TASK_IS(CONFIG_DAC_TASK,sa.task)) {
		if (dev_cfg_state == -1) {
			dev_cfg_state = get_cfg_state(dname);
			dt(10,1,"dev_cfg_state",dev_cfg_state);
		}
		rc = configure_device(dname);
	} else if (TASK_IS(CONFIG_DAR_TASK,sa.task)) {
		if (arr.dar_cfg_state == -1) {
			arr.dar_cfg_state = get_cfg_state(arr.dar_name);
			dt(10,1,"dar_cfg_state",arr.dar_cfg_state);
		}
		sprintf(attr, " -l %s", arr.dar_name);
		rc = run_method(arr.dar_name, CFG_METHOD, attr);
	} else if (TASK_IS(CONFIG_LUN_TASK,sa.task)) {
		/* Configure the LUN. */
		if (arr.lun_cfg_state == -1) {
			arr.lun_cfg_state = get_cfg_state(arr.lun_name);
			dt(10,1,"lun_cfg_state",arr.lun_cfg_state);
		}
		sprintf(attr, " -l %s", arr.lun_name);
		rc = run_method(arr.lun_name, CFG_METHOD, attr);
	} else {
		return(DM_ERROR_OTHER);
	}

	if (rc < 0) {
		dt(10,1,"ERROR: CONFIG rc", rc);
		return(DEV_CFG_FAILED);
	}

	return(SA_STATUS_GOOD);

} /* end config() */

/*
 * NAME: get_pdisk_ffc
 *
 * FUNCTION: Get the pdisk FFC from the PDiagAtt ODM database.
 *
 * NOTES:
 *
 * RETURNS: (long) FFC
 *
 */

long get_pdisk_ffc(char *inq_pid) {
	int rc;
	long ffc;
	struct PDiagAtt pdiagatt;

	sprintf(attr, "DType = 7135_pdisk and attribute = %s", inq_pid);
	rc = (int)odm_get_first(PDiagAtt_CLASS, attr, &pdiagatt);
	if (rc < 1) {
		dt(20,1,"ERROR: pdisk FFC not in PDiagAtt, PID",inq_pid);
		ffc = 0;
	} else {
		ffc = strtol(pdiagatt.value,ep,16);
	}
	dt(16,1,"FFC",ffc);

	return(ffc);

} /* end get_pdisk_ffc() */

/*
 * NAME: get_pdisk_capacity
 *
 * FUNCTION: Get the pdisk capacity from the CuAt ODM database.
 *
 * NOTES:
 *
 * RETURNS: (char *)capacity
 *
 */

char *get_pdisk_capacity(char *dar_name, int ch, int id) {
	int rc;
	char *capacity;
	char *temp_ptr;
	struct CuAt cuat;

	capacity = (char *)malloc(PDISK_CAPSIZE);

	sprintf(attr, "name = %s and value LIKE 'Drive %d%d*'",
		dar_name, ch, id);
	rc = (int)odm_get_first(CuAt_CLASS, attr, &cuat);
	if (rc < 1) {
		dt(25,"ERROR: pdisk not in CuAt (capacity).");
		dt(10,2,"ch", ch, "id", id);
		strcpy(capacity, "Unknown");
	} else {
		/* Get the capacity in MB's. */
		temp_ptr = &cuat.value[(strlen(cuat.value)-4)];
		if (atoi(temp_ptr) >= 1000) {
			sprintf(attr, "%3.3f%s",
				((float)atoi(temp_ptr) / 1000),
				(char *)diag_cat_gets(catd,
					   U7135_MSG_SET, GB));
		} else {
			sprintf(attr, "%3.3f%s",
				atoi(temp_ptr),
				(char *)diag_cat_gets(catd,
					   U7135_MSG_SET, MB));
		}
		strcpy(capacity, attr);
	}

	return(capacity);

} /* end get_pdisk_capacity() */

/*
 * NAME: get_pdisk_pid
 *
 * FUNCTION: Get the pdisk Product ID from the CuAt ODM database.
 *
 * NOTES:
 *
 * RETURNS: (char *)pid
 *
 */

char *get_pdisk_pid(char *dar_name, int ch, int id) {
	int rc;
	char *pid;
	char *temp_ptr;
	struct CuAt cuat;

	pid = (char *)malloc(PDISK_CAPSIZE);

	sprintf(attr, "name = %s and value LIKE 'Drive %d%d*'",
		dar_name, ch, id);
	rc = (int)odm_get_first(CuAt_CLASS, attr, &cuat);
	if (rc < 1) {
		dt(25,"ERROR: pdisk not in CuAt (pid).");
		dt(10,2,"ch", ch, "id", id);
		strcpy(pid, "Unknown");
	} else {
		strncpy(pid, &cuat.value[18], (INQ_PID_SIZE - 1));
		pid[(INQ_PID_SIZE - 1)] = '\0';
	}

	return(pid);

} /* end get_pdisk_pid() */

/*
 * NAME: get_data
 *
 * FUNCTION: Get data from the d7135.data file.
 *
 * NOTES:
 *
 * RETURNS: (char *)
 *
 */

char *get_data(int msg_set, int msg_id) {
	int found_set = 0;
	int found_msg = 0;
	char file_name[64];
	char tempstr[64];
	char *line_buf;
	char msg_id_str[10];
	char err_msg[] = {"ERROR: Data not found."};
	FILE *fptr;
	struct data_t {
		char *tag;
	};
	struct data_t data[] = {
	/* 0 */ {" 0 not used "},
	/* 1 */ {"$set SEQUENCE_DATA"},
	/* 2 */ {"$set ASL_SCREEN_DATA"},
	/* 3 */ {"$set SCATU_TUCB_DATA"},
	/* 4 */ {"$set SC_CDB_DATA"},
	/* 5 */ {"$set MISC_TASK_DATA"},
	/* 6 */ {"$set ERP_DATA"},
	};

	line_buf = (char *)malloc(256);

	sprintf(tempstr, "%s/u7135.data", DEFAULT_UTILDIR);
	if ((fptr = fopen(tempstr, "r")) == NULL) {
		dt(25,"ERROR: Cant open data file.");
		return(err_msg);
	}

	if (msg_set == ASL_SCREEN_DATA) {
		sprintf(msg_id_str, " %03.3d", msg_id);
	} else {
		sprintf(msg_id_str, "%4d", msg_id);
	}

	while (!found_msg) {
		fgets(line_buf, 255, fptr);
		if (feof(fptr)) {
			dt(25,"ERROR: Cant find data.");
			return(err_msg);
		}

		if (!found_set) {
			if (!strncmp(data[msg_set].tag, line_buf,
				     strlen(data[msg_set].tag))) {
				found_set = 1;
			}
		} else {
			if (!strncmp(msg_id_str, line_buf,
				     strlen(msg_id_str))) {
				found_msg = 1;
			}
		}
	}

	fclose(fptr);

	if (line_buf[strlen(line_buf)-1] == '\n') {
		line_buf[strlen(line_buf)-1] = '\0';
	}

	return(&line_buf[5]);

} /* end get_data() */

/*
 * NAME: retry_mgr()
 *
 * FUNCTION: Manage a dynamic linked list of retry structs.
 *
 * NOTES:
 *    action param: CLEAR_RETRY = Clear the retry struct for this task.
 *		    RETRY_RETRY =
 *
 * RETURNS:  ERP_GOOD: When called with CLEAR_RETRY.
 *	    ERP_RETRY:
 *	     ERP_FAIL:
 */

int retry_mgr(int action, int retry_count) {
	int i, rc;
	int found = FALSE;
	struct retry *rptr, *back, *temp;

	/* dt(10,2,"In retry_mgr(), action",action,"Count",retry_count); */

	/* Set up some local pointers to the retry struct. */
	back = rptr = retry;

	/* Try to find an existing retry struct for this step */
	/* and this sequence.				      */
	while (rptr != (struct retry *)NULL) {
		/* dt(10,1,"retry step",rptr->step); */
		if ((rptr->step == sa.seq->step) &&
		    (!strcmp(rptr->task_str, sa.seq->task_str))) {
			found = TRUE;
			break;
		}
		back = rptr;
		rptr = rptr->next;
	}

	dt(10,1,"Retry found",found);

	/* The action parameter decides the retry action to perform. */
	switch(action) {
	case CLEAR_RETRY:
		if (found) {
			/* Remove this retry struct from the list. */
			dt(25,"Removing retry struct.");
			back->next = rptr->next;
			free(rptr);
		}
		rc = ERP_GOOD;
		break;
	case CHECK_RETRY:
		rc = ERP_GOOD;
		if (found && (rptr->count < 1)) {
				rc = ERP_FAIL;
		}
		dt(16,1,"Check retry rc",rc);
		break;
	case RETRY_RETRY:
		if (!found) {
			dt(25,"Creating new retry struct.");
			temp = (struct retry *)calloc(1, sizeof(struct retry));
			temp->task_str = sa.seq->task_str;
			temp->step = sa.seq->step;
			temp->count = retry_count;
			temp->next = retry->next;
			retry->next = temp;
			rc = ERP_RETRY;
		} else {
			rptr->count -= 1;
			dt(10,1,"retry count",rptr->count);
			if (rptr->count < 1) {
				dt(25,"Exceeded retry count!");
			}
			rc = ERP_RETRY;
		}
		break;
	default:
		rc = ERP_FAIL;
		break;
	}

	return(rc);

} /* end of retry_mgr() */

/*
 * NAME: get_eeprom_fptr()
 *
 * FUNCTION: Get the latest level of EEPROM data.
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_GOOD
 *
 */

int get_eeprom_fptr(void) {
	int level = -1;
	int prev_level = 0;
	char fname[128];
	char tempstr[128];
	char *lptr;

	sprintf(tempstr, 
		"%s /etc/array/fw/7135eeprom.?? -print  2>/dev/null", LS);
	ucode.fptr = popen(tempstr, "r");
	while(fgets(fname,80,ucode.fptr) != NULL) {
		/* Get rid of the newline character */
		fname[strlen(fname) - 1] = '\0';
		lptr = str_parse(fname,".",2);
		dt(20,1,"lptr",lptr);
		if ((strlen(lptr) == 2) && (atoi(lptr) >= prev_level)) {
			level = atoi(lptr);
		}
	}

	if (level >= 0) {
		sprintf(ucode.download_fname,
			"/etc/array/fw/7135eeprom.%02.2d", level);
	} else {
		display_screen("3184");
		clean_up(DM_STATUS_GOOD);
	}
	fclose(ucode.fptr);

	if ((ucode.fptr = fopen(ucode.download_fname, "rb")) == NULL) {
		display_screen("3185");
		clean_up(DM_STATUS_GOOD);
	}

	return(DM_STATUS_GOOD);

} /* end of get_eeprom_fptr() */

/*
 * NAME: get_ucode_fname()
 *
 * FUNCTION: Get the ucode file name to download.
 *
 * NOTES:
 *
 * RETURNS: DM_STATUS_GOOD
 *
 */

int get_ucode_fname(void) {
	int rc;
	char fname[128];
	char curr_level[64] = {"00"};
	char prev_level[64] = {"INIT"};
	char latest_level[64] = {""};
	char tempstr[128] = {""};
	int current_level_found = 0;
	static int diskette_inserted = FALSE;

	if (ucode.flags & DOWNLOAD_FROM_DISKETTE) {
		if (!diskette_inserted) {
			/* Display the "insert diskette" message. */
			diskette_inserted = TRUE;
			rc = display_screen("3163");
			if (rc != DIAG_ASL_ENTER) {
				return(DM_USER_EXIT);
			}
		}
		rc = display_screen("3151");
		if (rc != DIAG_ASL_OK) {
			return(DM_USER_CANCEL);
		}
		/* Get a list (toc) of the ucode files on the diskette */
		sprintf(tempstr, "%s%s", RESTBYNAME, 
			" -TqSf/dev/rfd0 2>/dev/null");
		ucode.fptr = popen(tempstr, "r");
		if (ucode.fptr == NULL) {
			dt(10,1,"diskette read 1 rc", rc);
			rc = display_screen("3174");
			return(DM_ERROR_OTHER);
		}
	} else {
		sprintf(tempstr, "%s /etc/array/fw/* -print  2>/dev/null", 
			FIND);
		ucode.fptr = popen(tempstr, "r");
	}


	if (sa_selected == SA_uCODE_CNTRL) {
		ucode.model_level_start = 26;
		ucode.model_level_length = 2;
	} else {
		ucode.model_level_start = 31;
		ucode.model_level_length = 8;
	}

	while(fgets(fname,80,ucode.fptr) != NULL) {
		/* Get rid of the newline character */
		fname[strlen(fname) - 1] = '\0';
		/* Check to see if the mcode files are compatible */
		dt(20,1,"fname",fname);
		if((rc = ucode_compatible(ucode.current_fname,fname)) != -1) {
			dt(10,1," mcode compatible rc", rc);
			if (rc == 0) {
				current_level_found = 1;
			}
			strncpy(tempstr,&fname[ucode.model_level_start],
				ucode.model_level_length);
			tempstr[ucode.model_level_length] = '\0';
			if ((rc >= 0) &&
			    !(ucode.flags & DOWNLOAD_PREVIOUS_LEVEL)) {
				dt(25,"Latest level");
				if (strcmp(tempstr, latest_level) >= 0) {
					++ucode.num_files;
					strcpy(latest_level,tempstr);
					dt(20,1,"latest_level", latest_level);
				}
			}
			if ((rc <= 0) &&
			    (ucode.flags & DOWNLOAD_PREVIOUS_LEVEL)) {
				dt(25,"Previous level");
				if ((!strcmp(prev_level, "INIT")) ||
				    ((strcmp(prev_level,"INIT")) &&
				     (strcmp(tempstr, prev_level) <= 0))) {
					++ucode.num_files;
					strcpy(prev_level,tempstr);
					dt(20,1,"prev_level", prev_level);
				}
			}
		}
	} /* end while */

	pclose(ucode.fptr);

	if ((ucode.num_files == 0) && (!current_level_found)) {
		dt(25,"No ucode files!");
	    	if (!(ucode.flags & DL_ALL_PDISKS)) {
			rc = display_screen("3161");
		}
		if (sa_selected == SA_uCODE_PDISK) {
			arr.pdisk_ptr->flags |= PDISK_DL_NO_FILE;
		}
		return(DM_USER_CANCEL);
	}
	if (!current_level_found && !(ucode.flags & DL_ALL_PDISKS)) {
		dt(25,"Current level not found.");
		rc = display_screen("3162");
		if (rc != (DEF_SELECTION + 1)) {
			return(DM_USER_CANCEL);
		}
	}

	/* Build the name of the mcode file to download */
	strncpy(ucode.download_fname,
		ucode.current_fname,
		ucode.model_level_start);
	ucode.download_fname[ucode.model_level_start] = '\0';

	if (ucode.flags & DOWNLOAD_PREVIOUS_LEVEL) {
		strcat(ucode.download_fname,prev_level);
	} else {
		strcat(ucode.download_fname,latest_level);
	}
	dt(20,1,"download name", ucode.download_fname);

	if (!(ucode.flags & DL_ALL_PDISKS) &&
	    !(strcmp(ucode.current_fname, ucode.download_fname))) {
		/* Already at current level, download anyway? */
		rc = display_screen("3164");
		if (rc != (DEF_SELECTION + 1)) {
			return(DM_USER_CANCEL);
		}
	}


	/* Read the ucode file from diskette. */
	if (ucode.flags & DOWNLOAD_FROM_DISKETTE) {
		sprintf(attr,
		  	"%s -xqSf/dev/rfd0 %s 1>/dev/null 2>&1",
		  	RESTBYNAME,
		  	ucode.download_fname);
		rc = system(attr);
		if (rc != 0) {
			dt(10,1,"diskette read 2 rc", rc);
			rc = display_screen("3174");
			return(DM_ERROR_OTHER);
		}
	}

	return(DM_STATUS_GOOD);

} /* end get_ucode_fname() */

/*
 * NAME: mcode_compatible()
 *
 * FUNCTION: This procedure is responsible for determining if the
 *	     microcode files passed are of the same type.
 *
 * NOTES:
 *
 * RETURNS: -1 if not same type,
 *	     0 if the same type and model level,
 *	    -2 if the same type but earlier model level,
 *	     1 if the same type but later model level.
 */

int ucode_compatible(char c_mcode_name[], char f_name[])
{
	int i;
	int rc;
	char current_name[128];
	char new_name[128];

	/* First check for exact match */
	if (strcmp(c_mcode_name,f_name) == 0) {
		return(0);
	}
	/* Set up local copies of the file names */
	strcpy(current_name,c_mcode_name);
	strcpy(new_name,f_name);

	/* Clear out the model level field */
	for (i = ucode.model_level_start;
	     i < (ucode.model_level_start + ucode.model_level_length); i++) {
		current_name[i] = '0';
		new_name[i] = '0';
	}
	current_name[i] = '\0';
	new_name[i] = '\0';

	/* Check if the files are the same type */
	if (strcmp(current_name,new_name) != 0) {
		return(-1);
	}
	rc = strcmp(f_name,c_mcode_name);

	if (rc > 0) {
		rc = 1;
	} else {
		rc = -2;
	}

	return(rc);

} /* end ucode_compatible() */


/* end u7135t.c */
