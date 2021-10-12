static char sccsid[] = "@(#)42  1.22.3.14  src/bos/diag/da/disks/dhf.c, dadisks, bos41J, 9523C_all 6/9/95 18:16:29";
/*
 *   COMPONENT_NAME: DADISKS
 *
 *   FUNCTIONS: analyze_sense_data
 *              certify_disk
 *              chk_asl_status
 *              chk_ela
 *              chk_terminate_key
 *              clean_up
 *              create_frub
 *              diag_tst
 *              display_title
 *              get_sense_data
 *              main
 *              segment_count
 *              test_if_failed
 *              tu_test
 *
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>            /* This was needed for debug.       */
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <errno.h>
#include <sys/cfgodm.h>
#include <sys/errids.h>
#include <sys/buf.h>
#include <sys/devinfo.h>
#include <sys/scdisk.h>
#include <sys/scsi.h>
#include <diag/scsi_atu.h>
#include <diag/scsd.h>

#include "dhf.h"                                    /* test cases and globals */
#include "dhf_msg.h"                               /* message catalog numbers */

#include <diag/dascsi.h>
#include <diag/tm_input.h>                                 /* faking the ODM  */
#include <diag/tmdefs.h>                   /* diagnostic modes and variables  */
#include <diag/bit_def.h>
#include <diag/da.h>                       /* FRU Bucket structure definition */
#include <diag/diag_exit.h>                                /* DA return codes */
#include <diag/diag.h>                      /* Structures etc, used by the DA */
#include <diag/diago.h>                  /* ASL structures etc used by the DA */
#include "diag/dcda_msg.h"                    /* catalog of standard messages */
#include <locale.h>

#define isSCSD  (!strcmp(cudv->PdDvLn_Lvalue, "disk/scsi/scsd"))

int             problem_found = FALSE;
static          uint            curblock;
static          uint            maxblock;
DRIVE_INFO      drive_info;
ODM_INFO        odm_info;
struct          tm_input tm_input;       /* DA test parameters nad other info */
SCSI_TUTYPE     tucb;                          /* structure for scsi commands */
int             failing_function_code;   /* Used to build SRN */
short           reformat=FALSE;        /* Flag to indicate drive needs format */
short           osdisk;                         /* Flag to indicate OEM disk  */
int             page_code;              /* what to set for MODE SENSE command */
uchar 		lun_id;
uchar 		scsi_id;

/* Global variables for command time outs */
int 		send_diag_to = 60;	/* send diagnostic comamnd time out   */
int		readx_to = 30;		/* read extended comamnd time out     */
int		start_stop_to = 60;	/* start/stop comamnd time out        */

/******************************************************************************/
/*  initialze the frubucket structure that will be used to report back        */
/*  a fru if a problem is found. The information in this structure is dynam-  */
/*  ically set up in the create_fru function if a problem is detected.        */
/******************************************************************************/

#define MAX_FRU_ENTRY   5

struct fru_bucket frub[] =
{
/* 0 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 0, "de", "", DHF_DRIVE_ELEC_CARD, NOT_IN_DB, EXEMPT },
                       { 0, "hda", "",DHF_DISK_ENCLOSURE ,NOT_IN_DB, EXEMPT },
                },
        },
/* 1 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 0, "de", "", DHF_DRIVE_ELEC_CARD, NOT_IN_DB, EXEMPT },
                },
        },
/* 2 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 0, "de", "", DHF_DRIVE_ELEC_CARD, NOT_IN_DB, EXEMPT },
                       { 0, "hda", "", DHF_DISK_ENCLOSURE, NOT_IN_DB, EXEMPT },
                       { 0 , "", "", 0, PARENT_NAME, EXEMPT },
                },
        },
/* 3 */ { "", FRUB1, 0x00, 0x00, DHF_ADAPTER_CONFIG_FAIL,
                {
                        {80, "", "", 0, PARENT_NAME, NONEXEMPT},
                        {20, "", "", DHF_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
                }
        },
/* 4 */ { "", FRUB1, 0x00, 0x00, DHF_DEVICE_CONFIG_FAIL,
                {
                        {80, "", "", 0, DA_NAME, EXEMPT},
                        {10, "", "", 0, PARENT_NAME, NONEXEMPT,},
                        {10, "", "", DHF_SOFTWARE_ERROR, NOT_IN_DB, EXEMPT},
                }
        },
/* 5 */ { "", FRUB1, 0x00, ELA_ERROR3, DHF_DA_MSG_DISK_ERR3,
                {
                        {80, "", "", DHF_SCSI_BUS, NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, NONEXEMPT},
                        {10, "", "", 0, PARENT_NAME, NONEXEMPT,},
                }
        }
};

struct error_log_info {
        char    soft_data_error_found;
        char    soft_equip_error_found;
        uint    totcnt;
};

static int	init_adapter_state = -1;
static int	init_config_state = -1;
extern int	disp_menu();
extern nl_catd	diag_catopen(char *, int);
extern int	process_sense_data(int, int, short, short *, short *);
extern int	get_odm_info(char *, int *, short *, ODM_INFO *);
extern int   set_default_sense(uchar *, int, uchar *, uchar *, uchar *, int *);
char		*devname;		/* device name used in openx cmd */
char		dname[] = "/dev/r";
nl_catd		catd;			/* pointer to the catalog file */
int		tu_test();
struct CuDv	*cudv;
struct CuAt	*cuat;
struct listinfo	obj_info;

void 		db(int,...);
void		p_data(FILE *, char *, int);
extern int	diag_get_sid_lun(char *, uchar *, uchar *);

/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the SCSI Fixed-Disk Diagnostic Application.
 *           It is invoked by the diagnostic Controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: The return values are set up in each function called.
 *          DA_EXIT() is called to return the codes to the
 *          controller.
 */

main(argc,argv,envp)

int argc;
char **argv;
char **envp;

{

        int     analyze_sense_data();
        int     certify_disk();
        void    chk_asl_status();
        void    chk_terminate_key();
        void    chk_ela();
        void    clean_up();
        void    display_title();
        void    diag_tst();
        register        int     index;
        int     test_if_failed();
        int     lvm_rc = -99;                          /* LVM/SYS return code */
        int     rc = -99;             /* generic ret code used lots of places */
        int     rc_errno = -99;
	char	criteria[128];
	struct  disk_scsd_inqry_data *scsd_vpd;
	struct  disk_scsd_mode_data  *scsd_mode_data;	

        setlocale(LC_ALL, "");
        damode = (long)NULL;
        fdes = -99;
        DA_SETRC_TESTS(DA_TEST_FULL);	/* set this to other when needed */
        if (init_dgodm() == bad_da_error) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
        /*
          get the input parameters from tm_input
        */
        if (getdainput(&tm_input) != FALSE) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();                  /* can't get good stuff, so quit */
        } else {           /* create name for the openx command - /dev/xxxxxx */
                for(index=0;index<=MAX_FRU_ENTRY;index++)
                        strcpy(frub[index].dname, tm_input.dname);
                devname = (char *) malloc(NAMESIZE + (strlen(dname)+1));
                if (devname != NULL)
                        strcpy(devname, dname);
                strcat(devname, tm_input.dname);
        }
        db(1);
	db(20,1,"Device name",tm_input.dname);
        /*
          get the bit encoded version of the stuff in tm_input
        */
        damode = getdamode(&tm_input);
        if (damode == (long)NULL) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();                  /* can't get good stuff, so quit */
        }
        /*
          start ASL if console is present and check return
        */
        if (damode & DA_CONSOLE_TRUE) {
                chk_asl_status(diag_asl_init(NULL));
                catd = diag_catopen(MF_DHF, 0);
        }
        /*
          get information from the odm data base
        */

	rc = get_odm_info(tm_input.dname, &failing_function_code,
			  &osdisk, &odm_info);
	if (rc < 0) {
		if (rc == -2) 		/* missing attributes */
			disp_menu(MISSING_ATTRIBUTES, failing_function_code);
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up();
	}

	sprintf(criteria, "name = %s", tm_input.dname);
	cudv = get_CuDv_list(CuDv_CLASS, criteria, &obj_info, 1, 2);

	if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL)){
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();                  /* can't get good stuff, so quit */
	}	

	if (diag_get_sid_lun(cudv->connwhere, &scsi_id, &lun_id) == -1){
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();                  /* can't get good stuff, so quit */
	}

        /*
          get set to decide types of tests to be executed
        */
        if (damode & DA_DMODE_ELA)
                chk_ela();
        else if (damode & DA_DMODE_PD || damode & DA_DMODE_REPAIR
                                || damode & DA_DMODE_FREELANCE) {
                /*
                  Configure the device first, then
                  open the device described by devname
                */
                if (!(damode & DA_LOOPMODE_EXITLM)) {
                        display_title();
                        init_adapter_state = configure_device(tm_input.parent);
                        if (init_adapter_state == -1){
                                DA_SETRC_TESTS(DA_TEST_FULL);
                                create_frub(ADAPTER_CONFIG_FAIL);
                                DA_SETRC_STATUS(DA_STATUS_BAD);
                                if (insert_frub(&tm_input, &frub[3]) != 0) {
                                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                                        clean_up();
                                }
                                if (addfrub(&frub[3]) != 0) {
                                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                                        clean_up();
                                }
                                clean_up();
                        }

                        init_config_state = configure_device(tm_input.dname);
                        if (init_config_state == -1){
                                if (!(damode & DA_DMODE_REPAIR)){
                                        chk_ela();

	                        	/* If error log analysis detected problem,
					   then exit */
                                        if (problem_found)
                                                clean_up();
                                }
                        	/*
                           	   If control gets here ELA did not detect problem or
                           	   ELA can not be run.
                        	*/
                                DA_SETRC_TESTS(DA_TEST_FULL);
                                create_frub(DEVICE_CONFIG_FAIL);
                                DA_SETRC_STATUS(DA_STATUS_BAD);
                                if (insert_frub(&tm_input, &frub[4]) != 0) {
                                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                                        clean_up();
                                }
                                if (addfrub(&frub[4]) != 0) {
                                        DA_SETRC_ERROR (DA_ERROR_OTHER);
                                        clean_up();
                                }
                                clean_up();
                        }

                        fdes = openx(devname, O_RDWR, NULL,SC_DIAGNOSTIC);
                        rc_errno = errno;             /* get errno value */

/* Check if openx failed. If it  did and mode = concurrent then issue the lvm
 * command lchangepv to try and free the drive. The cmd 'system' is used to
 * issue the cmd. If the lvm cmd fails the error is returned to the controller
 * else diag_tst is called. If the mode is not concurrent then an open error
 * is given back to DC.
 */
                        if (fdes < 0) {
                                if (rc_errno == EIO){
                                        DA_SETRC_TESTS(DA_TEST_FULL);
                                        create_frub(DD_CONFIG);
                                        DA_SETRC_STATUS(DA_STATUS_BAD);
                                        if (insert_frub(&tm_input,
                                              &frub[conf_level]) != 0) {
                                                 DA_SETRC_ERROR
                                                       (DA_ERROR_OTHER);
                                                clean_up();
                                         }
                                        if (addfrub(&frub[conf_level]) != 0) {
                                                DA_SETRC_ERROR
                                                        (DA_ERROR_OTHER);
                                                clean_up();
                                        }
                                        clean_up();
                                }
                                if (rc_errno == EACCES) {
                                        if ((damode & DA_CONSOLE_TRUE) &&
                                                (! tm_input.system)){
                                                rc = freedisk(tm_input.dname);
                                        }else{
                                                chk_ela();
                                                if (problem_found == FALSE)
                                                     rc = -2;
                                                else /* exit to report FRU */
                                                     clean_up();
                                        }
                                        switch (rc){
                                        case -5 :
                                                DA_SETRC_USER(DA_USER_QUIT);
                                                clean_up();
                                                break;
                                        case -4 :
                                                DA_SETRC_USER(DA_USER_EXIT);
                                                clean_up();
                                                break;
                                        case -3 :
                                        case -2 :
                                        case -1 :
                                                if (damode & DA_DMODE_PD)
                                                        chk_ela();
                                                if (problem_found == FALSE)
                                                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                                                clean_up();
                                                break;
                                        default :
                                                fdes = openx(devname, O_RDWR,
                                                        NULL, SC_DIAGNOSTIC);
                                                if (fdes < 0){
                                                        if (damode & DA_DMODE_PD)
                                                                chk_ela();
                                                        if (problem_found == FALSE)
                                                                DA_SETRC_ERROR(DA_ERROR_OPEN);
                                                        clean_up();
                                                }
                                                DA_SETRC_TESTS(DA_TEST_FULL);
                                                display_title();
                                                chk_terminate_key();
                                                diag_tst(fdes);
                                                chk_terminate_key();
                                                break;
                                        }

                                } else if ((rc_errno == EINVAL) ||
                                            (rc_errno == ENOENT) ||
                                            (rc_errno == ENXIO) ||
                                            (rc_errno == EPERM)) {
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        clean_up();
                                } else {    /* could not open so quit cleanly */
                                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                                        clean_up();
                                }
                        } else {
                                DA_SETRC_TESTS(DA_TEST_FULL);
				
				if (isSCSD){
					/*
					  This is an SCSD disk
					*/

					/* 
					  Get main SCSD information
					  for this device. 
					*/
 
					rc = tu_issue_scsi_inquiry(fdes,
							      SCATU_INQUIRY, 
							      0xc7);
                        		switch(rc) {
                        		case SCATU_TIMEOUT :
                        		case SCATU_BUSY :
                        		case SCATU_RESERVATION_CONFLICT :
                        		case SCATU_CHECK_CONDITION :
                                		create_frub (rc);
                                		DA_SETRC_STATUS(DA_STATUS_BAD);
                                		if (insert_frub(&tm_input, &frub[conf_level]) != 0) {
                                        		DA_SETRC_ERROR (DA_ERROR_OTHER);
                                		}
						frub[conf_level].sn=failing_function_code;
                                		if (addfrub(&frub[conf_level]) != 0) {
                                        		DA_SETRC_ERROR (DA_ERROR_OTHER);
                                		}
                                        	clean_up();
                                		break;
                        		case SCATU_GOOD :
                                		break;
                        		default :
                                		DA_SETRC_ERROR(DA_ERROR_OTHER);
                                		clean_up();
                        		}

					scsd_vpd = (struct disk_scsd_inqry_data *) tu_buffer;
						
					/*
					  Set SCSD time out values
					*/
					if (scsd_vpd->send_diag_timeout)
						send_diag_to = scsd_vpd->send_diag_timeout;
					if (scsd_vpd->rw_timeout)
						readx_to = scsd_vpd->rw_timeout;
					if (scsd_vpd->start_timeout)
						start_stop_to = scsd_vpd->start_timeout;

					/* 
					  Get SCSD desired mode data
					  for this device. 
					*/

					rc = tu_issue_scsi_inquiry(fdes,
							      SCATU_INQUIRY, 
							      0xc8);
					if (rc == SCATU_GOOD) {
						scsd_mode_data = (struct disk_scsd_mode_data *) tu_buffer;
						odm_info.mode_data_length = scsd_mode_data->page_length;
						bcopy(scsd_mode_data->mode_data,
						      odm_info.mode_data,
						      scsd_mode_data->page_length);
					}
					else {
						/*
						  If inquiry page 0xc8 does not
						  exist, then this means that we
						  don't need to issue mode select
						  data.  So set mode_data_length to
						  zero.
						*/
						odm_info.mode_data_length = 0;
						odm_info.mode_data[0] = '\0';
					}


				} 				

                                chk_terminate_key();
                                diag_tst(fdes);
                                chk_terminate_key();

                        } /* endif fdes < 0 */
                }
                /*
                  if pd mode, and notlm or enterlm, do ela last
                */
                if (damode & DA_DMODE_PD  && (damode & DA_LOOPMODE_NOTLM ||
                    damode & DA_LOOPMODE_EXITLM) &&
                    ((DA_CHECKRC_STATUS() == DA_STATUS_GOOD) ||
                    (DA_CHECKRC_ERROR() == DA_ERROR_OPEN)))
                        chk_ela();
        } /* endif */
        /*
          If verifying a repair, display some menus depending on repair action
        */
        if ((damode & DA_DMODE_REPAIR) && (damode & DA_LOOPMODE_NOTLM ||
               damode & DA_LOOPMODE_EXITLM)) {
                if ((reformat == TRUE) && (damode & DA_CONSOLE_TRUE)) {
                        chk_asl_status(disp_menu(RUN_FORMAT_SA, 
				       failing_function_code));
		}
        }
        clean_up();                    /* exit call to controller...no return */
}  /* main end */
/*  */
/*
 * NAME: certify_disk()
 *
 * FUNCTION: Certify the hard disk to detect soft errors.
 *
 * EXECUTION ENVIRONMENT: This procedure can page fault.
 *
 * NOTES:
 *
 * DATA STRUCTURE: drive_info is a structure containing a count
 *                 of recovered data and equipment check errors along
 *                 with unrecovered data and equipment check errors.
 *
 * RETURNS: -2 if certify failed, -1 if unsuccessful, otherwise 0.
 */
#define CERTIFY_STEPS 8
#define MODE_SELECT_STEP 1
#define READ_CAPACITY_STEP 2
#define READ_EXTENDED_STEP 3
#define CHECK_COMPLETION_STEP 4
#define REQUEST_SENSE_STEP 5
#define TERMINATE_STEP  6
#define PROCESS_SENSE_DATA_STEP 7
#define LAST_STEP 8

int
certify_disk(fdes)
int     fdes;
{
        uint    blocklength;
        void    chk_asl_status();
        uint    error_block;
        int     old_percent;
        register int    rc;
        short   reassign, error_type;
        int     selection;
        int     stepcount[CERTIFY_STEPS + 1];
        int     step;
        extern SCSI_TUTYPE tucb;               /* structure for the scsi cmds */
	float   blocklength_in_mega_bytes;

        /* Initialize variables */

	db(25,"***** In certify_disk() *****");
        drive_info.percent_complete = 0;
        drive_info.drive_capacity = 0;
        drive_info.rec_data_errors = 0;
        drive_info.unrec_data_errors = 0;
        drive_info.rec_equ_check_errors = 0;
        drive_info.unrec_equ_check_errors = 0;
        sprintf(drive_info.loc_code,"");
        old_percent = 0;
        curblock = 0;
        for(step = 0; step <= CERTIFY_STEPS; ++step)
                stepcount[step] = 0;
        if (damode & DA_CONSOLE_TRUE)
                chk_asl_status(disp_menu (PLEASE_STAND_BY, failing_function_code));
        step = READ_CAPACITY_STEP;
        /*
           Loop and run all tests until step is set to 0 or when
           errors exceeded the threshold, in which case
           a menu is displayed to tell user that the drive is bad.
        */

        while(step){
                ++stepcount[step];
		db(10,2,"step",step, "\tstepcount", stepcount[step]);
                switch(step)    {
                case MODE_SELECT_STEP: /* Only get here if Device was reset */
                        rc = tu_test(fdes, SCATU_MODE_SELECT);
                        switch(rc) {
                        case SCATU_GOOD :
                                step = READ_CAPACITY_STEP;
                                break;
                        default :
                                step = TERMINATE_STEP;
                                break;
                        }
                        break;

                case READ_CAPACITY_STEP:
                        rc = tu_test(fdes, SCATU_READ_CAPACITY);
                        switch(rc) {
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                        case SCATU_TIMEOUT:
                                step = TERMINATE_STEP;
                                break;
                        case SCATU_GOOD:
                                maxblock = (uint) tu_buffer[0] << 24 |
                                           (uint) tu_buffer[1] << 16 |
                                           (uint) tu_buffer[2] <<  8 |
                                           (uint) tu_buffer[3];
                                blocklength = (uint) tu_buffer[4] << 24 |
                                              (uint) tu_buffer[5] << 16 |
                                              (uint) tu_buffer[6] <<  8 |
                                              (uint) tu_buffer[7];
				blocklength_in_mega_bytes = (float) blocklength / 1000000;
				drive_info.drive_capacity = blocklength_in_mega_bytes * maxblock;

				db(10,2,"\tmaxblock", maxblock,
				   "blocklength", blocklength);

                                step = READ_EXTENDED_STEP;
                                break;
                        default:
                                return(-1);
                        }
                        break;

                case READ_EXTENDED_STEP:
                        rc = tu_test(fdes, SCATU_READ_EXTENDED);
			db(10, 1, "\tREAD_EXTENDED rc", rc);

                        switch(rc) {
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                                step = TERMINATE_STEP;
                                break;
                        case SCATU_CHECK_CONDITION:
                                step = REQUEST_SENSE_STEP;
                                break;
                        case SCATU_GOOD:
                                curblock += 128;
                                step = CHECK_COMPLETION_STEP;
                                break;
                        default:
                                return(-1);
                        }
                        break;

                case CHECK_COMPLETION_STEP:
			db(10,1,"\tcurblock", curblock);

                        if (curblock > maxblock){
                        /* done certifying */
                                if (damode & DA_CONSOLE_TRUE)
                                        chk_asl_status(disp_menu
                                         (PLEASE_STAND_BY,
                                         failing_function_code));
                                step = LAST_STEP;
                                drive_info.percent_complete = 100;
                        } else {
                                step = READ_EXTENDED_STEP;
                                drive_info.percent_complete = (100L * curblock)
                                                /maxblock;
                                if (drive_info.percent_complete != old_percent){
                                        if (damode & DA_CONSOLE_TRUE)
                                            chk_asl_status(disp_menu
                                                 (PLEASE_STAND_BY,
                                                 failing_function_code));
                                        old_percent =
                                                 drive_info.percent_complete;
                                }
                        }
                        break;

                case REQUEST_SENSE_STEP:
                        rc = tu_test(fdes, SCATU_REQUEST_SENSE);
			db(10, 1, "\tREQUEST SENSE rc", rc);

                        switch(rc) {
                        case SCATU_TIMEOUT:
                        case SCATU_BUSY:
                        case SCATU_RESERVATION_CONFLICT:
                        case SCATU_CHECK_CONDITION:
                                step = TERMINATE_STEP;
                                break;
                        case SCATU_GOOD:
                                step = PROCESS_SENSE_DATA_STEP;
                                break;
                        default:
                                return(-1);
                        }
                        break;

                case TERMINATE_STEP: /* Cannot certify drive */
                        if ((damode & DA_CONSOLE_TRUE) && (reformat == FALSE))
                                chk_asl_status(disp_menu(CERTIFY_TERMINATED));
                        return(-2);

                case PROCESS_SENSE_DATA_STEP:
			db(16, 2, "\tsense_key", tucb.scsiret.sense_key,
			   "\tsense_code", tucb.scsiret.sense_code);
			db(30, tu_buffer, 32);

                        if ((tucb.scsiret.sense_key & 0x0f) == 0x06)
                                /* If already going through this twice, exit */
                                if (stepcount[MODE_SELECT_STEP] > 2)
                                        step=TERMINATE_STEP;
                                else
                                        step=MODE_SELECT_STEP;
                        else {
                        /* Recovery count is set to 4 because no Reassign */
                        /* is needed here.                                */
                                process_sense_data(
                                     tucb.scsiret.sense_key & 0x0f,
                                     tucb.scsiret.sense_code, (short)4,
                                     &reassign, &error_type);
                        /* Update count of errors based on error type */
                                switch(error_type){
                                case RECOVERED_EQUIP_ERROR:
                                        ++drive_info.rec_equ_check_errors;
                                        break;
                                case RECOVERED_DATA_ERROR:
                                        ++drive_info.rec_data_errors;
                                        break;
                                case UNRECOVERED_EQUIP_ERROR:
                                        ++drive_info.unrec_equ_check_errors;
                                        break;
                                case UNRECOVERED_DATA_ERROR:
                                        ++drive_info.unrec_data_errors;
                                        break;
                                default:
                                        break;
                                }
			db(10, 5, "\terror_type", error_type,
			   "\tsoft_equ_check", drive_info.rec_equ_check_errors,
			   "\trec_data_errors", drive_info.rec_data_errors,
			   "\tunrec_equ_check_errors", drive_info.unrec_equ_check_errors,
			   "\tunrec_data_errors", drive_info.unrec_data_errors);
                        }

                        /* If unit attention condition, do mode select */
                        if (step != PROCESS_SENSE_DATA_STEP)
                                break;

			/* Check the valid bit */
			if ((tu_buffer[0] & 0x80)) {
				error_block = ((uint) tu_buffer[3] << 24) +
					      ((uint) tu_buffer[4] << 16) +
					      ((uint) tu_buffer[5] <<  8) +
					      ((uint) tu_buffer[6]);
				curblock = error_block + 1;
			}
			else
				curblock += 128;

                        if ((drive_info.unrec_equ_check_errors > 0) ||
                                (drive_info.rec_equ_check_errors > 1) ||
                                (drive_info.unrec_data_errors > 10))
                                step = LAST_STEP;
                        else
                                step = CHECK_COMPLETION_STEP;
                        break;

                case LAST_STEP:
			db(25, "Last Step");

                        if (damode & DA_CONSOLE_TRUE)
                                chk_asl_status(disp_menu(
                                   CERTIFY_COMPLETED, failing_function_code));
                        if ((drive_info.unrec_equ_check_errors > 0) ||
                            (drive_info.rec_equ_check_errors > 1) ||
                            (drive_info.unrec_data_errors > 10)) {
                                return(-2);
                                }
                        if (drive_info.unrec_data_errors > 0) {
                                if (damode & DA_CONSOLE_TRUE)
                                        chk_asl_status(
                                                disp_menu(CERTIFY_COMPLETED_WARNING
                                                , failing_function_code));
                                }
                        step = 0; /* exit loop */
                        break;
                default:
                        return(-1);
                }
        } /* end while */
        return(0);
} /* end certify_disk */
/*  */
/*
 * NAME: display_title
 *
 * FUNCTION: Display the title of the menu depending on the diagnostic
 *           mode chosen by the user.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: none
 *
 * RETURNS: none.
 */

void
display_title()
{
        void    chk_asl_status();
        long    mnum;
        char    *sub[3];

        if (damode & DA_CONSOLE_TRUE){
                sub[0] = (char *)diag_cat_gets(catd, DHF_SET1, SCSI_DISK_DRIVE);
                sub[1] = tm_input.dname;
                sub[2] = tm_input.dnameloc;

       /* Show start test menu */
                if ((damode & DA_LOOPMODE_NOTLM) &&
                        (damode & DA_ADVANCED_FALSE)){
                        mnum = (failing_function_code * 0x1000) + 1;
                        chk_asl_status(diag_display_menu(CUSTOMER_TESTING_MENU,
                                mnum, sub, 0, 0));
                }
      /* Show start test menu - entering loop mode */
                if ((damode & DA_ADVANCED_TRUE) &&
                        (damode & DA_LOOPMODE_NOTLM)){
                        mnum = (failing_function_code * 0x1000) + 2;
                        chk_asl_status(diag_display_menu(ADVANCED_TESTING_MENU,
                                mnum, sub, 0, 0));
                }
       /* show status screen for loop mode */
                if ((damode & DA_ADVANCED_TRUE) &&
                        ((damode & DA_LOOPMODE_ENTERLM) ||
                        (damode & DA_LOOPMODE_INLM))){
                        mnum = (failing_function_code * 0x1000) + 3;
                        chk_asl_status(diag_display_menu(LOOPMODE_TESTING_MENU,
                                mnum, sub, tm_input.lcount, tm_input.lerrors));
                }
        }
} /* end display_title */
/*  */
/*
 * NAME: diag_tst()
 *
 * FUNCTION: Test the SCSI fixed-disk as outlined in the SCSI Fixed-
 *           Disk Problem Determination Guide. The function proceeds
 *           step by step calling it's respective test unit. It examines
 *           the return code from the TU and makes a decision on which
 *           step to do next. If an error occurres it sets the return
 *           code for the controller. If a problem is found during any
 *           of the steps, it calls the create_frub  procedure to set
 *           up the FRU bucket to be passed back to the controller. This is a
 *           very long function. It closely follows section 8.2.2 of the
 *           Problem Determination Guide.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: The return values are set up in each function called.
 *          DA_EXIT() is called to return the codes to the
 *          controller.
 */

#define NUM_STEPS               12   /* Max number of diagnostic steps to do */
#define EXIT_STEP               0
#define RESERVE_UNIT            1
#define TEST_UNIT_READY         2
#define MODE_SENSE              3
#define MODE_SELECT             4
#define SEND_DIAGNOSTICS        5
#define RELEASE_UNIT            6
#define REQUEST_SENSE           7
#define ANALYZE_SENSE_DATA      8
#define START_STOP_UNIT         9
#define DEVICE_RESET            10
#define REPORT_FRUB             11
#define CERTIFY_UNIT            12

void diag_tst(dt_fdes)
int dt_fdes;
{

        char    buf[3];
        int     rc;                                    /* tu_test return code */
        int     disp_rc;                    /* return from disp_menu function */
        int     current_step;
        int     stepcount[NUM_STEPS+1];         /* counter */
        int     found_error = 0;            /* Flag used to indicate error    */
        int     i;                                /* temp....remove for debug */
        int     step;                               /* current step executing */
        extern  int     insert_frub();              /* add data in FRU bucket */
        extern  int     addfrub();                   /* add FRU bucket for DC */
        extern SCSI_TUTYPE tucb;               /* structure for the scsi cmds */

	db(25,"In diag_tst()");
                                      /* clear the accumulative step counters */
        for(step = 0; step <= NUM_STEPS; ++step)
                stepcount[ step ] = 0;

        rc = 0;
        step = RESERVE_UNIT;
        while(step)                         /* Step = 0 gets us out of here */
        {                                                    /* run the tests */
                switch(step) {
                case RESERVE_UNIT :
                        ++stepcount[RESERVE_UNIT];
                        rc = tu_test(dt_fdes, SCATU_RESERVE_UNIT);
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                step = 0; /* exit loop */
                                break;
                        case SCATU_TIMEOUT :
                                if (stepcount[RESERVE_UNIT] == 1) {
                                    DA_SETRC_MORE(DA_MORE_CONT);
                                } else {
                                    create_frub (SCATU_TIMEOUT);
                                    step = REPORT_FRUB;
                                }
                                break;
                        case SCATU_BUSY :
                                step = DEVICE_RESET;
                                break;
                        case SCATU_RESERVATION_CONFLICT :
                                create_frub (SCATU_RESERVATION_CONFLICT);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_CHECK_CONDITION :
                                current_step = RESERVE_UNIT;
                                step = REQUEST_SENSE;
                                break;
                        case SCATU_GOOD :
                                step = TEST_UNIT_READY;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        } /* endswitch */
                        break;
                case TEST_UNIT_READY :
                        ++stepcount[TEST_UNIT_READY];
                        rc = tu_test(dt_fdes, SCATU_TEST_UNIT_READY);
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_CHECK_CONDITION :
                                current_step = TEST_UNIT_READY;
                                step = REQUEST_SENSE;
                                break;
                        case SCATU_GOOD :
                                step = MODE_SENSE;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case MODE_SENSE :
                        /* Should only do this step once to detect drives */
                        /* being formatted with block size other than 512 */
                        ++stepcount[MODE_SENSE];
                        if (stepcount[MODE_SENSE] == 1){
                                page_code=0x3f;
                                rc = tu_test(dt_fdes, SCATU_MODE_SENSE);
                                switch(rc) {
                                case SCATU_BAD_PARAMETER :
                                        step = EXIT_STEP;
                                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        break;
                                case SCATU_TIMEOUT :
                                case SCATU_BUSY :
                                case SCATU_RESERVATION_CONFLICT :
                                case SCATU_CHECK_CONDITION :
                                        create_frub (rc);
                                        step = REPORT_FRUB;
                                        break;
                                case SCATU_GOOD :
                                        /* if block size not 512K do a format */
                                        if ((tu_buffer[10] != 0x02)  ||
                                            (tu_buffer[11] != 0x00)) {
                                                reformat = TRUE;
					}
                                        step = MODE_SELECT;
                                        break;
                                default :
                                         step = EXIT_STEP;
                                         DA_SETRC_ERROR(DA_ERROR_OTHER);
                                         break;
                                }
                        } else
                                step = MODE_SELECT;
                        break;
                    case MODE_SELECT :
                        ++stepcount[MODE_SELECT];
                        rc = tu_test(dt_fdes, SCATU_MODE_SELECT);
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_CHECK_CONDITION :
                                current_step = MODE_SELECT;
                                step = REQUEST_SENSE;
                                break;
                        case SCATU_GOOD :
                                step = SEND_DIAGNOSTICS;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case SEND_DIAGNOSTICS :
                        ++stepcount[SEND_DIAGNOSTICS];
                        rc = tu_test(dt_fdes, SCATU_SEND_DIAGNOSTIC);
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_CHECK_CONDITION :
                                current_step = SEND_DIAGNOSTICS;
                                step = REQUEST_SENSE;
                                break;
                        case SCATU_GOOD :
                                step = CERTIFY_UNIT;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case RELEASE_UNIT :
                        ++stepcount[RELEASE_UNIT];
                        rc = tu_test(dt_fdes, SCATU_RELEASE_UNIT);
        /* If an error has been found previously, do not care about */
        /* the result of the release command. That way we will not  */
        /* overwrite any error previously reported in frub.         */
                        if (found_error) {
                                step = EXIT_STEP;
                                break;
                        }
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                        case SCATU_CHECK_CONDITION :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_GOOD :
                                step = EXIT_STEP;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case DEVICE_RESET :
                        ++stepcount[DEVICE_RESET];
                        close(dt_fdes);
                                  /* Reopen device, sending SCSI device reset */
                        dt_fdes = openx(devname, O_RDWR,
                                         NULL, SC_FORCED_OPEN);
                                 /* if open ok, close and reopen in diag mode */
                        if (dt_fdes < 0) {
                                DA_SETRC_ERROR(DA_ERROR_OPEN);
                                step = EXIT_STEP;
                        } else {
                                close(dt_fdes);
                                dt_fdes = openx(devname,
                                           O_RDWR, NULL, SC_DIAGNOSTIC);
                        }
                        /*
                          Reopen device, in diagnostic mode and test if open
                        */
                        if (dt_fdes < 0) {
                                DA_SETRC_ERROR(DA_ERROR_OPEN);
                                step = EXIT_STEP;
                        } else {
                                step = RESERVE_UNIT;
                        }
                        break;
                case REQUEST_SENSE :
                        ++stepcount[REQUEST_SENSE];
                        rc = tu_test(dt_fdes, SCATU_REQUEST_SENSE);
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                        case SCATU_CHECK_CONDITION :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_GOOD :
                                step = ANALYZE_SENSE_DATA;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case ANALYZE_SENSE_DATA :
                        ++stepcount[ANALYZE_SENSE_DATA];
                        step = analyze_sense_data(current_step, stepcount
                                        [ANALYZE_SENSE_DATA]);
                        break;
                case START_STOP_UNIT :
                        ++stepcount[START_STOP_UNIT];
                        if (damode & DA_CONSOLE_TRUE)
                                chk_asl_status(disp_menu (START_MOTOR, failing_function_code));
                        rc = tu_test(dt_fdes, SCATU_START_STOP_UNIT);
                        sleep (30);
                        display_title();
                        switch(rc) {
                        case SCATU_BAD_PARAMETER :
                               step = EXIT_STEP;
                               DA_SETRC_ERROR(DA_ERROR_OTHER);
                               break;
                        case SCATU_TIMEOUT :
                        case SCATU_BUSY :
                        case SCATU_RESERVATION_CONFLICT :
                                create_frub (rc);
                                step = REPORT_FRUB;
                                break;
                        case SCATU_CHECK_CONDITION :
                                current_step = START_STOP_UNIT;
                                step = REQUEST_SENSE;
                                break;
                        case SCATU_GOOD :
                                step = TEST_UNIT_READY;
                                break;
                        default :
                                step = EXIT_STEP;
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                break;
                        }
                        break;
                case REPORT_FRUB :
                        ++stepcount[REPORT_FRUB];
                                 /* We got here on error, try to do a release */
                                 /* the first time, but if we get here due to */
                                 /* failed release cmd, just get out.         */
                        if (stepcount[REPORT_FRUB] == 1) {
                                step = RELEASE_UNIT;
                                found_error = 1;  /* indicate error found */
                        } else {
                                step = EXIT_STEP;
                        }
                        /*
                          set return code bad to display FRU bucket
                          then set up the fru structure. If the error
                          shows that the drive is 100% at fault we
                          will do case 1(conf_level = 1).
                        */
                        DA_SETRC_STATUS(DA_STATUS_BAD);
                        switch(conf_level) {
                        case 0:
                        case 2:
                                DA_SETRC_MORE(DA_MORE_CONT);
                                break;
                        default:
                                break;
                        }
                        if (insert_frub(&tm_input, &frub[conf_level]) != 0) {
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                clean_up();
                        }
                        if (addfrub(&frub[conf_level]) != 0) {
                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                clean_up();
                        }
                        break;
                case CERTIFY_UNIT :
                        if ((! tm_input.system) && (damode & DA_CONSOLE_TRUE) &&
                             ((damode & DA_LOOPMODE_NOTLM) ||
                             (damode & DA_LOOPMODE_EXITLM))) {
                                ++stepcount[CERTIFY_UNIT];
                                 chk_asl_status(disp_menu(DO_WE_CERTIFY2,
                                         failing_function_code));
                                 if (menu_return != 2) {
                                        rc=certify_disk(dt_fdes);
                                        if (rc == -1)
                                           DA_SETRC_ERROR(DA_ERROR_OTHER);
                                        else if (rc == -2) {
                                           DA_SETRC_STATUS(DA_STATUS_BAD);
                                           DA_SETRC_TESTS(DA_TEST_FULL);
                                           create_frub(DEVICE_CERTIFY_FAIL);
                                           if (insert_frub(&tm_input,&frub[conf_level]) != 0) {
                                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                                clean_up();
                                           }
                                           if (addfrub(&frub[conf_level]) !=0) {
                                                DA_SETRC_ERROR(DA_ERROR_OTHER);
                                                clean_up;
                                           }
                                        }
                                }
                        }
                        step = RELEASE_UNIT;
                        break;
                default:
                    step = EXIT_STEP;
                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                    break;
                }
        } /* endwhile */
}  /* end of function diag_tst */
/*  */
/*
 * NAME: analyze_sense_data()
 *
 * FUNCTION: Given the sense data from the request sense command, analyze
 *           it and decide whether or not to create a frub, and also
 *           figure out the next test step to be performed.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Int value indicating the next step to be performed.
 */
int
analyze_sense_data(current_step, retry_count)
int     current_step;
int     retry_count;
{
        extern SCSI_TUTYPE tucb;               /* structure for the scsi cmds */
        int    step = EXIT_STEP;

        /* If command has been sent twice, then report FRUB */

        if ((retry_count > 2) && (tucb.scsiret.sense_key != 0x00) &&
                        (tucb.scsiret.sense_key != 0x01)){
                create_frub(ATU_ERROR);
                step = REPORT_FRUB;
                return(step);
        }
        switch(tucb.scsiret.sense_key) {
        case 0x0  :
        case 0x01 :
              /* Handle special case of 0x5d00 sense code */
              if (tucb.scsiret.sense_code == 0x5d00) {
                        create_frub(ATU_ERROR);
                        step = REPORT_FRUB;
              } else {

                      switch(current_step){
                      case RESERVE_UNIT :
                                step = TEST_UNIT_READY;
                                break;
                      case START_STOP_UNIT :
                      case TEST_UNIT_READY :
                                step = MODE_SENSE;;
                                break;
                      case MODE_SELECT :
                                 step = SEND_DIAGNOSTICS;
                                 break;
                      case  SEND_DIAGNOSTICS :
                                 step = CERTIFY_UNIT;
                                 break;
                      default :
                                 DA_SETRC_ERROR(DA_ERROR_OTHER);
                                 step = EXIT_STEP;
                                 break;
                       }
               }
               break;
        case 0x02 :
              switch(tucb.scsiret.sense_code) {
                case 0x0400 :
                case 0x0401 :
                case 0x0402 :
                     step = START_STOP_UNIT;
                     break;
                case 0x40b0 :
                case 0x40c0 :
                     if (retry_count == 1)
                              step = START_STOP_UNIT;
                     else {
                              create_frub (ATU_ERROR);
                              step = REPORT_FRUB;
                     }
                     break;
                case 0x3101:
                     if (retry_count == 1)
                              step = RESERVE_UNIT;
                     else {
                              create_frub (ATU_ERROR);
                              step = REPORT_FRUB;
                     }
                     break;
                default :
                     if (((tucb.scsiret.sense_code & 0x3100) == 0x3100)
                      || (tucb.scsiret.sense_code == 0x0404))
                        if (damode & DA_CONSOLE_TRUE)
                                chk_asl_status(disp_menu(BAD_FORMAT_MENUGOAL, 
						failing_function_code));
                     create_frub (ATU_ERROR);
                     step = REPORT_FRUB;
                     break;
              }
              break;
        case 0x03 :
              if (((tucb.scsiret.sense_code & 0x3100) == 0x3100)
                || (tucb.scsiret.sense_code == 0x3000))
                      if (retry_count == 1) {
                        step = SEND_DIAGNOSTICS;
                        break;
                        }
              if (retry_count == 1) {
                        step = RESERVE_UNIT;
                        break;
                        }
              if (damode & DA_CONSOLE_TRUE)
                chk_asl_status(disp_menu(
                     BAD_FORMAT_MENUGOAL, failing_function_code));
              create_frub (ATU_ERROR);
              step = REPORT_FRUB;
              break;
         case 0x04 :
         case 0x05 :
         case 0x0b :
               if (retry_count == 1)
                        step = RESERVE_UNIT;
               else {
                        create_frub (ATU_ERROR);
                        step = REPORT_FRUB;
               }
               break;
         case 0x06 :
               if (current_step != SEND_DIAGNOSTICS)
                       if (retry_count == 1)
                             step = RESERVE_UNIT;
                        else {
                                create_frub (ATU_ERROR);
                                step = REPORT_FRUB;
                        }
               else
                        if (tucb.scsiret.sense_code == 0x2900)
                              if (retry_count == 1)
                                        step = RESERVE_UNIT;
                              else {
                                        create_frub (ATU_ERROR);
                                        step = REPORT_FRUB;
                              }
                        else
                              if ((damode & DA_DMODE_REPAIR) &&
                                   ((damode & DA_LOOPMODE_NOTLM) ||
                                    (damode & DA_LOOPMODE_EXITLM)))
                                       step = CERTIFY_UNIT;
                              else
                                       step = RELEASE_UNIT;
               break;
         case 0x07 :
               if (retry_count < 3)
                        step = RESERVE_UNIT;
               else {
                      if (damode & DA_CONSOLE_TRUE){
                            chk_asl_status(disp_menu(
                                       WRITE_PROTECT,failing_function_code));
                        /* User indicates drive is write protected, exit */
                            if (menu_return == YES)
                                    step = RELEASE_UNIT;
                            else {
                                    create_frub (ATU_ERROR);
                                    step = REPORT_FRUB;
                            }
                       } else {
                               create_frub (ATU_ERROR);
                               step = REPORT_FRUB;
                       }
               }
               break;
         case 0x0e :
               if (retry_count < 3)
                          step = RESERVE_UNIT;     /* try again to be sure */
               else {
                          create_frub (ATU_ERROR);
                          step = REPORT_FRUB;
               }
               break;
         default :
               create_frub (ATU_ERROR);
               step = REPORT_FRUB;
               break;
         }
         return(step);
}
/*  */
/*
 * NAME: tu_test()
 *
 * FUNCTION: tu_test sets some parameters to be used in the execution
 *           of each test unit. It then calls the exectu function to
 *           actually perform the test.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the return code from the test unit
 *          called by the exectu function.
 */

tu_test(tu_fdes, tnum)
int   tu_fdes;
int   tnum;
{
        uint      blocks;
        char     buf[3];
        char     *real_test;
        int      step;
        int      rc;                             /* function return code      */
        register int i;
        extern   SCSI_TUTYPE tucb;              /* scsi command structure     */
        static uchar    changeable_mode_data[255];
        static uchar    desired_mode_data[255];
	static uchar    current_mode_data[255];
        int      sense_length;
        int      mode_data_length;

        tucb.header.tu = tnum;            /* put TU number to do in TU struct */
        tucb.header.mfg = 0;             /* always set to 0, only used by mfg */
        rc = scsitu_init (&tucb);
        if (rc != SCATU_GOOD) {
                return (SCATU_BAD_PARAMETER);
        }
        tucb.scsitu.flags = 0;
        tucb.scsitu.ioctl_pass_param = PASS_PARAM;
        tucb.scsitu.data_length = 0;
        tucb.scsitu.data_buffer = NULL;
        tucb.scsitu.cmd_timeout = 30;
        memset(tu_buffer, 0x00, 1024);
	if (lun_id < 8)
        	tucb.scsitu.scsi_cmd_blk[1] |= lun_id << 5;
	else
		tucb.scsitu.lun = lun_id;
        /*
          Based on TU to do, set up other parameters
        */
        switch(tnum) {
        case SCATU_TEST_UNIT_READY  :
                tucb.scsitu.cmd_timeout = 60;
                break;
        case SCATU_START_STOP_UNIT  :
                tucb.scsitu.cmd_timeout = start_stop_to;
                tucb.scsitu.scsi_cmd_blk[1] |= 0 << 0;
                break;
        case SCATU_MODE_SELECT  :
                /* If OEM drive, then get the changeable mode data from     */
                /* the drive and compare it against the desired mode data   */
                /* from the ODM data base, then only set the bits that are  */
                /* changeable.                                              */
		
		if (isSCSD && (odm_info.mode_data_length == 0)) {
			/* If SCSD and mode_data_length is zero */
			/* then don't issue mode select*/
			return(SCATU_GOOD);
		}
                if (osdisk){
                        rc = get_sense_data(tu_fdes,
                                changeable_mode_data, 
				desired_mode_data,
                                &mode_data_length);
                        if (rc != 0) {
                                return(rc);
			}
                        set_default_sense(desired_mode_data, 
					  mode_data_length,
					  current_mode_data,
                                	  changeable_mode_data, 
					  tu_buffer, 
					  &sense_length);
                        /* get_sense_data calls this function again. */
			/* tucb is global so initialize again.       */
			tucb.header.tu = SCATU_MODE_SELECT;
			rc = scsitu_init ( &tucb );
			if( rc != SCATU_GOOD ) {
				return ( SCATU_BAD_PARAMETER );
			}
			tucb.scsitu.ioctl_pass_param = PASS_PARAM;
                } else {
                	/* Use mode select from ODM data base (PDiagAtt). */
                        bcopy(odm_info.mode_data, tu_buffer,
                                 odm_info.mode_data_length);
                        sense_length = odm_info.mode_data_length;
                }
		tucb.scsitu.cmd_timeout = 30;
		tucb.scsitu.flags |= B_WRITE;
		tucb.scsitu.scsi_cmd_blk[1] |= 0x10;
                tucb.scsitu.scsi_cmd_blk[4]  = sense_length;
                tucb.scsitu.data_length = sense_length;
                tucb.scsitu.data_buffer = tu_buffer;    /* Put data in buff */
                break;
        case SCATU_SEND_DIAGNOSTIC  :
                tucb.scsitu.cmd_timeout = send_diag_to;
                tucb.scsitu.flags |= B_WRITE;
                if (odm_info.send_diag[0] != '\0')
                        bcopy(odm_info.send_diag,tucb.scsitu.scsi_cmd_blk+1,5);
                else{
                        tucb.scsitu.scsi_cmd_blk[1] |= 1 << 2; /* Self test  */
                        tucb.scsitu.scsi_cmd_blk[1] |= 0 << 1; /* Dev0fl bit */
                        tucb.scsitu.scsi_cmd_blk[1] |= 0 << 0; /* UnitOfl    */
                }
                break;
        case SCATU_REQUEST_SENSE    :
                tucb.scsitu.cmd_timeout = 30;
                tucb.scsitu.flags |= B_READ;
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
                break;
        case SCATU_RESERVE_UNIT     :
                tucb.scsitu.cmd_timeout = 30;
                break;
        case SCATU_MODE_SENSE    :
                tucb.scsitu.cmd_timeout = 30;
                tucb.scsitu.flags |= B_READ;
                tucb.scsitu.scsi_cmd_blk[2] = page_code;
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
                break;
        case SCATU_RELEASE_UNIT     :
                tucb.scsitu.cmd_timeout = 30;
                break;
        case SCATU_READ_CAPACITY:
                tucb.scsitu.data_length = 8;
                tucb.scsitu.data_buffer = tu_buffer;
                tucb.scsitu.flags |= B_READ;
                break;
        case SCATU_READ_EXTENDED:
                tucb.scsitu.cmd_timeout = readx_to;
                tucb.scsitu.flags |= B_READ;
                blocks = (maxblock - curblock) + 1;
                if (blocks > 128)
                        blocks = 128;
                tucb.scsitu.data_length = blocks * 512;
                tucb.scsitu.scsi_cmd_blk[2] = (uchar) (curblock >> 24);
                tucb.scsitu.scsi_cmd_blk[3] = (uchar) (curblock >> 16);
                tucb.scsitu.scsi_cmd_blk[4] = (uchar) (curblock >> 8);
                tucb.scsitu.scsi_cmd_blk[5] = (uchar) (curblock);
                tucb.scsitu.scsi_cmd_blk[7] = (uchar) (blocks >> 8);
                tucb.scsitu.scsi_cmd_blk[8] = (uchar) (blocks);
                tucb.scsitu.data_buffer = tu_buffer;
                break;
        default :
                return (SCATU_BAD_PARAMETER);
        }
        chk_terminate_key();
	/* Print the Data In from device. */
	/* db(2,&tucb); */
        rc=exectu(tu_fdes, &tucb);
	/* db(4,&tucb);	*/
	/* db(10,1,"exectu() rc",rc); */
	if (rc == SCATU_GOOD) {
		/*
		if (tucb.header.tu == SCATU_REQUEST_SENSE) {
			db(3,&tucb);
		} else {
			db(4,&tucb);
		}
		*/
        	if (osdisk && 
            	    (tnum == SCATU_MODE_SENSE) && (page_code == 0x3f)) {
                	/* Save the devices current mode data. */
                	bcopy(tucb.scsitu.data_buffer, current_mode_data,
                       	      (unsigned)tucb.scsitu.data_buffer[0]+1);
        	}
	}
        chk_terminate_key();
        return(rc);
}  /*  end of function tu_test */
/*  */
/*
 * NAME: create_frub ()
 *
 * FUNCTION: Will be called if the testing of the device indicates a
 *           problem. It sets up the FRU buckets based on what device
 *           is being tested. It will get the device type from the ODM.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

create_frub (scsi_err)

        int scsi_err;
{
        int    report_failure_percent = TRUE;
        extern SCSI_TUTYPE tucb;               /* structure for the scsi cmds */

        /* If need to use the Subsystem Diagnostic Procedure then generate */
        /* special SRN.                                                    */
        if (odm_info.use_subsystem_diag){
                conf_level = 1;
                frub[conf_level].rcode = SUBSYSTEM_SERVICE;
                frub[conf_level].rmsg = DHF_DA_MSG_274;
                problem_found=TRUE;
                return;
        }

        conf_level = (odm_info.de_card_fru) ? 0 : 1;
        switch(scsi_err) {
        case ADAPTER_CONFIG_FAIL:
                frub[3].rcode = ADAPTER_CONFIG_FAIL;
                frub[3].sn = failing_function_code;
                strcpy(frub[3].frus[0].floc, tm_input.parentloc);
                frub[3].frus[0].fru_flag = PARENT_NAME;
                frub[3].frus[0].fmsg = 0;
                frub[3].frus[0].fname[0] = '\0';
                frub[3].frus[0].fname[1] = '\0';
                return;
        case DEVICE_CONFIG_FAIL:
                frub[4].sn = failing_function_code;
                frub[4].rcode = DEVICE_CONFIG_FAIL;
                strcpy(frub[4].frus[0].floc, tm_input.dnameloc);
                frub[4].frus[0].fru_flag = DA_NAME;
                frub[4].frus[0].fmsg = 0;
                frub[4].frus[0].fname[0] = '\0';
                frub[4].frus[0].fname[1] = '\0';
                frub[4].frus[1].fru_flag = PARENT_NAME;
                frub[4].frus[1].fmsg = 0;
                frub[4].frus[1].fname[0] = '\0';
                frub[4].frus[1].fname[1] = '\0';
                return;
        case ATU_ERROR:
                switch(tucb.scsiret.sense_key) {
                case 0x01 :
                        if (tucb.scsiret.sense_code == 0x5d00) {
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                        } else
                                return; /* Ignore all other recovered error */
                        break;
                case 0x02 :                  /* Sense Key indicates Not Ready */
                                                  /* test for 40xx sense code */
                        switch(tucb.scsiret.sense_code) {
                        case 0x0300 :
                        case 0x4080 :
                        case 0x40b0 :
                        case 0x40c0 :
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                                break;
                        case 0x0400 :
                        case 0x0401 :
                        case 0x0402 :
                                frub[conf_level].rcode = DRIVE_NREADY;
                                frub[conf_level].rmsg = DHF_DA_MSG_269;
                                break;
                        case 0x0403 :
                                frub[conf_level].rcode = MOTOR_FAILURE;
                                frub[conf_level].rmsg = DHF_DA_MSG_256;
                                break;
                        case 0x0485 :
                                conf_level = 1;
                                frub[conf_level].rcode = BUS_FAILURE;
                                frub[conf_level].rmsg = DHF_DA_MSG_258;
                                break;
                        case 0x0500 :
                        case 0x0800 :
                        case 0x4c00 :
                        case 0x4400 :
                                if (odm_info.de_card_fru){
                                      frub[conf_level].rcode = CARD_FAILED;
                                      frub[conf_level].rmsg = DHF_DA_MSG_257;
                                } else {
                                      frub[conf_level].rcode = HARDWARE_ERROR;
                                      frub[conf_level].rmsg = DHF_DA_MSG_261;
                                }
                                break;
                        case 0x2200 :
                                report_failure_percent = FALSE;
                                frub[conf_level].rcode = PROTOCOL_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_262;
                                break;
                        default :
                                if ((hex4000 & tucb.scsiret.sense_code)
                                         == hex4000){
                                        frub[conf_level].rcode = DIAG_FAILURE;
                                        frub[conf_level].rmsg = DHF_DA_MSG_260;
                                } else if (((hex3100 & tucb.scsiret.sense_code)
                                         == hex3100)
                                        || (tucb.scsiret.sense_code == 0x404)){
                                        frub[conf_level].rcode = FORMAT_BAD;
                                        frub[conf_level].rmsg = DHF_DA_MSG_259;
              				if (damode & DA_CONSOLE_TRUE) {
                				chk_asl_status(disp_menu(
                     				BAD_FORMAT_MENUGOAL, 
						failing_function_code));
					}
                                } else {
                                        frub[conf_level].rcode = DEF_ATU_ERROR;
                                        frub[conf_level].rmsg = DHF_DA_MSG_270;
                                }
                                break;
                        } /* endswitch */
                        break;
                case 0x03 :               /* Sense Key indicates Media Error */
                                           /* test for 14xx & 19xx sense code */
                        switch (tucb.scsiret.sense_code) {
                        case 0x0200   :
                        case 0x0300   :
                        case 0x1500   :
                        case 0x4400   :
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                                break;
                        case 0x0c02   :
                        case 0x1000   :
                        case 0x1100   :
                        case 0x1104   :
                        case 0x110b   :
                        case 0x1200   :
                        case 0x1300   :
                        case 0x1400   :
                        case 0x1401   :
                        case 0x1405   :
                        case 0x1600   :
                        case 0x1604   :
                        case 0x1c00   :
                        case 0x1d00   :
                                frub[conf_level].rcode = NR_MEDIUM_ERR;
                                frub[conf_level].rmsg = DHF_DA_MSG_255;
                                break;
                        case 0x3000 :
                        case 0x3100 :
                                frub[conf_level].rcode = FORMAT_BAD;
                                frub[conf_level].rmsg = DHF_DA_MSG_259;
              			if (damode & DA_CONSOLE_TRUE) {
                			chk_asl_status(disp_menu(
                     			BAD_FORMAT_MENUGOAL, 
					failing_function_code));
				}
                                break;
                        default :
                                if (((hex1400 & tucb.scsiret.sense_code)
                                            == hex1400) ||
                                            ((hex1900 & tucb.scsiret.sense_code)
                                             == hex1900)) {
                                      frub[conf_level].rcode = NR_MEDIUM_ERR;
                                      frub[conf_level].rmsg = DHF_DA_MSG_255;
                                 } else if ((hex3200 & tucb.scsiret.sense_code)
                                            == hex3200){
                                      frub[conf_level].rcode = HARDWARE_ERROR;
                                      frub[conf_level].rmsg = DHF_DA_MSG_261;
                                 } else {
                                      frub[conf_level].rcode = DEF_ATU_ERROR;
                                      frub[conf_level].rmsg = DHF_DA_MSG_270;
                                 }
                                 break;
                        }
                        break;
                case 0x04 :             /* Sense Key indicates Hardware Error */
                                                  /* test for 40xx sense code */
                        switch (tucb.scsiret.sense_code) {
                        case 0x0100   :
                        case 0x0200   :
                        case 0x0300   :
                        case 0x0400   :
                        case 0x0500   :
                        case 0x0600   :
                        case 0x0900   :
                        case 0x1000   :
                        case 0x1100   :
                        case 0x1200   :
                        case 0x1400   :
                        case 0x1401   :
                        case 0x1600   :
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                                break;
                        case 0x1b00   :
                        case 0x0800   :
                        case 0x4700   :
                                /*
                                  For 857mb this is considered HW
                                  and 100 % probability.
                                */
                                conf_level = 1;
                                frub[conf_level].rcode = BUS_FAILURE;
                                frub[conf_level].rmsg = DHF_DA_MSG_258;
                                break;
                        case 0x3100   :
                                frub[conf_level].rcode = FORMAT_BAD;
                                frub[conf_level].rmsg = DHF_DA_MSG_259;
              			if (damode & DA_CONSOLE_TRUE) {
                			chk_asl_status(disp_menu(
                     			BAD_FORMAT_MENUGOAL, 
					failing_function_code));
				}
                                break;
                        case 0x3e00   :
                        case 0x4000   :
                        case 0x4400   :
                        case 0x4500   :
                                frub[conf_level].rcode = CARD_FAILED;
                                frub[conf_level].rmsg = DHF_DA_MSG_257;
                                break;
                        case 0x2200   :
                                report_failure_percent = FALSE;
                                frub[conf_level].rcode = PROTOCOL_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_262;
                                break;
                        case 0x4100   :
                                frub[conf_level].rcode = DIAG_FAILURE;
                                frub[conf_level].rmsg = DHF_DA_MSG_260;
                                break;
                        case 0xb900   :
                                if (odm_info.de_card_fru){
                                      frub[conf_level].rcode = CARD_FAILED;
                                      frub[conf_level].rmsg = DHF_DA_MSG_257;
                                } else {
                                      frub[conf_level].rcode = HARDWARE_ERROR;
                                      frub[conf_level].rmsg = DHF_DA_MSG_261;
                                }
                                break;
                        default :
                                if (((hex4000 & tucb.scsiret.sense_code)
                                     == hex4000) ||
                                  ((hex1500 & tucb.scsiret.sense_code)
                                     == hex1500) ||
                                  ((hex3200 & tucb.scsiret.sense_code)
                                     == hex3200)) {
                                     frub[conf_level].rcode = HARDWARE_ERROR;
                                     frub[conf_level].rmsg = DHF_DA_MSG_261;
                                } else {
                                     frub[conf_level].rcode = DEF_ATU_ERROR;
                                     frub[conf_level].rmsg = DHF_DA_MSG_270;
                                }
                                break;
                        }
                        break;
                case 0x05 :            /* Sense Key indicates Illegal Request */
                        switch (tucb.scsiret.sense_code) {
                        case 0x1900   :
                        case 0x1a00   :
                        case 0x2000   :
                        case 0x2100   :
                        case 0x2200   :
                        case 0x2400   :
                        case 0x2500   :
                        case 0x2600   :
                        case 0x2601   :
                        case 0x3d00   :
                        case 0x4900   :
                                report_failure_percent = FALSE;
                                frub[conf_level].rcode = PROTOCOL_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_262;
                                break;
                        default :
                                if (((hex3200 & tucb.scsiret.sense_code)
                                     == hex3200) ||
                                     (tucb.scsiret.sense_code == 0x3900)){
                                        frub[conf_level].rcode = HARDWARE_ERROR;
                                        frub[conf_level].rmsg = DHF_DA_MSG_261;
                                } else {
                                        frub[conf_level].rcode = DEF_ATU_ERROR;
                                        frub[conf_level].rmsg = DHF_DA_MSG_270;
                                }
                                break;
                        }
                        break;
                case 0x06 :              /* Unit Attention */
                        if (tucb.scsiret.sense_code == 0x4200){
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                        } else
                                frub[conf_level].rcode = DEF_ATU_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_270;
                        break;
                case 0x07 :              /* Sense key indicates write protect */
                        switch(tucb.scsiret.sense_code) {
                        case 0x2700 :
                                frub[conf_level].rcode = WRT_PROTECT_ERR;
                                frub[conf_level].rmsg = DHF_DA_MSG_272;
                                break;
                        default :
                                frub[conf_level].rcode = DEF_ATU_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_270;
                                break;
                        }
                        break;
                case 0x0b :            /* Sense Key indicates Aborted Command */
                        switch (tucb.scsiret.sense_code) {
                        case 0x4500   :
                        case 0x4600   :
                        case 0x4700   :
                        case 0x4800   :
                        case 0x4900   :
                        case 0x4e00   :
                                /*
                                  For 857mb this is considered HW
                                  and 100 % probability.
                                */
                                conf_level = 1;
                                frub[conf_level].rcode = BUS_FAILURE;
                                frub[conf_level].rmsg  = DHF_DA_MSG_258;
                                break;
                        case 0x2500   :
                        case 0x4300   :
                                report_failure_percent = FALSE;
                                frub[conf_level].rcode = PROTOCOL_ERROR;
                                frub[conf_level].rmsg  = DHF_DA_MSG_262;
                                break;
                        case 0x4400   :
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                                break;
                        default :
                                frub[conf_level].rcode = DEF_ATU_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_270;
                                break;
                        }
                        break;
                case 0x0e :                 /* Sense key indicates miscompare */
                        switch(tucb.scsiret.sense_code) {
                        case 0x1d00 :
                                frub[conf_level].rcode = HARDWARE_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_261;
                                break;
                        default :
                                frub[conf_level].rcode = DEF_ATU_ERROR;
                                frub[conf_level].rmsg = DHF_DA_MSG_270;
                                break;
                        }
                        break;
                default :
                        conf_level = 1;
                        frub[conf_level].rcode = DEF_ATU_ERROR;
                        frub[conf_level].rmsg = DHF_DA_MSG_270;
                        break;
                }
                break;
        case DEVICE_CERTIFY_FAIL:
                frub[conf_level].rcode = DEVICE_CERTIFY_FAIL;
                frub[conf_level].rmsg  = DHF_DEVICE_CERTIFY_FAIL;
                break;
        case SCATU_TIMEOUT:
                conf_level = 0;
                frub[conf_level].rcode = SCSI_TIMEOUT;
                frub[conf_level].rmsg  = DHF_DA_MSG_263;
                break;
        case SCATU_CHECK_CONDITION:
                frub[conf_level].rcode = SCSI_CHK_COND;
                frub[conf_level].rmsg  = DHF_DA_MSG_266;
                break;
        case SCATU_RESERVATION_CONFLICT:
                conf_level = 1;
                frub[conf_level].rcode = SCSI_RESERVE;
                frub[conf_level].rmsg  = DHF_DA_MSG_265;
                break;
        case SCATU_COMMAND_ERROR:
                frub[conf_level].rcode = SCSI_BUSY;
                frub[conf_level].rmsg  = DHF_DA_MSG_264;
                break;
        case DD_CONFIG:
                conf_level = 0;
                frub[conf_level].rcode = DD_CONFIG;
                frub[conf_level].rmsg  = DHF_DA_MSG_267;
                break;
        case ELA_ERROR1:
                frub[conf_level].rcode = ELA_ERROR1;
                frub[conf_level].rmsg  = DHF_DA_MSG_268;
                break;
        case ELA_ERROR3:
                conf_level = 5;
                frub[5].sn = failing_function_code;
        	problem_found = TRUE; /* signal that a FRU has been reported */
                return;
        default :
                conf_level = 1;
                frub[conf_level].rcode = DEF_ATU_ERROR;
                frub[conf_level].rmsg = DHF_DA_MSG_270;
                break;
        }
        /*
          Set up fru_bucket based on what kind of drive is being tested.
        */
        frub[conf_level].sn = failing_function_code;
        strcpy(frub[conf_level].frus[0].floc, tm_input.dnameloc);
        if (report_failure_percent){
                if (! odm_info.de_card_fru) {
                        frub[conf_level].frus[0].fru_flag = DA_NAME;
                        frub[conf_level].frus[0].fmsg = 0;
                        frub[conf_level].frus[0].fname[0] = '\0';
                        frub[conf_level].frus[0].fname[1] = '\0';
                        if (conf_level != Conf1){
                                frub[conf_level].frus[1].fru_flag = PARENT_NAME;
                                frub[conf_level].frus[1].fmsg = 0;
                                frub[conf_level].frus[1].fname[0] = '\0';
                                frub[conf_level].frus[1].fname[1] = '\0';
                        }
                        if (conf_level == Conf0)
                                if (frub[conf_level].rcode == ELA_ERROR1){
                                    frub[conf_level].frus[0].conf = Conf60;
                                    frub[conf_level].frus[1].conf = Conf40;
                                } else {
                                    frub[conf_level].frus[0].conf = Conf80;
                                    frub[conf_level].frus[1].conf = Conf20;
                                }
                        else
                                frub[conf_level].frus[0].conf  = Conf100;
                } else {
                        if (conf_level == Conf0){
                                strcpy(frub[conf_level].frus[1].floc, tm_input.dnameloc);
                                switch(frub[conf_level].rcode){
                                case SCSI_CHK_COND:
                                case SCSI_BUSY:
                                case SCSI_TIMEOUT:
                                    frub[conf_level].frus[0].conf = Conf80;
                                    frub[conf_level].frus[1].conf = Conf20;
                                    frub[conf_level].frus[1].fru_flag = PARENT_NAME;
                                    frub[conf_level].frus[1].fmsg = 0;
                                    frub[conf_level].frus[1].fname[0] = '\0';
                                    frub[conf_level].frus[1].fname[1] = '\0';
                                    break;
                                case ELA_ERROR1:
                                    frub[conf_level].frus[0].conf = Conf60;
                                    frub[conf_level].frus[1].conf = Conf40;
                                    break;
                                case DD_CONFIG:
                                    frub[conf_level].frus[0].conf = Conf60;
                                    frub[conf_level].frus[1].conf = Conf40;
                                    frub[conf_level].frus[1].fru_flag = PARENT_NAME;
                                    frub[conf_level].frus[1].fmsg = 0;
                                    frub[conf_level].frus[1].fname[0] = '\0';
                                    frub[conf_level].frus[1].fname[1] = '\0';
                                    break;
                                default:
                                    frub[conf_level].frus[0].conf = Conf80;
                                    frub[conf_level].frus[1].conf = Conf20;
                                    break;
                                }
                         } else /* either bucket 2 or 1 */
                              if (conf_level == 2){
                                   strcpy(frub[conf_level].frus[1].floc,
                                         tm_input.dnameloc);
                                   frub[conf_level].frus[0].conf  = Conf50;
                                   frub[conf_level].frus[1].conf  = Conf30;
                                   frub[conf_level].frus[2].conf  = Conf20;
                              } else
                                   frub[conf_level].frus[0].conf = Conf100;
                }
        }
        problem_found = TRUE; /* signal that a FRU has been reported */
        /*
          Tell DC to test parent if not sure we nailed the problem
        */
        if (conf_level != 1)
                DA_SETRC_MORE(DA_MORE_CONT);
} /* endfunction create_frub  */
/*  */
/*
 * NAME: chk_asl_status()
 *
 * FUNCTION: Checks the return code status of the asl commands. Based
 *           on the return code the function sets error codes to be
 *           returned back to the diagnostic controller. If the asl error
 *           is catastrophic the function will call the clean_up function
 *           which will terminate the module and return control to the
 *           diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 *
 */

void
chk_asl_status(asl_rc)
long asl_rc;
{
        switch (asl_rc) {
        case DIAG_ASL_OK:
                break;
        case DIAG_MALLOCFAILED:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_INITSCR:
        case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                clean_up();
                break;
        default:
                break;
        }
} /* endfunction chk_asl_status */
/*  */
/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Checks whether a user has asked to terminate an application
 *           by hitting the Esc or Cancel key.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 *
 */

void chk_terminate_key()
{
        if (damode & DA_CONSOLE_TRUE)
                chk_asl_status(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL, NULL));
} /* endfunction chk_terminate_key */
/*  */
/*
 * NAME: chk_ela()
 *
 * FUNCTION: Scans the error log for entries put in the log by the device
 *           driver. These log entries may indicate either a hardware or
 *           software problem with the SCSI fixed-disks.
 *
 * EXECUTION ENVIRONMENT: Called when the tm_input.dmode is set to ELA.
 *                        This function will also be called if the problem
 *                        determination proc indicates no trouble found.
 *
 * NOTES: At this time criteria for temporary errors have not been set, so
 *        these errors are ignored until thresholds are set by engineering.
 *        Several abreviations will be used in the comments and variables
 *        in this function. Abreviations found are: hre...hard read error;
 *        sre...soft read error; se...soft equipment error;
 *        sk...seek error.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

void chk_ela()
{
        uint     segment_count();
        struct   errdata err_data;                 /* error log data template */
        char     srch_crit[80];                    /* search criteria string  */
        register int      op_flg;           /* tells error_log_get what to do */
        register int      i;                              /* counter variable */
        register int      rc = -99;         /* return code from error_log_get */
        int      err_count[num_errs + 1]; /*keep track of number errids found */
        int      first_soft_err = FALSE;     /* indicate 1st soft error found */
        short    reassign, error_type;
        int      found = FALSE;
        int      sense_key;
        int      sense_code;
        char     insert_fru_bucket = FALSE;
        uint     initcnt = 0;
        uint     nextcnt = 0;
        uint     previous = 0;
        uint      lba;
        uint      *stored_lbas= (uint *)NULL;/* Array used to store lbas */
        uint      total_lba_count = 0;
        struct   error_log_info error_info;

        db(25,"In chk_ela()");
        DA_SETRC_TESTS (DA_TEST_FULL);              /* set type tests to full */
        error_info.totcnt = 0;
        error_info.soft_data_error_found = FALSE;
        error_info.soft_equip_error_found = FALSE;
        sprintf(srch_crit, srchstr, tm_input.date, tm_input.dname);
        stored_lbas=(uint *)malloc(odm_info.max_hard_data_error);
        /* clear the accumulative error counters */
        for(i = 0; i <= num_errs; ++i)
                err_count[ i ] = 0;
        for(i = 0; i < odm_info.max_hard_data_error; i++)
                stored_lbas[ i ] = -1;
        op_flg = INIT;
        do {
                /* get 1st entry in log if any */
                rc = error_log_get(op_flg, srch_crit, &err_data);
                if (rc < 0) {
                        op_flg = TERMI;
                        rc = error_log_get(op_flg, srch_crit, &err_data);
                        clean_up();
                }
                if (rc > 0) {
                        db(16,1,"1st pass, err_data.err_id",err_data.err_id);
                        db(30, err_data.detail_data_len,
                               err_data.detail_data);
                        switch (err_data.err_id) {
                        case ERRID_DISK_ERR1:
                                /* Media error - Defined in the PDG as a non-
                                 * recoverable data error. Check for valid LBA
                                 * then make sure it is not already counted.
                                 */
                                if (err_data.detail_data[20] & 0x80) {
                                    lba=((uint)err_data.detail_data[23] << 24) +
                                        ((uint)err_data.detail_data[24] << 16) +
                                        ((uint)err_data.detail_data[25] <<  8) +
                                         (uint)err_data.detail_data[26];
                                    /* Search in lba list to see if it is in */
                                    for(i=0;i<= odm_info.max_hard_data_error;
                                                i++)
                                        if (lba == stored_lbas[i]) {
                                                found = TRUE;
                                                break;
                                        }
                                        if (found)
                                        /* reset flag for next search */
                                                found = FALSE;
                                        else {
                                                if (total_lba_count <
                                                  odm_info.max_hard_data_error){
                                                    stored_lbas[total_lba_count]
                                                                 = lba;
                                                    ++total_lba_count;
                                                }
                                        ++err_count[1];
                                        }
                                } else
                                        ++err_count[1];
                                if (err_count[1] >= odm_info.max_hard_data_error){
                                        if (stored_lbas != (uint *)NULL)
                                                free(stored_lbas);
                                        create_frub(ELA_ERROR1);
                                        insert_fru_bucket = TRUE;
                                        op_flg = TERMI;
                                        DA_SETRC_STATUS(DA_STATUS_BAD);
                                } else
                                        op_flg = SUBSEQ;
                                if ((err_count[1] != 0)
                                            && (damode & DA_CONSOLE_TRUE))
                                        chk_asl_status(disp_menu(BACKUP_DRIVE
                                                ,failing_function_code));
                                break;
                        case ERRID_DISK_ERR2:
                                /* Hardware Error - Defined in the PDG as a non-
                                 * recoverable equipment check. None are allowed
                                 * so if we get one, create fru then exit.
                                 */
                                ++err_count[2];
                                if (err_count[2]>odm_info.max_hard_equip_error){
                                        create_frub(ELA_ERROR1);
                                        insert_fru_bucket = TRUE;
                                        op_flg  = TERMI;
                                        DA_SETRC_STATUS(DA_STATUS_BAD);
                                } else
                                        op_flg = SUBSEQ;
                                break;
                        case ERRID_DISK_ERR3:
                                /* Adapter detected error. 
                                 */
                                ++err_count[3];
                                if (err_count[3] > 0 ){
                                        create_frub(ELA_ERROR3);
                                        insert_fru_bucket = TRUE;
                                        op_flg  = TERMI;
                                        DA_SETRC_STATUS(DA_STATUS_BAD);
                                } else
                                	op_flg = SUBSEQ;
                                break;
                        case ERRID_DISK_ERR4:
                                op_flg = SUBSEQ; /* skip on first pass */
                                break;
                        case ERRID_DISK_ERR5: /* Unknown error - ignored here */
                                op_flg = SUBSEQ;
                                break;
                        default:
                                op_flg =SUBSEQ;
                                break;
                        }
                } else
                        op_flg = TERMI;
        } while (op_flg != TERMI);
        if (stored_lbas != (uint *)NULL)
                free(stored_lbas);
        /* close out error_log_get */
        if (error_log_get(op_flg, srch_crit, &err_data) < 0)
                clean_up();
        if (! insert_fru_bucket){
                 /*
                    Process error log again this time only look for
                    soft errors and search by name only.
                 */
                sprintf(srch_crit, " -N %s" , tm_input.dname);
                op_flg = INIT;
                do {
                        /* get 1st entry in log if any */
                     rc = error_log_get(op_flg, srch_crit, &err_data);
                     if (rc < 0) {
                        op_flg = TERMI;
                        rc = error_log_get(op_flg, srch_crit, &err_data);
                        clean_up();
                     }
                     if (rc > 0) {
                        db(16,1,"2nd pass, err_data.err_id",err_data.err_id);
			db(10,1, "err_data.detail_data_len", 
                                  err_data.detail_data_len);
                        db(30, err_data.detail_data_len,
                               err_data.detail_data);
                        switch (err_data.err_id) {
                        case ERRID_DISK_ERR4:
                             ++err_count[4];
                             sense_key = (int)err_data.detail_data[22];
                             sense_code = (int)(err_data.detail_data[32] << 8)
                                           + (int)(err_data.detail_data[33]);
                             process_sense_data(
                                     sense_key & 0x0f, sense_code, (short)4,
                                     &reassign, &error_type);
			     db(16,2,"sense key",(sense_key & 0x0F),
				     "sense code",sense_code);
			     db(10,1,"error _type",error_type);
                             if (error_type == RECOVERED_EQUIP_ERROR) {
                                        error_info.soft_equip_error_found=TRUE;
					db(25,"Soft Equip Error");
			     }
                             if (error_type == RECOVERED_DATA_ERROR) {
                                        error_info.soft_data_error_found=TRUE;
					db(25,"Soft Data Error");
			     }

                /* Determine total segment count by substracting the
                 * last segment count value in the log from the segment
                 * count value of the first entry. Also handle case where
                 * segment count is not in decreasing order in the log.
                 */
                             if (! first_soft_err){
                                initcnt =  segment_count(err_data);
				db(10,1,"1st segment count",initcnt);
                                previous = initcnt;
                                first_soft_err = TRUE;
                             } else {
                                nextcnt = segment_count(err_data);
				db(10,1,"next segment count",nextcnt);
                                /* Current segment count should always
                                 * be less than previous. If not, reset
                                 * has occured.
                                 */
                                if (nextcnt > previous) {
                                /* compute segment count now then reset
                                 * initcnt.
                                 */
                                       if (initcnt == previous)
                                           error_info.totcnt = error_info.totcnt
                                                   + initcnt;
                                        else
                                           error_info.totcnt = error_info.totcnt
                                                   + (initcnt - previous);
                                        initcnt = nextcnt;
                                }
                                previous = nextcnt;
                             }
			     db(10,3,"initcnt",initcnt,"previous",previous,
				     "nextcnt",nextcnt);
			     db(10,1,"-- error_info.totcnt",error_info.totcnt);
                             op_flg = SUBSEQ;
                             break;
                         default:
                                op_flg = SUBSEQ;
                                break;
                         } /* end switch */
                    } else { /* rc == 0 */
                         op_flg = TERMI;
                         if (err_count[4] > 1){
                                if (initcnt == nextcnt)
                                     error_info.totcnt = error_info.totcnt +
                                                initcnt;
                                else
                                     error_info.totcnt = error_info.totcnt +
                                            (initcnt - nextcnt);
                                if (test_if_failed(error_info, err_count[4])
                                        != FALSE) {
                                      create_frub(ELA_ERROR1);
                                      insert_fru_bucket = TRUE;
                                      DA_SETRC_STATUS(DA_STATUS_BAD);
                                }
                        }
                    }
            } while (op_flg != TERMI);
            if (error_log_get(op_flg, srch_crit, &err_data) < 0)
                clean_up();
            }
            if (insert_fru_bucket){
                if (insert_frub(&tm_input, &frub[conf_level]) != 0) {
                          DA_SETRC_ERROR(DA_ERROR_OTHER);
                          clean_up();
                }
                if (addfrub(&frub[conf_level]) != 0) {
                          DA_SETRC_ERROR(DA_ERROR_OTHER);
                          clean_up();
                }
            }
} /* endfunction chk_ela */
/*  */
/* NAME:segment_count()
 *
 * FUNCTION: Calculate the segment count from the detail data field
 *           of the error entry.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES: Byte 148-151 represents the segment count.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: segment count.
 */

uint segment_count(err_data)
struct  errdata err_data;
{
        uint    seg_cnt;

        seg_cnt =((uint)(err_data.detail_data[148] << 24)+
                   (err_data.detail_data[149] << 16)+
                   (err_data.detail_data[150] << 8)+
                   (err_data.detail_data[151]));
        return(seg_cnt);
}
/*  */
/*
 * NAME:test_if_failed()
 *
 * FUNCTION: Tests if the number of soft data/equipment errors found in the
 *           error log have exceeded the limits found in the PDG.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES: errdata
 *
 * RETURNS: None.
 */

int test_if_failed(error_info, tot_errs)
struct  error_log_info error_info;
int     tot_errs;
{
        int     it_failed_ela = FALSE;             /* did it fail             */
        float   error_rate, threshold;
        int     min_data_rd;
        int     max_data_errs;

	db(25,"In test_if_failed()");
	db(10,1,"error_info.totcnt",error_info.totcnt);
	db(10,1,"tot_errs",tot_errs);
        if (error_info.soft_data_error_found) {
                if ((error_info.totcnt >= 1000) || (
                        (tot_errs >= (2 * odm_info.max_soft_data_error)) &&
                        (error_info.totcnt>=10))){
                    error_rate=(float)tot_errs / (float)error_info.totcnt;
                    it_failed_ela=(error_rate > odm_info.soft_data_error_ratio);
                }
        }
        if (error_info.soft_equip_error_found){
                if ((error_info.totcnt >= 1000) || (
                        (tot_errs >= (2 * odm_info.max_soft_equip_error)) &&
                        (error_info.totcnt >=10))){
                     error_rate = (float)tot_errs / (float)error_info.totcnt;
                     it_failed_ela=(error_rate>odm_info.soft_equip_error_ratio);
                }
        }
        return(it_failed_ela);
} /* endfunction test_if_failed */
/*  */
/*
 * NAME: clean_up()
 *
 * FUNCTION: Clean_up is called whenever a fatal error occurs or after
 *           the diagnostic tests have completed. The purpose is to
 *           close files, quit ASL and provide a single point exit back
 *           to the diagnostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

void    clean_up()
{
        register  int rc;

        term_dgodm();                                    /* terminate the ODM */
        if (fdes > 0){                             /* Close device, if open */
                close(fdes);

        }
        if (init_config_state >= 0){
                rc = initial_state(init_config_state, tm_input.dname);
                if (rc == -1)
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
        }
        if (init_adapter_state >= 0){
                rc = initial_state(init_adapter_state, tm_input.parent);
                if (rc == -1)
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
        }
        if (catd != CATD_ERR)		/* Close catalog files */
                catclose(catd);
        if (damode & DA_CONSOLE_TRUE)
                diag_asl_quit(NULL);	/* Terminate the ASl stuff */
	if (cudv)
		odm_free_list(cudv, &obj_info);

        db(999);
        DA_EXIT();			/* Exit to the Diagnostic controller */
} /* endfunction clean_up */

/*  */
/*
 * NAME: get_sense_data()
 *
 * FUNCTION: Retrieve sense data information by querying the device and
 *           also obtain the desired sense data from the ODM data base.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Return codes from Mode Sense command
 */

int
get_sense_data(fdes, changeable_mode_data,
                desired_mode_data, mode_data_length)
int     fdes;
char    *changeable_mode_data;
char    *desired_mode_data;
int     *mode_data_length;
{
        int     rc;
        int     sense_length;

        page_code=0x7f; /* Report changeable mode data */
        rc = tu_test(fdes, SCATU_MODE_SENSE);
        if (rc != 0)
                return(rc);
        sense_length=tu_buffer[0];
        bcopy(tu_buffer, changeable_mode_data, sense_length);
        get_diag_att("osdisk", "deflt_mode_data", 'b', &sense_length,
                        desired_mode_data);
        *mode_data_length=sense_length;
        return(0);
}


/* Debug stuff */

/*
 * NAME: db()
 *
 * FUNCTION: Write debug data to a file.
 *
 * NOTES:
 *      - The calls to db() are for debug. If the debug file exists,
 *        debug information will be written to it. If the debug file
 *        does not exist, db() will simply return. To turn on debug,
 *        touch /tmp/.DIAG_DISK_DBUG and that will be the debug file.
 *        To append to the debug file each time the DA/SA is run,
 *        export DIAG_DISK_DBUG=APPEND before running diagnostics.
 *
 *      - The function has variable arguments and will call va_start()
 *        and va_end() even if the debug file does not exist.
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
        sprintf(fname, "/tmp/.dt.%s", tm_input.dname);

        if ((rc = stat("/tmp/.DIAG_DADISK_TRACE",&file_status)) == -1) {
                va_end(ag);
                return; /* Debug file not present. */
        }

        if (!strcmp((char *)getenv("DIAG_DADISK_TRACE"),"APPEND")) {
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
                fprintf(dbfptr," SCSI CDB (Data Out)\n\t");
                p_data(dbfptr, db_tucb->scsitu.scsi_cmd_blk,
                               db_tucb->scsitu.command_length);
                if (db_tucb->scsitu.data_length) {
                        fprintf(dbfptr," Param List (Data Out)\n\t");
                        p_data(dbfptr, db_tucb->scsitu.data_buffer, 64);
                }
                break;
        case 3: /* print Sense Data */
                db_tucb = va_arg(ag, SCSI_TUTYPE *);
                fprintf(dbfptr,
                        " Sense Key %02.2X, ASC/ASCQ = %04.4X\n",
                        (db_tucb->scsiret.sense_key & 0x0F),
                        db_tucb->scsiret.sense_code);
                fprintf(dbfptr," Sense Data\n\t");
                p_data(dbfptr, db_tucb->scsitu.data_buffer, 128);
                break;
        case 4: /* print db_tucb info after call to exectu(). */
		db_tucb = va_arg(ag, SCSI_TUTYPE *);
		if ((db_tucb->scsitu.flags & B_READ) &&
		    (db_tucb->scsitu.data_length)) {
		    	fprintf(dbfptr,"\tParam List (Data In)\n\t");
			p_data(dbfptr, db_tucb->scsitu.data_buffer,
			db_tucb->scsitu.data_length);
		}
		break;
	case 10: /* Print multiple integers (int_name = int_value). */
        case 16:
                i1 = va_arg(ag, int); /* Number of int name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        i2 = va_arg(ag, int);
                        if (dbug_num == 16) /* Hex */
                                fprintf(dbfptr, " %s = %X\n", s1, i2);
                        else /* Decimal */
                                fprintf(dbfptr, " %s = %d\n", s1, i2);
                }
                break;
        case 20: /* print  multiple strings (ptr_name = ptr). */
                i1 = va_arg(ag, int); /* Number of ptr name/value pairs. */
                for (i = 0; i < i1; i++) {
                        s1 = va_arg(ag, char *);
                        s2 = va_arg(ag, char *);
                        fprintf(dbfptr, " %s = %s\n", s1, s2);
                }
                break;
        case 25:
                /* Print a simple string. */
                s1 = va_arg(ag, char *);
                fprintf(dbfptr, "%s\n",s1);
                break;
        case 30: /* Display buffer data. */
                i1 = va_arg(ag, int);
                s2 = va_arg(ag, char *);
                fprintf(dbfptr, " Data Buffer\n\t");
                p_data(dbfptr, s2, i1);
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

        if (len > 0xFF)
		len = 64;
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



/*
 * NAME: tu_issue_scsi_inquiry
 *
 * FUNCTION: Issues SCSI inquiry to device
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the return code from the test unit
 *          called by the exectu function.
 */
int
tu_issue_scsi_inquiry(tu_fdes, tnum,code_page)
int   tu_fdes;
int   tnum;
int   code_page;
{
        extern   SCSI_TUTYPE tucb;              /* scsi command structure  */
        int      rc;                            /* function return code    */

        tucb.header.tu = tnum;            /* put TU number to do in TU struct */
        tucb.header.mfg = 0;             /* always set to 0, only used by mfg */
        rc = scsitu_init (&tucb);
        if (rc != SCATU_GOOD) {
                return (SCATU_BAD_PARAMETER);
        }
        tucb.scsitu.flags = 0;
        tucb.scsitu.ioctl_pass_param = PASS_PARAM;
        memset(tu_buffer, 0x00, 1024);
	if (lun_id < 8)
        	tucb.scsitu.scsi_cmd_blk[1] |= lun_id << 5;
	else
		tucb.scsitu.lun = lun_id;
        /*
          Based on TU to do, set up other parameters
        */

	if (tnum == SCATU_INQUIRY) {
		if (code_page != (int)NO_INQ_PAGE)  {

			/*
			 An Extended Inquiry is being requested
			*/

			tucb.scsitu.scsi_cmd_blk[1] |= 0x1;
			tucb.scsitu.scsi_cmd_blk[2] = (uchar)code_page ;
		}

                tucb.scsitu.cmd_timeout = 30;
                tucb.scsitu.flags |= B_READ;
                tucb.scsitu.scsi_cmd_blk[4] = 255;
                tucb.scsitu.data_length = 255;
                tucb.scsitu.data_buffer = tu_buffer;
		chk_terminate_key();
		/* Print the Data In from device. */
		db(25,"In tu_issue_scsi_inquiry");
		db(2,&tucb);
		rc=exectu(tu_fdes, &tucb);
		db(4,&tucb);
		db(10,1,"exectu() rc",rc);
		chk_terminate_key();
		return(rc);
	}
	else {
                return (SCATU_BAD_PARAMETER);
	}
}
