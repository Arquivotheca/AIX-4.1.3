static char sccsid[] = "@(#)73  1.17.2.24  src/bos/diag/util/uformat/uformat.c, dsauformat, bos41J, 9523C_all 6/9/95 18:07:18";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: base_path
 *		clean_up
 *		command_certify
 *		command_format
 *		command_prolog
 *		do_work
 *		get_sense_data
 *		main
 *		p_tucb
 *		tu_test
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <locale.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <memory.h>
#include <errno.h>
#include <sys/buf.h>
#include <sys/signal.h>

#include <sys/scsi.h>
#include <sys/scdisk.h>
#include <diag/scsd.h>

#include "diag/scsi_atu.h"
#include "diag/diag.h"
#include "diag/da.h"		/* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/dcda_msg.h"
#include "diag/diago.h"
#include "diag/dascsi.h"
#include "diag/diag_exit.h"	/* return codes for DC */
#include "diag/tmdefs.h"	/* diagnostic modes and variables     */
#include "diag/diagodm.h"
#include  "sys/cfgodm.h"
#include "dt.h"
#include "uformat.h"
#include "ufmt_msg.h"

#define	isSCSD	(!strcmp(cudv_selected->PdDvLn_Lvalue, "disk/scsi/scsd"))

uint            maxbyte;
int		progmode;
DRIVE_INFO      drive_info;	/* information about drive		    */
ODM_INFO	odm_info;       /* Structure containing ODM information     */
char           *progname;	/* will be base of program name		    */
uchar           tu_buffer[MAX_SEND];/* buffer for write and request sense  */
SCSI_TUTYPE     tucb;		/* structure for scsi commands		    */
int             i_reserved_it;	/* set to true when a reserve is done	    */
int		pvid_destroyed; /* true then need to clear data base        */
int		is_working;	/* set to tru when formatting/certif. drive */
int             fdes = 0;	/* file descriptor of device		    */
struct listinfo obj_info;	/* struct for ODM retrieve		    */
struct listinfo sd_obj_info;	/* struct for ODM retrieve		    */
struct CuDv    *cudv;		/* struct for ODM retrieve		    */
struct CuDv    *cudv_selected;	/* struct for ODM retrieve		    */
struct CuDv    *sd_cudv;	/* struct for ODM retrieve		    */
int             led;		/* led value for selected drive		    */
short		osdisk;		/* flag to indicate OEM drive		    */
int		page_code;      /* What to set for PAGE CODE		    */
int             certify_after;	/* certify after format (0 or 1)	    */
uint            maxblock;	/* maximum block address on drive           */
uint        	blocklength;	/* block length 		            */
uint            curblock;	/* next block to read from drive            */
char            devname[512];	/* special file for device	            */
int		old_percent;
uint            error_block;	/* lba of first block in error              */
char		srw_call[128];
static int	init_adapter_state = -1;
static int	init_config_state = -1;
uchar		lun_id;	/* LUN of device on scsi id	 */
uchar		scsi_id;		/* device's scsi id	 */

/* Global variables for command time outs */
int		rw_to = 30;		/* read/write command time out	      */
int		start_stop_to = 60;	/* start/stop comamnd time out        */
int		reassign_to = 120;	/* reassign comamnd time out          */
int		format_unit_to = 0;	/* format unit comamnd time out       */
struct da_menu	dmnu =
{
        "", DUFORMAT_SET, 0, 0
};

extern int	process_sense_data(int, int, short, short *, short *);
extern int	get_odm_info(char *, int *, short *, ODM_INFO *);
extern int	set_default_sense(uchar *,int,uchar *,uchar *,uchar *,int *);
extern void	int_hand();
extern int	diag_get_sid_lun(char *, uchar *, uchar *);

#define	PROLOG_STEP_1 1
#define	PROLOG_STEP_2 2
#define	PROLOG_STEP_3 3
#define	PROLOG_STEP_5 5
#define	PROLOG_STEP_6 6
#define	PROLOG_STEP_7 7
#define	PROLOG_STEP_8 8
#define	PROLOG_STEP_9 9
#define	PROLOG_STEP_10 10
#define PROLOG_NUM_STEPS 10

#define	CERTIFY_STEP_1 1
#define	CERTIFY_STEP_2 2
#define	CERTIFY_STEP_3 3
#define	CERTIFY_STEP_4 4
#define	CERTIFY_STEP_5 5
#define	CERTIFY_STEP_6 6
#define	CERTIFY_STEP_7 7
#define	CERTIFY_STEP_8 8
#define	CERTIFY_STEP_9 9
#define CERTIFY_NUM_STEPS 9

#define	FORMAT_STEP_1 1
#define	FORMAT_STEP_2 2
#define	FORMAT_STEP_3 3
#define	FORMAT_STEP_4 4
#define	FORMAT_STEP_5 5
#define	FORMAT_STEP_6 6
#define	FORMAT_STEP_7 7
#define	FORMAT_STEP_8 8
#define FORMAT_NUM_STEPS 8

/* begin main */

int
main(argc, argv, envp)
int             argc;	/* argument list count		 */
char          **argv;	/* argument list vector		 */
char          **envp;	/* environment list vector	 */
{
	struct sigaction act;

	void            do_work();	/* do the format or certify	 */
	void            clean_up();	/* what to do on exit		 */
	char           *base_path();	/* point to base of program name */
	int             disp_menu();
	void		clear_pvid();
	extern char    *progname;	/* base of program name		 */
	extern int      progmode;	/* defines device we are testing */
	extern DRIVE_INFO drive_info;
	extern int      i_reserved_it;	/* set to true when reserve is done */

	act.sa_handler = int_hand;
	sigaction(SIGQUIT, &act, (struct sigaction *) NULL);

	act.sa_handler = int_hand;
	sigaction(SIGINT, &act, (struct sigaction *) NULL);

	setlocale(LC_ALL, "");
	init_dgodm();		/* set up the odm */
	diag_asl_init(0);	/* set up asl */

	progname = base_path(argv[0]);	/* just get base of the name	 */

	is_working = 0;	/* drive is not currently formatting */
	pvid_destroyed = 0; /* pvid is still intact */
	i_reserved_it = 0;	/* no reservation yet		 */
	drive_info.percent_complete = 0;	/* nothing complete yet */
	drive_info.drive_capacity = 0;
	drive_info.rec_data_errors = 0;
	drive_info.unrec_data_errors = 0;
	drive_info.rec_equ_check_errors = 0;
	drive_info.unrec_equ_check_errors = 0;
	strcpy(drive_info.loc_code, "");

	dmnu.catfile = "ufmt.cat";

	/*
	 * Now, lets actually do something useful. 
	 */

	dt("certify", DT_MSG, "============== start =============");
	do_work();
	clean_up();		/* DOES NOT RETURN */

	return (0);		/* keep lint happy */
}				/* main end */

/*
 * NAME: do_work
 *
 * FUNCTION: Open device to be tested and control the testing.
 *
 * EXECUTION ENVIRONMENT:
 *	This function must have sole use of the device to be tested.
 *
 * NOTES: More detailed description of the function, down to
 *	what bits / data structures, etc it manipulates.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: NONE
 */

void
do_work()
{
	extern int      fdes;	/* file descriptor of device	 */
	extern int	errno;  /* error description from openx  */

	char            criteria[128];	/* temp for odm get */
	/*
	 * counter for reaching various checkpoints in the test procedure 
	 */
	int             temp_i;	/* temporary int */
	int		ipl_flag;
	int     	cuat_mod;
	int    	 	ipl_mod;
	struct CuAt     *cuat;        /* ODM Customized device struct */
	int             how_many;
	int		model_7008=FALSE;
	int		normal_mode=FALSE;
	int 		locerrno=0;
	int		rc=0;
	char		*diagdir, command[256];
	int             disp_menu();
	int             command_prolog();
	int             command_certify();
	int             command_format();
	int             i;
	struct  	disk_scsd_inqry_data *scsd_vpd;
	struct  	disk_scsd_mode_data  *scsd_mode_data;	


	/* get list of drives on system, except disk array devices.   */

	sprintf(criteria,
	"PdDvLn LIKE disk/* and chgstatus != 3 and PdDvLn != disk/dar/array");
	cudv = get_CuDv_list(CuDv_CLASS, criteria, &obj_info, 16, 2);
	if ((cudv == (struct CuDv *) - 1) ||
	    (cudv == (struct CuDv *) NULL)) {
		/* no hard drives */
		disp_menu(NO_HARD_DISKS);
		clean_up();
	}

	/* check to see if machine is a Model 7008 */

	ipl_mod = get_cpu_model(&cuat_mod);
	if (ipl_mod == CAB_MODEL) {

		/* search ODM customized device attribute file */

		model_7008=TRUE;
		cuat = (struct CuAt *)getattr("sys0","keylock",FALSE,&how_many);
		if ((cuat == (struct CuAt *) - 1) || (cuat == (struct CuAt *) NULL))            {
			disp_menu(MENU_NO_ATTR_2);
			clean_up();
		}
		else	{
			rc=strcmp(cuat->value,"normal");
			if (rc==0) {
				normal_mode=TRUE;
				disp_menu(OS_WARN_CONTINUE);
			}
			else {
				disp_menu(OS_AND_DATA_WARN);
				clean_up();
			}
		}
	}

	temp_i = disp_menu(MENU_FORMAT_OR_CERTIFY); 
	if (temp_i == 1)
		progmode = UFORMAT;
	else if (temp_i == 2)
		progmode = UCERTIFY;
	else
		clean_up();	/* bad choice */

	/* get info about the drive to be worked on */

	temp_i = disp_menu(MENU_SELECT_DRIVE);
	if (temp_i < 1 || temp_i > obj_info.num) {
		/* invalid drive selection */
		clean_up();
	}
	cudv_selected = cudv + (temp_i - 1);

	/* Retrieve other information from ODM data base for SCSI disk */
	(void)get_odm_info(cudv_selected->name, &led, &osdisk, &odm_info);

	strcpy(devname, "/dev/r");
	strcat(devname, cudv_selected->name);

	/* get directory path to use for execution of command */
	if ((diagdir = (char *)getenv("DIAGNOSTICS")) == NULL)
		diagdir = DIAGNOSTICS;

	/*......................................................*/
	/* If selected drive is a serial redwing, then call     */
	/* srw_call (see srwfor.c)                              */
	/*......................................................*/

	if (led == S_REDWING) {

		/* get the controller parent (adapter) name for config */
		sprintf(criteria,
		    "name = %s and chgstatus != 3",
		    cudv_selected->parent);
		sd_cudv = get_CuDv_list(CuDv_CLASS, criteria, &sd_obj_info, 128, 2);
		if ((sd_cudv == (struct CuDv *) - 1) || 
		    (sd_cudv == (struct CuDv *) NULL)) {
			disp_menu(MENU_TERMINATED);
			clean_up();
		}
		certify_after = 0;
		if (progmode == UFORMAT) {
			temp_i = disp_menu(MENU_F_SR_CERTIFY_AFTER);
			if (temp_i == 1)
				certify_after = 1;
		}
		/* build the command for a system call to srwfor */
		strcpy(command, diagdir);
		strcat(command, "/bin/srwfor");
		sprintf(srw_call,
		    "%s %s %s %d %s %s %s %d",
		    command,
		    cudv_selected->name,
		    cudv_selected->location,
		    progmode,
		    cudv_selected->connwhere,
		    cudv_selected->parent,
		    sd_cudv->parent, certify_after);
		term_dgodm();
		diag_asl_quit();
		exit(system(srw_call));
	}

	/* check led value and call kazusa format if needed */

	if (led == DIRECT_BUS_ATTACH) {
		char            kaz_call[512];	/* command for kazusa
						 * format/certify */
		strcpy(kaz_call, diagdir);
		strcat(kaz_call, "/bin/kazfor");
		strcat(kaz_call, " ");
		strcat(kaz_call, devname);
		strcat(kaz_call, " ");
		strcat(kaz_call, cudv_selected->location);
		if (progmode == UFORMAT) {
			temp_i = disp_menu(MENU_F_A_CERTIFY_AFTER);
			if (temp_i == 1) {
				/* format */
				strcat(kaz_call, " 1");
			} else if (temp_i == 2) {
				/* format and surface analysis */
				strcat(kaz_call, " 2");
			} else if (temp_i == 3) {
				/* format and surface analysis with certify */
				strcat(kaz_call, " 3");
			} else {
				/* unknown selection */
				clean_up();
			}
		} else {
			/* certify only */
			strcat(kaz_call, " 4");
		}
		term_dgodm();
		diag_asl_quit();
		exit(system(kaz_call));
	}
	certify_after = 0;
	if (progmode == UFORMAT) {
		temp_i = disp_menu(MENU_F_CERTIFY_AFTER);
		if (temp_i == 1)
			certify_after = 1;
		else if (temp_i == 3)
			progmode = UDCLASS;

		if (progmode != UDCLASS)
		{
			temp_i = disp_menu(MENU_F_WARNING);
			if (temp_i == 2) {
				/* not a yes answer */
				clean_up();
			}
		}
	}

	if (diag_get_sid_lun(cudv_selected->connwhere, &scsi_id, &lun_id)==-1){
		disp_menu(MENU_TERMINATED);
		clean_up();
	}

	init_adapter_state = configure_device(cudv_selected->parent);
	if (init_adapter_state == -1){
		disp_menu(ADAPTER_CONFIGURE);
		clean_up();
	}
	init_config_state = configure_device(cudv_selected->name);
	if (init_config_state == -1){
		disp_menu(DEVICE_CONFIGURE);
		clean_up();
	}

	strcpy(drive_info.loc_code, cudv_selected->location);
	/* open the device */
	fdes = openx(devname, O_RDWR, NULL, SC_DIAGNOSTIC);
	if (fdes < 0) {
		locerrno=errno;
		ipl_mode(&ipl_flag);
		if ((ipl_flag == 0)&&((locerrno==EBUSY)||(locerrno==EACCES))){
			if (model_7008 && normal_mode) {
				disp_menu(OS_WARN_RETURN);
			}
			else {
				if (progmode == UFORMAT) {
					disp_menu(MENU_F_USE_FLOPPY_2);
				}
				else if (progmode == UCERTIFY) {
					disp_menu(MENU_C_USE_FLOPPY_2);
				}
				else 
					disp_menu(MENU_E_USE_FLOPPY);
			}
		} else {
			disp_menu(MENU_TERMINATED);
		}
		return;
	}
	
	if (isSCSD){
		/*
		  This is an SCSD disk
		  */	
		/* 
		  Get SCSD desired mode data
		  for this device. 
		  */
		rc = tu_issue_scsi_inquiry(fdes,
					   SCATU_INQUIRY, 
					   0xc7);
		if (rc == SCATU_GOOD) {
			scsd_vpd = (struct disk_scsd_inqry_data *) tu_buffer;
			
			/*
			  Set SCSD time out values
			  */
			
			if (scsd_vpd->rw_timeout) 
				rw_to = scsd_vpd->rw_timeout;
			if (scsd_vpd->start_timeout)
				start_stop_to = scsd_vpd->start_timeout;
			if (scsd_vpd->reassign_timeout)
				reassign_to = scsd_vpd->reassign_timeout;
			if (scsd_vpd->fmt_timeout)
				format_unit_to = scsd_vpd->fmt_timeout;
		}
		else {
			disp_menu(MENU_TERMINATED);
			clean_up();
		}
		
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

	if (command_prolog() != 0) {
		if (fdes > 0)
			close(fdes);
		return;
	}

	if (progmode == UFORMAT) {
		if (command_format() != 0) {
			if (fdes > 0)
				close(fdes);
			return;
		}
		if (certify_after) {
			if (command_prolog() != 0) {
				if (fdes > 0)
					close(fdes);
				return;
			}
			/* change progmode (to get messages correct) */
			progmode = UCERTIFY;
			if (command_certify() != 0) {
				if (fdes > 0)
					close(fdes);
				return;
			}
		}
	} else if (progmode == UCERTIFY) {
		if (command_certify() != 0) {
			if (fdes > 0)
				close(fdes);
			return;
		}
	} else if (progmode == UDCLASS) {
		rc = erase(devname);
		if (fdes > 0)
			close(fdes);
	}
	return;
}

/*
 * NAME: command_prolog
 *
 * FUNCTION: set up steps prior to erasure operation.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 * 		0 - for good compeletion.
 *              NONE 0 - for a failure. 
 */

int
command_prolog()
{
	extern int      i_reserved_it;	/* set when reserve is good	 */
	extern int      fdes;	/* file descriptor of device	 */
	extern SCSI_TUTYPE tucb;/* structure for scsi commands	 */

	int             rc;	/* return code from tu_test()	 */
	int             tu_test();
	/*
	 * counter for reaching various checkpoints in the test procedure 
	 */
	int             stepcount[PROLOG_NUM_STEPS + 1];
	int             step;	/* current step executing	 */
	int             temp_i;	/* temporary int */
	int             disp_menu();


	drive_info.percent_complete = 0;
	/*
	temp_i = disp_menu(MENU_PLEASE_STAND_BY);
        */

	/* clear accumulative step counters */
	for (step = 0; step <= PROLOG_NUM_STEPS; ++step)
		stepcount[step] = 0;

	/* start at step 1 */
	step = PROLOG_STEP_1;

	dt("certify", DT_MSG, "PROGLOG starts ");

	/* Run all of the necessary tests  */
	while (step) {
		dt("certify", DT_DEC, " step", step);

		++stepcount[step];
		switch (step) {
		case PROLOG_STEP_1:	/* RESERVE UNIT */

			dt("certify", DT_DEC, " RESERVE rc", rc);

			rc = tu_test(fdes, SCATU_RESERVE_UNIT);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_TIMEOUT:
				step = PROLOG_STEP_5;
				break;
			case SCATU_BUSY:	/* or command_error */
				if (stepcount[PROLOG_STEP_1] == 1) {
					step = PROLOG_STEP_6;
				} else {
					step = PROLOG_STEP_5;
				}
				break;
			case SCATU_RESERVATION_CONFLICT:
				if (stepcount[PROLOG_STEP_1] == 1) {
					step = PROLOG_STEP_7;
				} else {
					step = PROLOG_STEP_5;
				}
				break;
			case SCATU_CHECK_CONDITION:
				step = PROLOG_STEP_2;
				break;
			case SCATU_GOOD:
				i_reserved_it = 1;
				step = PROLOG_STEP_8;
				break;
			default:
				return (-1);
			}
			break;
		case PROLOG_STEP_2:	/* REQUEST SENSE */
			rc = tu_test(fdes, SCATU_REQUEST_SENSE);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
				if (stepcount[PROLOG_STEP_2] == 1) {
					step = PROLOG_STEP_6;
				} else {
					step = PROLOG_STEP_5;
				}
				break;
			case SCATU_TIMEOUT:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
				step = PROLOG_STEP_5;
				break;
			case SCATU_GOOD:
				step = PROLOG_STEP_3;
				break;
			default:
				return (-1);
			}
			break;
		case PROLOG_STEP_3:	/* examine the request sense data */

			dt("certify", DT_MHEX, 2,
				"PROLOG sense_key", tucb.scsiret.sense_key,
				"PROLOG sense_code", tucb.scsiret.sense_code);
			dt("certify", DT_BUFF, tu_buffer, 32);

			if (tucb.scsiret.sense_key == 0x06) {
				/* UNIT ATTENTION */
				if (stepcount[PROLOG_STEP_3] == 1) {
					step = PROLOG_STEP_1;
				} else {
					step = PROLOG_STEP_5;
				}
				break;
			} else if (tucb.scsiret.sense_key == 0x02) {
				switch(tucb.scsiret.sense_code) {
				case 0x0400 :
				case 0x0401 :
				case 0x0402 :
					step = PROLOG_STEP_8;
					break;
				default:
					step = PROLOG_STEP_5;
					break;
				}
				break;
			}
			/* unaccounted for sense key, dead drive */
			step = PROLOG_STEP_5;
			break;
		case PROLOG_STEP_5:	/* unable to use drive */
			disp_menu(MENU_TERMINATED);
			return (-1);
		case PROLOG_STEP_6:	/* we got here because of a BUSY */
			if (fdes > 0)
				close(fdes);

			/* Reopen device, sending SCSI device reset */
			fdes = openx(devname, O_RDWR, NULL, SC_FORCED_OPEN);

			/* if open ok, close and reopen in diag mode */
			if (fdes < 0) {
				step = PROLOG_STEP_5;
			} else {
				if (fdes > 0)
					close(fdes);
				fdes = openx(devname, O_RDWR, NULL,
				    SC_DIAGNOSTIC);
			}

			/* Reopen device, in diagnostic mode */
			/* and test if open ok.              */
			if (fdes < 0) {
				step = PROLOG_STEP_5;
			} else {
				step = PROLOG_STEP_1;
			}
			break;
		case PROLOG_STEP_7:	/* RESERVATION CONFLICT */
			step = PROLOG_STEP_5;
			break;
		case PROLOG_STEP_8:	/* do a START_UNIT */
			dt("certify", DT_DEC, " START_UNIT rc", rc);

			rc = tu_test(fdes, SCATU_START_STOP_UNIT);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
			case SCATU_TIMEOUT:
				step = PROLOG_STEP_5;
				break;
			case SCATU_GOOD:
				step = PROLOG_STEP_9;
				break;
			default:
				return (-1);
			}
			break;
		case PROLOG_STEP_9:	/* do a MODE_SENSE */
			page_code = 0x3f;
			rc = tu_test(fdes, SCATU_MODE_SENSE);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
			case SCATU_TIMEOUT:
				step = PROLOG_STEP_5;
				break;
			case SCATU_GOOD:
				/* if block size not 512 do a format */
				if (((tu_buffer[10] != 0x02) ||
				    (tu_buffer[11] != 0x00)) &&
				    (progmode == UCERTIFY)) {
					disp_menu(MENU_SHOULD_FORMAT);
					is_working = 0;
					return(-1);
				}
				step = PROLOG_STEP_10;
				break;
			default:
				return (-1);
			}
			break;
		case PROLOG_STEP_10:	/* do a MODE_SELECT */
			rc = tu_test(fdes, SCATU_MODE_SELECT);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
			case SCATU_TIMEOUT:
				step = PROLOG_STEP_5;
				break;
			case SCATU_GOOD:
				/* get out */
				step = 0;
				break;
			default:
				return (-1);
			}
			break;
		default:
			dt("certify", DT_DEC, " UNEXPECTED prolog step", step);
			return (-1);

		}
	}			/* end of while and switch */
	return (0);
}				/* end of command_prolog */

/*
 * NAME: command_certify
 *
 * FUNCTION: set up steps prior to certify operation.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 * 		0 - for good compeletion.
 *              NONE 0 - for a failure. 
 */

int
command_certify()
{
	extern int      i_reserved_it;	/* set when reserve is good	 */
	extern int      fdes;	/* file descriptor of device	 */
	extern SCSI_TUTYPE tucb;/* structure for scsi commands	 */
	uint		block_with_error = 0;
	int             rc;	/* return code from tu_test()	 */
	int             tu_test();
	/*
	 * counter for reaching various checkpoints in the test procedure 
	 */
	int             stepcount[CERTIFY_NUM_STEPS + 1];
	int             step;	/* current step executing	 */
	int             temp_i;	/* temporary int */
	int             disp_menu();
	int             command_prolog();
	short		reassign_block;
	short		error_type;
	int		retry_count = 0;
	int		retry = 0;
	uint            blocks;	/* temporary in read_extended */
	short		recovery_count = 0;
	float		maxblock_in_mega_bytes;

	curblock = 0;
	old_percent = 0;

	drive_info.percent_complete = 0;
	/*
	temp_i = disp_menu(MENU_PLEASE_STAND_BY);
        */

	/* clear accumulative step counters */
	for (step = 0; step <= CERTIFY_NUM_STEPS; ++step)
		stepcount[step] = 0;

	/* start at step 1 */
	step = CERTIFY_STEP_1;

	dt("certify", DT_MSG, "CERTIFY starts:");

	/* Run all the necessary tests  */
	while (step) {
		++stepcount[step];

		dt("certify", DT_DEC, " step", step);

		switch (step) {
		case CERTIFY_STEP_1:
			rc = tu_test(fdes, SCATU_READ_CAPACITY);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
			case SCATU_TIMEOUT:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_GOOD:
				maxblock = (uint) tu_buffer[0] << 24 |
				           (uint) tu_buffer[1] << 16 |
				           (uint) tu_buffer[2] << 8  |
				           (uint) tu_buffer[3];
				blocklength = (uint) tu_buffer[4] << 24 |
				              (uint) tu_buffer[5] << 16 |
				              (uint) tu_buffer[6] << 8  |
				              (uint) tu_buffer[7];
				maxblock_in_mega_bytes = (float) maxblock / 1000000;
				drive_info.drive_capacity = maxblock_in_mega_bytes * blocklength;
				dt("certify", DT_MDEC, 2, "\tmaxblock",
				   maxblock, "\tlocklength", blocklength);

				step = CERTIFY_STEP_2;
				is_working = 1;
				break;
			default:
				return (-1);
			}
			break;
		case CERTIFY_STEP_2:	/* read a chunk */
 			blocks = (maxblock - curblock) + 1;
			if (blocks > 128)
 				blocks = 128;
			maxbyte = blocks * 512;
			rc = tu_test(fdes, SCATU_READ_EXTENDED);

			dt("certify", DT_DEC, "\tREAD_EXTENDED rc", rc);

			switch (rc) {
			case SCATU_BAD_PARAMETER:
				is_working = 0;
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_CHECK_CONDITION:
				step = CERTIFY_STEP_4;
				break;
			case SCATU_GOOD:
				curblock += 128;
				step = CERTIFY_STEP_3;
				break;
			default:
				is_working = 0;
				return (-1);
			}
			break;
		case CERTIFY_STEP_3:
			dt("certify", DT_DEC, "\tcurblock", curblock);

			if (curblock > maxblock) {
				/* finished */
				step = CERTIFY_STEP_8;
				drive_info.percent_complete = 100;
				is_working = 0;
				disp_menu(MENU_PLEASE_STAND_BY);
			} else {
				step = CERTIFY_STEP_2;
				drive_info.percent_complete =
				    (100L * curblock) / maxblock;

				if (drive_info.percent_complete < old_percent){
					step = CERTIFY_STEP_5;
					dt("certify", DT_MSG,
					   "*** LOOPING. PROGRAM TERMINATED ***");
					break;
				}

				if (drive_info.percent_complete !=
				    old_percent) {
					disp_menu(MENU_PLEASE_STAND_BY);
					old_percent =
					    drive_info.percent_complete;
				}
			}
			break;
		case CERTIFY_STEP_4:	/* REQUEST SENSE */
			rc = tu_test(fdes, SCATU_REQUEST_SENSE);
			dt("certify", DT_DEC, "\tREQUEST SENSE rc", rc);

			switch (rc) {
			case SCATU_BAD_PARAMETER:
				is_working = 0;
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_GOOD:
				step = CERTIFY_STEP_6;
				break;
			default:
				is_working = 0;
				return (-1);
			}
			break;
		case CERTIFY_STEP_5:	/* unable to use drive */
			is_working = 0;
			disp_menu(MENU_TERMINATED);
			return (-1);
		case CERTIFY_STEP_6:	/* examine the request extended sense
								 * data */
			dt("certify", DT_MHEX, 2,
				"\tREQ EXT SENSE sense_key", tucb.scsiret.sense_key,
				"\tsense_code", tucb.scsiret.sense_code);
			dt("certify", DT_BUFF, tu_buffer, 32);

			reassign_block = 0;
			recovery_count = ((((unsigned)tu_buffer[16]) << 8) +
			    ((unsigned)tu_buffer[17]));

			switch (tucb.scsiret.sense_key & 0x0f) {
			case 6:/* UNIT ATTENTION */
				/* was reset */
				if (block_with_error != curblock){
					retry = 0;
					block_with_error = curblock;
				}

				dt("certify", DT_DEC, "\tretry", retry);
				dt("certify", DT_MDEC, 2,
					"\tUNIT ATT curblock", curblock,
					"\terror_block", block_with_error);

				/* Retry command only twice	*/
				if ((retry < 2)||(curblock != block_with_error)){
					++retry;
					if (command_prolog() != 0) {
						is_working = 0;
						disp_menu(MENU_TERMINATED);
						return (-1);
					}
				} else {
					is_working = 0;
					disp_menu(MENU_TERMINATED);
					return (-1);
				}
				step = CERTIFY_STEP_3;
				break;
			case 0:/* NO SENSE */
			case 0x0b:	/* ABORTED COMMAND */
				step = CERTIFY_STEP_5;
				break;
			default:
				if (tucb.scsiret.sense_code == 0x3101) {
					step = CERTIFY_STEP_5;
					break;
				}

				error_type = 0; /* init */
				process_sense_data(
				    tucb.scsiret.sense_key & 0x0f,
				    tucb.scsiret.sense_code, recovery_count,
				    &reassign_block, &error_type);
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

				 dt("certify", DT_MDEC, 5,
				    "\terror_type", error_type,
				    "\tsoft_equ_check", drive_info.rec_equ_check_errors,
				    "\trec_data_errors", drive_info.rec_data_errors,
				    "\tunrec_equ_check_errors", drive_info.unrec_equ_check_errors,
				    "\tunrec_data_errors", drive_info.unrec_data_errors);

				break;
			} /* end switch sense_key */

			if (step != CERTIFY_STEP_6)
				break;

			if ((!(tu_buffer[0] & 0x80)) && (retry_count <= 4))
				/* Valid bit not set and retry less than 4 */
				/* Issue the READ command again and do not */
				/* increment the Current data block read.  */
				++retry_count;
			else if (tu_buffer[0] & 0x80){
				/* Valid bit is set to indicate that we have
				   a valid LBA. Increment current block and
				   set the error_block to get ready for the
				   reassign cmd if necessary */
				   retry_count = 0; /* clear retry count */
				   error_block = ((uint) tu_buffer[3] << 24) +
						 ((uint) tu_buffer[4] << 16) +
						 ((uint) tu_buffer[5] <<  8) +
						 ((uint) tu_buffer[6]);
				   curblock = error_block + 1;
				   }
			else {  /* we don't have an LBA, go to the next 128.
				 * we should try the next block and so on,
				 * but we'll fix it later with defect 79858 */
				 curblock += 128;
				 retry_count = 0;
			}

			if ((drive_info.unrec_equ_check_errors > 0) ||
			    (drive_info.unrec_data_errors > 9)) {
				step = CERTIFY_STEP_8;
				break;
			}

			/* Criteria for Reassign of bad blocks */
			if ((reassign_block && certify_after) && 
			    ((retry_count == 0) || (retry_count > 4))
			    && (tu_buffer[0] & 0x80))
				step = CERTIFY_STEP_7;
			else
				step = CERTIFY_STEP_3;
			break;
		case CERTIFY_STEP_7:	/* reassign the bad block */
			rc = tu_test(fdes, SCATU_REASSIGN_BLOCKS);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				is_working = 0;
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_CHECK_CONDITION:
				break;
			case SCATU_GOOD:
				step = CERTIFY_STEP_3;
				break;
			default:
				is_working = 0;
				return (-1);
			}
			if (step != CERTIFY_STEP_7)
				break;
			/* check condition from the reassign */
			step = CERTIFY_STEP_5;
			break;
		case CERTIFY_STEP_8:
			is_working = 0;
			rc = tu_test(fdes, SCATU_RELEASE_UNIT);

			dt("certify", DT_DEC, "\tRELEASE UNIT rc", rc);

			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
				step = CERTIFY_STEP_5;
				break;
			case SCATU_GOOD:
				/* it works, get out of this loop */
				step = CERTIFY_STEP_9;
				i_reserved_it = 0;
				break;
			default:
				return (-1);
			}
			break;
		case CERTIFY_STEP_9:
			disp_menu(MENU_C_COMPLETE);
			if ((drive_info.unrec_equ_check_errors > 0) ||
			    (drive_info.unrec_data_errors > 1))
				disp_menu(MENU_C_COMPLETE_BAD);
			step = 0;
			break;
		default:
			dt("certify", DT_DEC, 1,
			   " UNEXPECTED step", step);

			return (-1);
		}
	}			/* end of while and switch */
	return (0);
}				/* end of command_certify */

/*
 * NAME: command_format
 *
 * FUNCTION: set up steps prior to format operation.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 * 		0 - for good compeletion.
 *              NONE 0 - for a failure. 
 */

int
command_format()
{
	extern int      i_reserved_it;	/* set when reserve is good	 */
	extern int      fdes;	/* file descriptor of device	 */
	extern SCSI_TUTYPE tucb;/* structure for scsi commands	 */
	int             rc;	/* return code from tu_test()	 */
	int             tu_test();
	/*
	 * counter for reaching various checkpoints in the test procedure 
	 */
	int             stepcount[FORMAT_NUM_STEPS + 1];
	int             step;	/* current step executing	 */
	int             temp_i;	/* temporary int */
	int             disp_menu();
	int             command_prolog();
	float		maxblock_in_mega_bytes;


	old_percent = 0;
	drive_info.percent_complete = 0;

	/* clear accumulative step counters */
	for (step = 0; step <= FORMAT_NUM_STEPS; ++step)
		stepcount[step] = 0;
	
	/* Determine drive capacity to set Format command time out */
	rc = tu_test(fdes, SCATU_READ_CAPACITY);
	if (rc == SCATU_GOOD){
		maxblock =	(uint) tu_buffer[0] << 24 |
				(uint) tu_buffer[1] << 16 |
				(uint) tu_buffer[2] <<  8 |
				(uint) tu_buffer[3];
		blocklength =	(uint) tu_buffer[4] << 24 |
		    		(uint) tu_buffer[5] << 16 |
		    		(uint) tu_buffer[6] <<  8 |
		    		(uint) tu_buffer[7];
		maxblock_in_mega_bytes = (float) maxblock / 1000000;
		drive_info.drive_capacity = maxblock_in_mega_bytes * blocklength;
	} else /* Cannot read capacity, set a default value and go on */
		drive_info.drive_capacity = 10000;


	/* Handle other SCSI drives */
	if (osdisk)
		temp_i = disp_menu(FORMAT_IN_PROGRESS);
	else
		temp_i = disp_menu(MENU_PLEASE_STAND_BY);

	/* start at step 1 */
	step = FORMAT_STEP_1;

	/* Run all the necessary tests  */
	while (step) {
		++stepcount[step];
		switch (step) {
		case FORMAT_STEP_1:
			rc = tu_test(fdes, SCATU_FORMAT_UNIT);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_TIMEOUT:
				step = FORMAT_STEP_5;
				break;
			case SCATU_CHECK_CONDITION:
				step = FORMAT_STEP_2;
				break;
			case SCATU_GOOD:
				is_working = 1;
				pvid_destroyed = 1;
				if (osdisk)
					step = FORMAT_STEP_6;
				else
					step = FORMAT_STEP_2;
				break;
			default:
				return (-1);
			}
			break;
		case FORMAT_STEP_2:	/* REQUEST SENSE */
			rc = tu_test(fdes, SCATU_REQUEST_SENSE);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_BUSY:
			case SCATU_TIMEOUT:
			case SCATU_NONEXTENDED_SENSE:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
				step = FORMAT_STEP_5;
				break;
			case SCATU_GOOD:
				step = FORMAT_STEP_3;
				break;
			default:
				return (-1);
			}
			break;
		case FORMAT_STEP_3:	/* examine the request extended sense
								 * data */
			switch (tucb.scsiret.sense_key & 0x0f) {
			case 6:/* UNIT ATTENTION */
				/* was reset */
				if (stepcount[FORMAT_STEP_1] == 2) {
					step = PROLOG_STEP_5;
					break;
				}
				if (command_prolog() != 0) {
					return (-1);
				}
				step = FORMAT_STEP_1;
				break;
			case 0:/* NO SENSE */
				step = FORMAT_STEP_4;
				break;
			case 2:/* NOT READY */
				if (tucb.scsiret.sense_code != 0x00404)
					step = FORMAT_STEP_5;
				break;
			default:
				step = FORMAT_STEP_5;
				break;
			}

			if (step != FORMAT_STEP_3)
				break;

			if (!(tu_buffer[15] & 0x80)) {
				#ifdef DEBUGSCSI_ASL
				diag_asl_msg("byte 15 bit 7 not set\n");
				#endif
				;
			} else {
				drive_info.percent_complete =
				((((unsigned) tu_buffer[16]) << 8 |
				((unsigned) tu_buffer[17])) * 100) / 65536;
			}
			/* Update screen to show new percentage */
			if (drive_info.percent_complete > old_percent) {
				disp_menu(MENU_PLEASE_STAND_BY);
				old_percent = drive_info.percent_complete;
			}
			sleep(1);
			step = FORMAT_STEP_2;
			break;
		case FORMAT_STEP_4:	/* finished */
			is_working = 0;
			step = FORMAT_STEP_6;
			drive_info.percent_complete = 100;
			disp_menu(MENU_PLEASE_STAND_BY);
			break;
		case FORMAT_STEP_5:	/* unable to use drive */
			is_working = 0;
			disp_menu(MENU_TERMINATED);
			return (-1);
		case FORMAT_STEP_6:
			rc = tu_test(fdes, SCATU_RELEASE_UNIT);
			switch (rc) {
			case SCATU_BAD_PARAMETER:
				return (-1);
			case SCATU_TIMEOUT:
			case SCATU_BUSY:
			case SCATU_RESERVATION_CONFLICT:
			case SCATU_CHECK_CONDITION:
				step = FORMAT_STEP_5;
				break;
			case SCATU_GOOD: /* it works, get out of this loop */
				step = FORMAT_STEP_8;
				i_reserved_it = 0;
				break;
			default:
				return (-1);
			}
			break;
		case FORMAT_STEP_8:
			disp_menu(MENU_F_COMPLETE);
			step = 0;
			break;
		default:
			return (-1);

		}
	}			/* end of while and switch */
	return (0);
}				/* end of command_format */

/*
 * NAME: tu_test
 *
 * FUNCTION: Set up and Execute a particular test unit.
 *
 * NOTES: The large internal buffer used when necesary is tu_buffer.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: It returns the return code from exectu().
 */


int
tu_test(fdescr, tnum)
int             fdescr;
int             tnum;
{
	int             rc;		/* functions return code	 */
	extern SCSI_TUTYPE tucb;	/* structure for scsi commands	 */
	static uchar	changeable_mode_data[255];
	static uchar	desired_mode_data[255];
	static uchar	current_mode_data[255];
	int		sense_length;
	int		mode_data_length;

	tucb.header.tu = tnum;	/* test unit number		 */
	tucb.header.mfg = 0;	/* not in manufacturing		 */

	if (tnum != SCATU_USER_DEFINED)
	{
		rc = scsitu_init(&tucb);	/* init the structure */
		if (rc != SCATU_GOOD) {
			return (SCATU_BAD_PARAMETER);
		}
		tucb.scsitu.flags = 0;
		tucb.scsitu.ioctl_pass_param = PASS_PARAM;
		tucb.scsitu.data_length = 0;
		tucb.scsitu.data_buffer = NULL;
		tucb.scsitu.cmd_timeout = 30;
		if (lun_id < 8)
			tucb.scsitu.scsi_cmd_blk[1] |= lun_id << 5;
		else
			tucb.scsitu.lun = lun_id;
	}

	switch (tnum) {
	case SCATU_TEST_UNIT_READY:
		/* do nothing special */
		break;
	case SCATU_START_STOP_UNIT:
		tucb.scsitu.cmd_timeout = start_stop_to;
		break;
	case SCATU_REQUEST_SENSE:
		/* zero out the request sense data */
		memset(tu_buffer, 0x0ff, 255);
		tucb.scsitu.scsi_cmd_blk[4] = 255;
		tucb.scsitu.data_length = 255;
		tucb.scsitu.data_buffer = tu_buffer;
		tucb.scsitu.flags |= B_READ;
		break;
	case SCATU_READ_CAPACITY:
		/* zero out the data */
		memset(tu_buffer, 0x000, 8);
		tucb.scsitu.data_length = 8;
		tucb.scsitu.data_buffer = tu_buffer;
		tucb.scsitu.flags |= B_READ;
		break;
	case SCATU_RESERVE_UNIT:
		tucb.scsitu.cmd_timeout = 30;
		break;
	case SCATU_MODE_SENSE:
		tucb.scsitu.cmd_timeout = 30;
		tucb.scsitu.flags |= B_READ;
		tucb.scsitu.scsi_cmd_blk[2] = page_code;
		tucb.scsitu.scsi_cmd_blk[4] = 255;
		tucb.scsitu.data_length = 255;
		tucb.scsitu.data_buffer = tu_buffer;
		memset(tu_buffer, 0x0ff, 255);
		break;
	case SCATU_RELEASE_UNIT:
		/* do nothing special */
		break;
	case SCATU_INQUIRY:
		tucb.scsitu.flags |= B_READ;
		tucb.scsitu.scsi_cmd_blk[4] = 255;
		tucb.scsitu.data_length = 255;
		tucb.scsitu.data_buffer = tu_buffer;
		break;
	case SCATU_READ_EXTENDED:
		tucb.scsitu.cmd_timeout = rw_to;
		tucb.scsitu.flags |= B_READ;
		tucb.scsitu.data_length = maxbyte;
		tucb.scsitu.scsi_cmd_blk[2] = (uchar) (curblock >> 24);
		tucb.scsitu.scsi_cmd_blk[3] = (uchar) (curblock >> 16);
		tucb.scsitu.scsi_cmd_blk[4] = (uchar) (curblock >> 8);
		tucb.scsitu.scsi_cmd_blk[5] = (uchar) (curblock);
		tucb.scsitu.scsi_cmd_blk[7] = (uchar) ((maxbyte/512) >> 8);
		tucb.scsitu.scsi_cmd_blk[8] = (uchar) (maxbyte/512);
		tucb.scsitu.data_buffer = tu_buffer;
		break;
	case SCATU_WRITE:
		tucb.scsitu.cmd_timeout = rw_to;
		tucb.scsitu.flags = B_WRITE;
		tucb.scsitu.scsi_cmd_blk[1] = 0x1F & (curblock >> 16);
		tucb.scsitu.scsi_cmd_blk[2] = curblock >> 8 & 0xff;
		tucb.scsitu.scsi_cmd_blk[3] = curblock & 0xff;
		tucb.scsitu.data_buffer = tu_buffer;
		tucb.scsitu.data_length = maxbyte;
		tucb.scsitu.scsi_cmd_blk[4] = maxbyte/512;
		break;
	case SCATU_MODE_SELECT:
                /* If OEM drive, then get the changeable mode data from     */
                /* the drive and compare it against the desired mode data   */
                /* from the ODM data base, then only set the bits that are  */
                /* changeable.                                              */
		if (isSCSD && (odm_info.mode_data_length == 0)) {
			/* If mode_data_length is zero */
			/* then don't issue mode select*/
			return(SCATU_GOOD);
		}
                if (osdisk) {
                        rc = get_sense_data(fdescr, changeable_mode_data,
                                            desired_mode_data,
                                            &mode_data_length);
                        if (rc != 0) {
                                return(rc);
                        }
                        set_default_sense(desired_mode_data,
                                          mode_data_length,
                                          current_mode_data,
                                          changeable_mode_data,
                                          tu_buffer, &sense_length);
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
                tucb.scsitu.flags = B_WRITE;
                tucb.scsitu.scsi_cmd_blk[1] |= 0x10;
                tucb.scsitu.data_length = sense_length;
                tucb.scsitu.scsi_cmd_blk[4] = sense_length;
                tucb.scsitu.data_buffer = tu_buffer;    /* Put data in buff */
                break;
	case SCATU_REASSIGN_BLOCKS:
		tucb.scsitu.cmd_timeout = reassign_to;
		tucb.scsitu.flags |= B_WRITE;
		tucb.scsitu.data_length = 8;
		tucb.scsitu.data_buffer = tu_buffer;
		tu_buffer[0] = 0;
		tu_buffer[1] = 0;
		tu_buffer[2] = 0;
		tu_buffer[3] = 4;
		tu_buffer[4] = ((unsigned) error_block >>24) & 0x00ff;
		tu_buffer[5] = ((unsigned) error_block >>16) & 0x00ff;
		tu_buffer[6] = ((unsigned) error_block >> 8) & 0x00ff;
		tu_buffer[7] = ((unsigned) error_block    ) & 0x00ff;
		break;

	case SCATU_FORMAT_UNIT:
		tucb.scsitu.flags |= B_WRITE;
		/* 10 Meg. will take 60 seconds to format */
		if (!format_unit_to)
			tucb.scsitu.cmd_timeout = (drive_info.drive_capacity * 6);
		else
			tucb.scsitu.cmd_timeout = format_unit_to;
		tucb.scsitu.scsi_cmd_blk[1] = 0x18;
		tucb.scsitu.scsi_cmd_blk[2] = 0x00;
		tucb.scsitu.scsi_cmd_blk[3] = 0x00;
		tucb.scsitu.scsi_cmd_blk[4] = 0x01;
		tucb.scsitu.data_length = 4;
		tucb.scsitu.data_buffer = tu_buffer;
		tu_buffer[0] = 0;
		if (osdisk)
			tu_buffer[1] = 0;
		else
			tu_buffer[1] = 0x02; /* Immed. bit set to 1 */
		tu_buffer[2] = 0;
		tu_buffer[3] = 0;
		break;
	case SCATU_WRITE_EXTENDED:
		tucb.scsitu.cmd_timeout = rw_to;
		tucb.scsitu.flags |= B_WRITE;
		tucb.scsitu.data_length = maxbyte;
		tucb.scsitu.scsi_cmd_blk[2] = (uchar) (curblock >> 24);
		tucb.scsitu.scsi_cmd_blk[3] = (uchar) (curblock >> 16);
		tucb.scsitu.scsi_cmd_blk[4] = (uchar) (curblock >> 8);
		tucb.scsitu.scsi_cmd_blk[5] = (uchar) (curblock);
		tucb.scsitu.scsi_cmd_blk[7] = (uchar) ((maxbyte/512) >> 8);
		tucb.scsitu.scsi_cmd_blk[8] = (uchar) (maxbyte/512);
		tucb.scsitu.data_buffer = tu_buffer;
		break;

	case SCATU_USER_DEFINED:

		break;

	default:
		/* error */
		return (SCATU_BAD_PARAMETER);
	}

	rc = exectu(fdescr, &tucb);

        if (osdisk && (rc == SCATU_GOOD) &&
            (tnum == SCATU_MODE_SENSE) && (page_code == 0x3f)) {
                /* Save the devices current mode data. */
                bcopy(tucb.scsitu.data_buffer, current_mode_data,
                       (unsigned)tucb.scsitu.data_buffer[0]+1);
        }

	return (rc);
}				/* end of tu_test() */

/***********************************************************************/
/* Inproper exit from the DA and return an error return code to the   */
/* Daignostic controller.                                             */
/***********************************************************************/

/*
 * NAME: clean_up
 *
 * FUNCTION: Perform what is necessary for an orderly exit from this DA.
 *
 * NOTES: Will release a reservation on the tape being tested if it
 *	is currently reserved.
 *
 * RETURNS: DOES NOT RETURN.
 */


void
clean_up()
{
	extern int      i_reserved_it;
	extern int      fdes;	/* file descriptor of device	 */

	if (is_working) {	/* drive is currently formatting */
		/* we should stop it */
		if (fdes > 0)
			close(fdes);

		/* Reopen device, sending SCSI device reset */
		fdes = openx(devname, O_RDWR, NULL, SC_FORCED_OPEN);
	} else if (i_reserved_it) {
		/* at least try to release the device */
		(void) tu_test(fdes, SCATU_RELEASE_UNIT);
	}

	if (fdes > 0)
		close(fdes);

	if (pvid_destroyed)
		/* Clear pvid in data base */
		clear_pvid(cudv_selected->name);

	if (init_config_state >= 0) {
		if (initial_state(init_config_state, cudv_selected->name)
		    == -1) {
			disp_menu(DEVICE_CONFIGURE);
		}
	}
	if (init_adapter_state >= 0) {
		if (initial_state(init_adapter_state, cudv_selected->parent)
		    == -1) {
			disp_menu(ADAPTER_CONFIGURE);
		}
	}

	if (cudv_selected)
		odm_free_list(cudv_selected, &obj_info);

	term_dgodm();
	diag_asl_quit();
	dt("certify", DT_END);
	exit(0);
}


/*
 * NAME: base_path
 *
 * FUNCTION: Return a pointer into the input string that points to the
 *	last word.  Words are separated by "/".
 *	Used to determine execution program name.
 *
 * RETURNS: pointer to last word in input string.
 */

char           *
base_path(source)		/* just get base of the name	 */
char           *source;
{
	int             source_len;	/* length of source */
	char           *dest;	/* will point to base of name */

	source_len = strlen(source);
	dest = source + source_len - 1;
	if (*dest == '/')
		return (0);
	if (source_len < 2)
		return (dest);
	while (--source_len && (*(dest - 1) != '/'))
		--dest;
	return (dest);
}

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
int	fdes;
char	*changeable_mode_data;
char	*desired_mode_data;
int	*mode_data_length;
{
	int	rc;
	int	sense_length;
	char	criteria[80];

	page_code=0x7f; /* Report changeable mode data */
	rc = tu_test(fdes, SCATU_MODE_SENSE);
	if (rc != 0)
		return(rc);
	sense_length=tu_buffer[0];
	bcopy(tu_buffer, changeable_mode_data, sense_length);
	sprintf(criteria, "DType = osdisk AND attribute = deflt_mode_data");
	get_diag_att("osdisk", "deflt_mode_data", 'b', &sense_length,
	    desired_mode_data);
	*mode_data_length=sense_length;
	return(0);
}



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
		rc=exectu(tu_fdes, &tucb);

		return(rc);
	}
	else {
                return (SCATU_BAD_PARAMETER);
	}

}
