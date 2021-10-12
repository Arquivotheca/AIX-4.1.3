static char sccsid[] = "@(#)33  1.20.1.16  src/bos/diag/da/cdrom/dcdrom.c, dacdrom, bos41J, 9512A_all 3/17/95 08:49:58";
/*
 *   COMPONENT_NAME: DACDROM
 *
 *   FUNCTIONS: bld_srn
 *              check_menu
 *              clean_up
 *              do_ela
 *              get_dev_type
 *              init_scsi
 *              int_handler
 *              main
 *              seq_check
 *              tu_test
 *              getIndex
 *		tu_issue_scsi_inquiry
 *
 *   ORIGINS: 27, 83
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */


/* #define DEBUGSCSI */

#include <errno.h>              /* common error id's     */
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <memory.h>
#include <nl_types.h>
#include <signal.h>
#include <stdio.h>
#include <sys/cdrom.h>
#include <sys/cfgodm.h>
#include <sys/devinfo.h>
#include <sys/errids.h>         /* error log id's        */
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/types.h>

#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/diag_exit.h"
#include <diag/scsi_atu.h>
#include <diag/scsd.h>

#include "diag/tm_input.h"
#include "diag/tmdefs.h"        /* diagnostic modes and variables  */

#include "dcdrom_msg.h"
#include "dcdrom.h"


/* The following macro definitions are used to understand the test mode
 * environment.
 */

#define CONSOLE ((int)(tm_input.console == CONSOLE_TRUE))
#define SYSTEM ((int)(tm_input.system == SYSTEM_TRUE))
#define NOTLM ((int)(tm_input.loopmode == LOOPMODE_NOTLM))
#define ENTERLM ((int)(tm_input.loopmode == LOOPMODE_ENTERLM))
#define INLM ((int)(tm_input.loopmode == LOOPMODE_INLM))
#define EXITLM ((int)(tm_input.loopmode == LOOPMODE_EXITLM))
#define ELA ((int)(tm_input.dmode == DMODE_ELA))
#define SYSX ((int)(tm_input.exenv == EXENV_SYSX))
#define PD (int) (tm_input.dmode == DMODE_PD)
#define GENERIC_ERROR	-1
#define MAX_TEMP_ERR	1

#define SET_FRUB_ONLY(bType, rMsg, rCode) \
	frub[bType].rmsg=rMsg;\
	frub[bType].rcode=rCode;

#define SET_FRUB_AND_EXIT(bType, rMsg, rCode) \
	SET_FRUB_ONLY(bType, rMsg, rCode); \
	bld_srn(bType);

/* if adding more steps to this "enum", add them before 'S_MAX_STEP' */
enum {	S_RESERVE_UNIT, S_RESERVE_UNIT_REQ, S_LOOP_BACK,
	S_SIX, S_INQUIRY, S_TEST_UNIT_READY,
	S_SEND_DIAGNOSTIC, S_SEARCH_AUDIO_TRACK, S_PLAY_AUDIO,
	S_PLAY_AUDIO_TRACK_INDEX, S_CHECK_TONE, S_MODE_SENSE, 
	S_MODE_SELECT, S_AUDIO_TRACK_SEARCH_TUR,
	S_AUDIO_TRACK_SEARCH, S_MODE_SELECT_TUR, S_MAX_STEP};

#ifdef DEBUGSCSI
char	*step_names[] =
	{"S_RESERVE_UNIT", "S_RESERVE_UNIT_REQ", "S_LOOP_BACK",
	"S_SIX", "S_INQUIRY", "S_TEST_UNIT_READY",
	"S_SEND_DIAGNOSTIC", "S_SEARCH_AUDIO_TRACK", "S_PLAY_AUDIO",
	"S_PLAY_AUDIO_TRACK_INDEX", "S_CHECK_TONE", "S_MODE_SENSE",
	"S_MODE_SELECT", "S_AUDIO_TRACK_SEARCH_TUR",
	"S_AUDIO_TRACK_SEARCH", "S_MODE_SELECT_TUR", "S_MAX_STEP"};
#endif

/* if adding more error code to this "enum", add them before 'MAX_ERR_CODE' */
enum {ERR_TIMEOUT, ERR_RESERVE_CONFLICT, ERR_CHECK_CONDITION,
	ERR_COMMAND_ERROR, MAX_ERR_CODE};

#define	SCATU_MAX_ERR	11

struct	List {
		int	num;
		int	val[SCATU_MAX_ERR];
		};

struct	List	scatu_list =
	{ SCATU_MAX_ERR,
	 	{SCATU_GOOD, SCATU_TIMEOUT, SCATU_RESERVATION_CONFLICT,
		SCATU_CHECK_CONDITION, SCATU_COMMAND_ERROR, SCATU_BUSY,
		SCATU_BAD_REQUEST_SENSE_DATA, SCATU_NONEXTENDED_SENSE,
		SCATU_IO_BUS_ERROR, SCATU_ADAPTER_FAILURE,
		SCATU_BAD_PARAMETER
		}
	};

/* The following array is to hold the status of the steps performed */
int	step_status_buff[S_MAX_STEP][MAX_ERR_CODE];

/*    da_fru structure holds informations for the FRU bucket    */
/*    is initialized to a default value.                        */

#define MAX_FRU_ENTRY   3

struct fru_bucket frub[] =
{
/* 0 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 90, "", "", DA_NAME, EXEMPT },
                       { 10, "", "", PARENT_NAME, EXEMPT },
                },
        },
/* 1 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 100, "", "", DA_NAME, EXEMPT },
                },
        },
/* 2 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 80, "", "", DA_NAME, EXEMPT },
                       { 10, "", "", PARENT_NAME, EXEMPT },
                       { 10, "", "", msgSoftware, NOT_IN_DB,EXEMPT},
                },
        },
/* 3 */ { "", FRUB1,  0x0 ,  0x0 ,  0 ,
                {
                       { 70, "", "", msgTestDisc, NOT_IN_DB,EXEMPT },
                       { 30, "", "", DA_NAME, EXEMPT },
                },
        }
};

/* Following is a structure that holds informations to display a menu.
 * It is initialized with the first menu number to be displayed.
 */

int             msgnum = MENU_ONE;	/* msg number */
int             fdes=0;			/* file descript. for Device Driver */
SCSI_TUTYPE     tucb_ptr;		/* TU structure pointer             */
unsigned char   scsi_buffer[2336];	/* Used to hold sense data */
unsigned char   current_mode_data[255];
unsigned char   scsd_mode_data_buffer[255];
int		scsd_mode_data_length = 0;
int		global_mode_data_length = 0;

struct tm_input tm_input;       	/* To hold the da mode              */
char            devname[512];		/* special file for device */
int             page_code;      /* What to set for PAGE CODE                */
static int      init_adapter_state = -1;
static int      init_config_state = -1;
static int      have_disk = 0;
static int      i_reserved_it = 0;
static int      is_other_scsi = FALSE;
int             led_num = 0;
int		non_xa_mode = FALSE;
int		has_caddy = FALSE;
int             step;
uchar		firstTimeAudioTrackSearchTUR = TRUE;
uchar		lun_id;
uchar		scsi_id;
int		is_scsd_device = FALSE;
uchar           scsd_media_pn[SCSD_CD_MEDIA_PN];

/* Global variables for command time outs */
int 		send_diag_to = 30;	/* send diagnostic comamnd time out   */

/* Catalog file descriptor and diag catalog routines. */
nl_catd catd;
extern nl_catd diag_catopen();

extern          getdainput();	/* to get a local copy of tm_input  */
extern          disp_menu();	/* to display menus                 */
extern          scsitu_init();	/* to initialize SCSI               */
extern          exectu();	/* to exec the TU's                 */
extern 		diag_get_sid_lun(); 
extern		set_default_sense(); /* mask off unchangeable mode data */

#ifdef DEBUGSCSI
FILE            *debug_fdes;
char            debug_fname[256];
#endif

/*  */
/*
 * NAME: main()
 *
 * FUNCTION: Main routine for the CDROM DRIVE Diagnostic Application.
 *           It is invoked by the diagnostic Controller.
 *
 * NOTES:   This function gets the input from the file tm_input.
 *
 * RETURN VALUE :  DDOPEN - the device driver could not open
 *                 GOOD   - the testing was completed successfully
 *                 BAD    - the testing was not completed  successfully
 *
 */
main()
{
        int		i, j;
        int             rc;
        int             get_dev_type();
        void            clean_up();
        void            int_handler(int);
        struct sigaction  act;
	struct  cd_scsd_inqry_data *scsd_vpd;
	struct  cd_scsd_mode_data  *scsd_mode_data;	


        setlocale(LC_ALL,"");
        /* set up interrupt handler     */

        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        /*
         * set up the odm
         */
        if (init_dgodm() == -1) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* init DC return codes  */
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_USER(DA_USER_NOKEY);
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_MORE(DA_MORE_NOCONT);
        DA_SETRC_ERROR(DA_ERROR_NONE);

        /*
         * Get the da mode from ODM and build a bit field that holds it
         * in da mode.
         */
        if ((rc = getdainput(&tm_input)) < 0) {
                /* in case ODM does not respond. */
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }
	/*
	 * If in Sysx termination pass, nothing to execute.
	 */
	if(SYSX && EXITLM) {
		tm_input.console = CONSOLE_FALSE;
		clrdainput();
		clean_up();
	}

	memset(scsd_media_pn, '\0',SCSD_CD_MEDIA_PN);

#ifdef DEBUGSCSI
        sprintf(debug_fname,"%s%s","/tmp/dbug.",tm_input.dname);
        debug_fdes = fopen(debug_fname,"a");
        fprintf(debug_fdes,"-------- start of test --------\n");
        fprintf(debug_fdes,"SYSX = %s\n", (SYSX)?"TRUE":"FALSE");
        fprintf(debug_fdes,"LOOPMODE_ENTERLM= %s\n", (ENTERLM)?"TRUE":"FALSE");
        fprintf(debug_fdes,"LOOPMODE_INLM = %s\n", (INLM)?"TRUE":"FALSE");
#endif

        for(i=0;i<=MAX_FRU_ENTRY;i++)
                strcpy(frub[i].dname, tm_input.dname);

        /*
         * set up asl
         */
        if (CONSOLE) {
		/* Initialize ASL with type ahead param. */
                if (INLM || SYSTEM) {
                        /* Allow type ahead to catch Cancel or Exit. */
                        rc = diag_asl_init("DEFAULT");
                } else {
                        /* Dont allow any type ahead. */
                        rc = diag_asl_init("NO_TYPE_AHEAD");
                }
                if (rc == -1){
#ifdef DEBUGSCSI
			fprintf(debug_fdes, "ASL init error\n");
#endif
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                }
                /* Open catalog file for messages. */
                catd = diag_catopen("dcdrom.cat",0);
                if ((int) catd == -1) {
#ifdef DEBUGSCSI
			fprintf(debug_fdes, "Catalog open error\n");
#endif
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                }
        }

        /* determine device type */
        led_num = get_dev_type();
        switch (led_num){
	case ORIG_CDROM:             /* IBM - not xa mode. requires a caddy */
                non_xa_mode = TRUE;
		has_caddy = TRUE;
		break;
	case ATL_CDROM:              /* IBM - xa mode. requires a caddy */
		has_caddy = TRUE;
		break;
        case OTHER_SCSI:  /* OEM - we don't know, so don't run certain tests */
                is_other_scsi = TRUE;
		break;
        case GENERIC_ERROR:          
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
		break;
	default:                     /* IBM - xa mode. has a tray. */
		break;
	}

        if (EXITLM) {
                if (CONSOLE)
                        rc = disp_menu(LAST_LOOP_MENU);

	/* get have_disk so can put remove disk menu if appropriate */

                getdavar(tm_input.dname, "have_disk",
                        DIAG_INT, &have_disk); 
                sleep(2);

                DA_SETRC_STATUS(DA_STATUS_GOOD);
                DA_SETRC_ERROR(DA_ERROR_NONE);
                clean_up();
        }

        if (((ELA) || (PD)) && ((NOTLM) || (ENTERLM)))
		(void) do_ela();

        /**********************************
         * If not ELA, perform the tests. *
         **********************************/

        if (!(ELA)) {
                /* Display the proper menu before config. */
                if (!(SYSTEM) && (CONSOLE)) {
                        if (NOTLM || ENTERLM)
                                rc=disp_menu(MENU_CHECKOUT);
                        if (INLM) {
                                getdavar(tm_input.dname, "have_disk",
                                        DIAG_INT, &have_disk);
                                if (!SYSX)
                                        disp_menu(LOOP_MODE_MENU);
                        }
                } else {
                        /* System Checkout, Sysx, or No Console mode. */
                        have_disk = 0;
                        if (CONSOLE) {
                                if (NOTLM)
                                        rc = disp_menu(MENU_CHECKOUT);
                                else {
                                        rc = disp_menu(LOOP_MODE_MENU);
                                        sleep(2);
                                }
                        }
                        if (SYSX) {
                                getdavar(tm_input.dname, "have_disk",
                                         DIAG_INT, &have_disk);
                        }
                }

                init_adapter_state = configure_device(tm_input.parent);
                if (init_adapter_state == -1){
                        /* does not return */
			SET_FRUB_AND_EXIT(2, msgConfigFailed, 0x0112);
                }
                init_config_state = configure_device(tm_input.dname);
                if (init_config_state == -1){
                        if (tm_input.dmode != DMODE_REPAIR) {
                                do_ela();
                                clean_up();     /* does not return */
                        }
                        /* does not return */
			SET_FRUB_AND_EXIT(2, msgConfigFailed, 0x0112);
                }
                /*
                 * Try to open the CD ROM Device Driver.
                 */
                strcpy(devname, "/dev/");
                strcat(devname, tm_input.dname);
                fdes = openx(devname, O_RDONLY, NULL, SC_DIAGNOSTIC);
                if (fdes < 0) {
                          switch (errno) {
                          case EACCES:
                          case EBUSY:
                                    do_ela();
                                    DA_SETRC_ERROR(DA_ERROR_OPEN);
                                    clean_up();
                                    break;
                          /* EIO must result in SRN */
                          case EIO:
                          case ENXIO:
                        	    /* does not return */
				    SET_FRUB_AND_EXIT(2, msgDeviceDriverOpenFailed, 0x0173);
                                    break;
                          default:
                                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                                    clean_up();
                                    break;
                          }
                }

		if (is_scsd_device) {
			/* This is an SCSD device */
			
			
			/* 
			  Get main SCSD information
			  for this device. 
			  */
			
			rc = tu_issue_scsi_inquiry(fdes,
						   SCATU_INQUIRY, 
						   0xc7);
			if (rc == SCATU_GOOD) {
				scsd_vpd = (struct cd_scsd_inqry_data *) scsi_buffer;
				

				if (scsd_vpd->scsi_support_flags & SCSD_CD_PLAY_AUDIO_FLG) {
					/* Device supports Play Audio command set */
					non_xa_mode = FALSE;
				}
				else {
					/* Device does not support Play Audio command set */
					non_xa_mode = TRUE;
				}

				/*
				  Set SCSD time out values
				  */
				if (scsd_vpd->send_diag_timeout)
					send_diag_to = scsd_vpd->send_diag_timeout;

				/* Extract Test Media Part Number */
				
				bcopy(scsd_vpd->test_media_pn,scsd_media_pn,SCSD_CD_MEDIA_PN);
				
			}
			else {
				clean_up();
				
			}
			
			/* 
			  Get SCSD desired mode data
			  for this device. 
			  */
			
			rc = tu_issue_scsi_inquiry(fdes,
						   SCATU_INQUIRY, 
						   0xc8);
			if (rc == SCATU_GOOD) {
				scsd_mode_data = (struct cd_scsd_mode_data *) scsi_buffer;
				scsd_mode_data_length = scsd_mode_data->page_length;
				bcopy(scsd_mode_data->mode_data,
				      scsd_mode_data_buffer,
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
				scsd_mode_data_length = 0;
				scsd_mode_data_buffer[0] = '\0';
			}
		}
                /* Ask for test disc */
                if (!(SYSTEM) && (CONSOLE)) {
                        /* System False, Console True */
                        if (NOTLM || ENTERLM) {
                                if (SYSX) {
                                        if (!(is_other_scsi) &&
					tm_input.advanced == ADVANCED_TRUE)
                                                have_disk=disp_menu(EXER_MENU);
                                        if (have_disk) {
						if (has_caddy)
                                                	disp_menu(EXER_INSERT);
						else
							disp_menu(EXER_INSERT_TRAY);
						}
                                        diag_asl_quit();
					tm_input.console = CONSOLE_FALSE;
                                        /* Tell the cntrl to cont. on */
                                        clrdainput();
                                } else {
                                        rc = 0;
                                        msgnum = MENU_ONE;
                                        while (rc != 1)
					{
                                                rc = check_menu();
					}	
                                }
                                if (is_other_scsi)
                                        have_disk = 0;
                                putdavar(tm_input.dname, "have_disk",
                                        DIAG_INT, &have_disk);
				if (SYSX) { /* do not perform the test in
						interaction pass */
					clean_up();
                        	}
                       	}
                }

#ifdef DEBUGSCSI
        fprintf(debug_fdes,"CONSOLE = %s\n", (CONSOLE)? "TRUE":"FALSE");
        fprintf(debug_fdes,"SYSTEM = %s\n", (SYSTEM)? "TRUE":"FALSE");
        fprintf(debug_fdes,"have_disk = %d\n",have_disk);
	fprintf(debug_fdes,"has_caddy = %d\n",has_caddy);
#endif

                /*
                 * init to zero the area that contains the times that steps
                 * are run.
                 */
                for (i = 0; i < S_MAX_STEP; i++)
                	for (j = 0; j < MAX_ERR_CODE; j++)
                        	step_status_buff[i][j] = 0;

                /* Starting from step 1, go to the execution of steps */
                step = S_RESERVE_UNIT;
                while (step != S_MAX_STEP) {
                        rc = seq_check(step);
                        step = rc;
                }
                if (CONSOLE)
                        disp_menu(CHK_ASL_READ);
        }
        /*
         * the GOOD return Code is then set to return to the Controller
         */
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        clean_up();
}
/*  end of M A I N  routine                                         */
/*  */
/*
 * NAME: seq_check()
 *
 * FUNCTION: Test sequence as outlined in the SCSI CD-ROM Problem
 *           Determination. According to the step number received in
 *           input by the main function, this function, may call the
 *           check_menu() or the tu_exec() function.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS:This function will return the step number
 *                           just executed.
 *
 */

seq_check(step)
        int             step;   /* current step in execution */
{
        int             rc = 0; /* will contain the return code from the TU */
        int             response;
        int             sense_key;
        int             sense_code;
        int		j;
        /*
         * to get a different choice of the confidence level to be passed
         * to the srn function.
         */
        static int      disp_media_msg = 0;
	static int	scatu_index;


#ifdef DEBUGSCSI
        fprintf(debug_fdes,"step = %s\n",step_names[step]);
#endif
        switch (step) {
        case S_RESERVE_UNIT:
                rc = tu_test(SCATU_RESERVE_UNIT);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                        i_reserved_it = 1;
                        step = S_INQUIRY;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
			if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
                        /* was the disc changed ? */
                        if ((tucb_ptr.scsiret.sense_code & 0x0ff00)==0x02800){
                                if (step_status_buff[step][scatu_index] <= 2)
                                        step = S_RESERVE_UNIT;
                        	else {
					SET_FRUB_AND_EXIT(0, msgCdromError, 0x0121);
                        	}
                        } else if (step_status_buff[step][scatu_index] <= 3) {
                        	if (tucb_ptr.scsiret.sense_key == 6) {
                               		step = S_RESERVE_UNIT;
                        	} else {
					SET_FRUB_AND_EXIT(1, msgCdromError, 0x0122);
                        	}
                	} else {        /* exit */
				SET_FRUB_AND_EXIT(1, msgCdromError, 0x0123);
			}
                        break;
                case SCATU_COMMAND_ERROR:
                default:
                        if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(0, msgUnknownErrorFound, 0x0199);
			}
                        break;
                }
                break;
        case S_INQUIRY:
                rc = tu_test(SCATU_INQUIRY);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                        step = S_TEST_UNIT_READY;
                        break;
                case SCATU_TIMEOUT:
			if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_COMMAND_ERROR:
                case SCATU_CHECK_CONDITION:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_TEST_UNIT_READY:
                rc = tu_test(SCATU_TEST_UNIT_READY);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                        step = S_SEND_DIAGNOSTIC;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
                        if (!(have_disk))
                                step = S_MAX_STEP;
                        else {
                        	sense_key = tucb_ptr.scsiret.sense_key & 0x0f;
                        	sense_code = tucb_ptr.scsiret.sense_code;
                        	switch(sense_code) {
                        	case 0x2800:   /* hardware problem */
                        	case 0x6400:   /* hardware problems */
                                	if (step_status_buff[step][scatu_index] == 1) {
                                        	step = S_TEST_UNIT_READY;
                                	} else {
						SET_FRUB_AND_EXIT(1, msgCdromError, 0x0128);
					}
                                	break;
                        	case 0x3000:/* incompatible disk installed */
                        	case 0x3001:/* Unknown Format */
                        	case 0x3002:/* Incompatible Format */
                        	case 0x5700:/* cannot recov table of contents */
                        	case 0x3a00:/* Medium not present */
                        	case 0x0400:/* Drive not ready */
                        	case 0x0401:
                                	if ((CONSOLE) && !disp_media_msg) {
						if (has_caddy)
                                        		response = disp_menu(NEW_MEDIA_MSG);
						else
                                        		response = disp_menu(NEW_MEDIA_MSG_TRAY);

						++disp_media_msg;
                                        	step = S_TEST_UNIT_READY;
                                	} else {
						SET_FRUB_AND_EXIT(3, msgMediaError, 0x0150);
					}
                                	break;
                        	default:
                                	if (step_status_buff[step][scatu_index] == 1) {
                                        	step = S_TEST_UNIT_READY;
                                	} else {
						SET_FRUB_AND_EXIT(1, msgCdromError, 0x0127);
                                	}
                                	break;
                        	}
                       	}
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_SEND_DIAGNOSTIC:
                rc = tu_test(SCATU_SEND_DIAGNOSTIC);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                        step = S_AUDIO_TRACK_SEARCH_TUR;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3) {
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
                        sense_key = tucb_ptr.scsiret.sense_key & 0x0f;
                        sense_code = tucb_ptr.scsiret.sense_code;
                        if ((sense_code == 0x3000) ||
                        	(sense_code == 0x3A00) ||
                                (sense_code == 0x4200) ||
                                (sense_code == 0x5700) ||
                                (sense_key == 0x03) ||
                                (sense_key == 0x04)) {
                                if (CONSOLE && !disp_media_msg) {
                                /* Check media and retry. */
					if (has_caddy)
                                		response = disp_menu(NEW_MEDIA_MSG);
					else
                                                response = disp_menu(NEW_MEDIA_MSG_TRAY);
					++disp_media_msg;
                                        sleep(10);
                                        step = S_INQUIRY;
                                } else {
					SET_FRUB_AND_EXIT(3, msgMediaError, 0x0150);
                                }
                        } else if ((sense_code == 0x2800) ||
                                (sense_key == 0x06)) {
                                if (step_status_buff[step][scatu_index] <= 2) {
                                        /* Retry SEND DIAG */
                                        step = S_SEND_DIAGNOSTIC;
                                }
                        } else {
				SET_FRUB_AND_EXIT(1, msgCdromError, 0x0127);
                        }
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
	case S_AUDIO_TRACK_SEARCH_TUR:
                if ((INLM) || (SYSX) ||
                    (SYSTEM) || !(CONSOLE) || !(have_disk)) {
                        /* skip the rest */
                        sleep(2);
                        step = S_MAX_STEP;
                        break;
                }
                /*
                 * Menus are to be displayed only one time
                 */
                if (firstTimeAudioTrackSearchTUR){
			firstTimeAudioTrackSearchTUR = FALSE;
                        msgnum = MENU_NINE;	/* stereo head set available? */
                        rc = 0;
                        while (rc != 1)
                                rc = check_menu();
                }

                rc = tu_test(SCATU_TEST_UNIT_READY);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                	/* Determine what step to go next based on drive type */
                        step = non_xa_mode ? S_AUDIO_TRACK_SEARCH : S_PLAY_AUDIO_TRACK_INDEX;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
			}
                        break;
                case SCATU_CHECK_CONDITION:
			switch (tucb_ptr.scsiret.sense_key) {
			case 6:
				if (step_status_buff[step][scatu_index] <= 1)
					step = S_RESERVE_UNIT;
				else {
					SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
				}
				break;
			case 1:
                        	step = non_xa_mode ? S_AUDIO_TRACK_SEARCH : S_PLAY_AUDIO_TRACK_INDEX;
				break;
			default:
				SET_FRUB_AND_EXIT(1, msgMediaError, 0x0150);
				break;
			}
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_AUDIO_TRACK_SEARCH:
                rc = tu_test(SCATU_AUDIO_TRACK_SEARCH);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                        step = S_PLAY_AUDIO;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
			}
                        break;
                case SCATU_CHECK_CONDITION:
			switch (tucb_ptr.scsiret.sense_key) {
			case 6:
				if (step_status_buff[step][scatu_index] <= 1)
					step = S_RESERVE_UNIT;
				else {
					SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
				}
				break;
			case 1:
                        	step = S_PLAY_AUDIO;
				break;
			default:
				SET_FRUB_AND_EXIT(1, msgMediaError, 0x0150);
				break;
			}
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_PLAY_AUDIO:                
                rc = tu_test(SCATU_PLAY_AUDIO);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                	step = S_CHECK_TONE;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                        	SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                       	}
                        	break;
                case SCATU_RESERVATION_CONFLICT:
			if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
			}
                        break;
                case SCATU_CHECK_CONDITION:
			switch (tucb_ptr.scsiret.sense_key) {
			case 6:
				if (step_status_buff[step][scatu_index] <= 1)
					step = S_RESERVE_UNIT;
				else {
					SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
				}
				break;
			case 1:
				step = S_CHECK_TONE;
				break;
			default:
				SET_FRUB_AND_EXIT(1, msgMediaError, 0x0150);
				break;
			}
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_PLAY_AUDIO_TRACK_INDEX:
                rc = tu_test(SCATU_PLAY_AUDIO_TRACK_INDEX);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                	step = S_CHECK_TONE;
                        break;
                case SCATU_TIMEOUT:
                       	if (step_status_buff[step][scatu_index] > 1){
                               	SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                       	}
                       	break;
                case SCATU_RESERVATION_CONFLICT:
                       	if (step_status_buff[step][scatu_index] > 3){
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
			}
                        break;
                case SCATU_CHECK_CONDITION:
			switch (tucb_ptr.scsiret.sense_key) {
			case 6:
				if (step_status_buff[step][scatu_index] <= 1)
					step = S_RESERVE_UNIT;
				else {
					SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
				}
				break;
			case 1:
				step = S_CHECK_TONE;
				break;
			default:
				SET_FRUB_AND_EXIT(1, msgMediaError, 0x0150);
				break;
			}
                        break;
		case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        case S_CHECK_TONE:
                /*
                 * Menus are to be displayed to ask user if a tone was
                 * present.
                 */
                msgnum = MENU_THIRTEEN;
                rc = check_menu();
		if (non_xa_mode)
			step = S_MAX_STEP;
		else
                        step = S_MODE_SELECT_TUR;
                break;
        case S_MODE_SELECT_TUR:
                rc = tu_test(SCATU_TEST_UNIT_READY);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
                switch (rc) {
                case SCATU_GOOD:
                	step = S_MODE_SENSE;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3) {
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
			if (step_status_buff[step][scatu_index] > 3) {
                                SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        }
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
		break;
	case S_MODE_SENSE:
		/* get default mode data */
		page_code = 0x3f;
                rc = tu_test(SCATU_MODE_SENSE);
                scatu_index = getIndex(scatu_list, rc);
                step_status_buff[step][scatu_index]++;
 
                switch (rc){
                case SCATU_GOOD:
			step = S_MODE_SELECT;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3) {
                                SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
                        if (step_status_buff[step][scatu_index] > 3) {
                                SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        }
                        break;
                case SCATU_COMMAND_ERROR:
                default:
                        SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
		break;
        case S_MODE_SELECT:
		/* get changeable mode data, stored mode data from ODM and */
		/* modify mode data if bits changeable then do mode select */
                rc = tu_test(SCATU_MODE_SELECT);
		scatu_index = getIndex(scatu_list, rc);
		step_status_buff[step][scatu_index]++;
		switch (rc){
                case SCATU_GOOD:
                	step = S_MAX_STEP;
                        break;
                case SCATU_TIMEOUT:
                        if (step_status_buff[step][scatu_index] > 1){
                                SET_FRUB_AND_EXIT(0, msgTimeout, 0x0151);
                        }
                        break;
                case SCATU_RESERVATION_CONFLICT:
                        if (step_status_buff[step][scatu_index] > 3) {
				SET_FRUB_AND_EXIT(1, msgReserveConflict, 0x0152);
                        }
                        break;
                case SCATU_CHECK_CONDITION:
			if (step_status_buff[step][scatu_index] > 3) {
                                SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        }
                        break;
                case SCATU_COMMAND_ERROR:
                default:
			SET_FRUB_AND_EXIT(1, msgUnknownErrorFound, 0x0199);
                        break;
                }
                break;
        default:
                step = S_MAX_STEP;
                break;
        }                       /* end of big step switch */

        return (step);
}                               /* end of seq_check()  */
/*  */
/*
 * NAME: tu_test()
 *
 * FUNCTION: Will first initialize the structure to be passed to the
 *           called TU, with a basic external function. Than will call
 *           the internal initializin function to complete the
 *           initialization and finally will call the TU execution
 *           function.
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: The return code from the TU, is
 *                            zero, in case of no errors, or is a
 *                            negative integer value ranging from
 *                            -10 to -99.
 *
 */
int
tu_test(tstptr)
        int             tstptr;
{
        int             rc;
        int             rs_rc;
        int             i;

	tucb_ptr.header.tu = tstptr;
	tucb_ptr.header.mfg = 0;
	(void) scsitu_init(&tucb_ptr);
	(void) init_scsi(tstptr);
	if ((global_mode_data_length == 0 ) &&
	    (tstptr == SCATU_MODE_SELECT)) {
		/* If the mode data length is 0    */
		/* then don't issue the mode select*/
		step = S_MAX_STEP;

	}
	rc = exectu(fdes, &tucb_ptr);

#ifdef DEBUGSCSI
        fprintf(debug_fdes,"\ttu_num = %d ,\trc = %d\n", tstptr, rc);
#endif

        if ((rc == SCATU_GOOD) &&
            (tstptr == SCATU_MODE_SENSE) && (page_code == 0x3f)) {
                /* Save the devices current mode data. */
                bcopy(tucb_ptr.scsitu.data_buffer, current_mode_data,
                       (unsigned)tucb_ptr.scsitu.data_buffer[0]+1);
        }

	if (rc == SCATU_CHECK_CONDITION){
		for (i=0; i < MAX_REQ_TRY; i++){
			rs_rc = tu_test(SCATU_REQUEST_SENSE);
			if (rs_rc == SCATU_GOOD)
				break;
		}
		if (rs_rc != SCATU_GOOD)
			return(SCATU_BAD_REQUEST_SENSE_DATA);

#ifdef DEBUGSCSI
                fprintf(debug_fdes,"\t\tsense_key = %X\tsense_code = %X\n",
                        (tucb_ptr.scsiret.sense_key & 0x0f),
			tucb_ptr.scsiret.sense_code);
#endif
	}

        return (rc);
}                               /* end of tu_test()  */
/*  */
/*
 * NAME: get_dev_type()
 *
 * FUNCTION: Looks at the led value found in the PdDv structure in the odm
 *           data base. The function searches the Customized Devices Data
 *           base for the name passed in tm_input.dname. It then uses the
 *           link found in the PdDvLn entry to go to the Predefined Devices
 *           data base to retreive the value found in the led entry.
 *           This is neccessary to set up the test parameters for each
 *           fixed-disk type that may be tested and to create the correct
 *           fru bucket in the event a test indicates a drive failure.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: This function returns the led value if found or and error
 *          code if the data was not found.
 */

int
get_dev_type()
{
        char                    odm_search_crit[80];
        struct CuDv             *cudv;
	struct CuAt		*cuat_ptr;
        struct listinfo         obj_info;
	int                     led;
	uchar  			operation_flags;  
	int			rc,bc;
/*	extern int		diag_get_sid_lun(char *, uchar *, uchar *); */

        sprintf(odm_search_crit, "name = %s", tm_input.dname);
        cudv = get_CuDv_list(CuDv_CLASS, odm_search_crit, &obj_info, 1, 2);
        if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL))
                return(GENERIC_ERROR);

	if (diag_get_sid_lun(cudv->connwhere, &scsi_id, &lun_id) == -1)
		return(GENERIC_ERROR);

	led = cudv->PdDvLn->led;
	if(led == 0) {
		/* No LED number in PdDv so check for type Z */
		/* attribute                                 */
		
		
		if ((cuat_ptr = (struct CuAt *)getattr(cudv->name,
						       "led",0,&bc)) 
		    == (struct CuAt *)NULL) {
			
			/* Error from CuAt */
			return(GENERIC_ERROR);
		}	
		led = (int)strtoul(cuat_ptr->value,NULL,0);					
	} 

	if (!strcmp(cudv->PdDvLn->type,"scsd")) {

		/* This is an SCSD device */

		is_scsd_device = TRUE;

		rc = diag_get_scsd_opflag(cudv->name,&operation_flags);
		if (rc == -1)
			return (GENERIC_ERROR);
		
		
		if (operation_flags & SCSD_TRAY_FLG) {
			 has_caddy = FALSE;
		}
		else {
			 has_caddy = TRUE;
		}		
	}
	odm_free_list(cudv, &obj_info);
	return(led);
} /* end function get_dev_type */

/*  */
/*
 * NAME: check_menu()
 *
 * FUNCTION: This is the routine that calls an external function
 *           to display menus to the user and analize the user response.
 *
 * NOTES:    Will first read the value that are written in the structure
 *           msgnum, that specify the menu number to be displayed.
 *           Returning, the next menu will be written in the same field.
 *
 * RETURN VALUE DESCRIPTIONS: this function returns the integer 0 if
 *                            other menus are to be displayed. It
 *                            returns 1 if no more menus are needed.
 */
check_menu()
{
        int             rc = 0;		/* to return to the caller */
        int             j = 0;
        int             response;       /* will contain the user selection */

        /*
         * Following are the test environment conditions to display menus.
         */
        if (((NOTLM) || ((ENTERLM) && !(SYSTEM))) && !(is_other_scsi)) {
                /* call the external display routine  */
                response = disp_menu(msgnum);

                switch (msgnum) {
                case MENU_ONE:  /* the user had the test disk */
                        if (response == YES_ANS) {
                                have_disk = 1;
				if (has_caddy)
  	                              msgnum = MENU_TWO;
				else
				      msgnum = MENU_FOUR_TRAY;
                        }
                        else {
                                have_disk = 0;
                                rc = 1;
                        }
                        break;
                case MENU_TWO:  /* told the user to insert empty caddy */
                        sleep(10);
                        msgnum = MENU_THREE;
                        break;
                case MENU_THREE:        /* was the LED flashing? */
                        if (response == YES_ANS)
                                msgnum = MENU_FOUR;
                        else {
                        	/* does not return */
				SET_FRUB_AND_EXIT(1, msgResponseError, 0x0211);
                        }
                        break;
               case MENU_FOUR:/* told the user to replace disk */
               case MENU_FOUR_TRAY:
                        sleep(10);
                        rc = 1;
                        break;
                case MENU_NINE:/* was the stereo headset available? */
                        if (response == YES_ANS)
                                msgnum = MENU_TEN;
                        else {
                                /* cannot continue test */
				clean_up();
                        }
                        break;
                case MENU_TEN:  /* plug in the stereo headset */
                        msgnum = MENU_ELEVEN;
                        break;
                case MENU_ELEVEN:       /* the user should hear a tone */
                        msgnum = MENU_ELEVEN;
                        rc = 1;
                        break;
                case MENU_THIRTEEN:       /* was the tone present? */
                        if (response == YES_ANS)
                                rc = 1;
                        else {
                                /* negative response, means error. */
                        	/* does not return */
				SET_FRUB_AND_EXIT(1, msgResponseError, 0x0281);
                        }
                        break;
                default:
                        rc = 1;
                        break;
                }
        } else {
                rc = 1;
        }
        /* in case of LOOP MODE, a message is to be displayed */
        if ((INLM) && (CONSOLE) && !(SYSX)) {
                response = disp_menu(LOOP_MODE_MENU);
        }
        return (rc);
}
                               /* end of check_menu() */

/*  */
/*
 * NAME: init_scsi()
 *
 * FUNCTION: Complete the initialization of the structure to be passed
 *           to the TU's. At first, the common CD-ROM data, then, for
 *           any specific TU.
 *
 * NOTES:    No return code.
 *
 */
init_scsi(tunum)
        int             tunum;
{
        int             rc;             /* functions return code         */
	uchar           changeable_mode_data[255];
        uchar           desired_mode_data[255];
        int             sense_length;
        int             mode_data_length;
	

        /* initialize  transmit data structure with common data    */
        tucb_ptr.scsitu.data_length = 0;
        tucb_ptr.scsitu.data_buffer = NULL;
        tucb_ptr.scsitu.cmd_timeout = 30;       /* default value */
	if (lun_id < 8)
		tucb_ptr.scsitu.scsi_cmd_blk[1] = lun_id << 5;
	else
		tucb_ptr.scsitu.lun = lun_id;
        tucb_ptr.scsitu.flags = 0;
        tucb_ptr.scsitu.ioctl_pass_param = CDIOCMD;

        /* then, initialize according to the specific TU to be run */
        switch (tunum) {
        case SCATU_RESERVE_UNIT:                /* reserve command */
        case SCATU_TEST_UNIT_READY:             /* unit ready command */
                break;
        case SCATU_REQUEST_SENSE:               /* sense command */
                tucb_ptr.scsitu.flags |= B_READ;
                tucb_ptr.scsitu.scsi_cmd_blk[4] = 18;
                tucb_ptr.scsitu.data_length = 18;
                tucb_ptr.scsitu.data_buffer = scsi_buffer;
                memset(scsi_buffer, 0x0ff, 255);
                break;
        case SCATU_SEND_DIAGNOSTIC:             /* send diagnostic command */
                tucb_ptr.scsitu.cmd_timeout = send_diag_to;       /* with 3, don't work */
                tucb_ptr.scsitu.scsi_cmd_blk[1] |= 1 << 2;
                tucb_ptr.scsitu.flags |= B_WRITE;
                break;
        case SCATU_INQUIRY:             /* inquiry command */
                tucb_ptr.scsitu.data_buffer = scsi_buffer;
                memset(scsi_buffer, 0x0ff, 255);
                tucb_ptr.scsitu.flags |= B_READ;
                break;
        case SCATU_PLAY_AUDIO:          /* play audio command */
                tucb_ptr.scsitu.flags |= B_READ;
                tucb_ptr.scsitu.command_length = 10;
                tucb_ptr.scsitu.scsi_cmd_blk[1] |= 3;
                tucb_ptr.scsitu.scsi_cmd_blk[2] |= 3;
                tucb_ptr.scsitu.scsi_cmd_blk[9] |= 0x80;
                break;
        case SCATU_AUDIO_TRACK_SEARCH:          /* search audio track command */
                tucb_ptr.scsitu.flags |= B_READ;
                tucb_ptr.scsitu.command_length = 10;
                tucb_ptr.scsitu.scsi_cmd_blk[1] |= 1;
                tucb_ptr.scsitu.scsi_cmd_blk[2] |= 2;
                tucb_ptr.scsitu.scsi_cmd_blk[9] |= 0x80;
                break;
        case SCATU_PLAY_AUDIO_TRACK_INDEX:/* play audio track index command */
                tucb_ptr.scsitu.flags |= B_READ;
                tucb_ptr.scsitu.command_length = 10;
                tucb_ptr.scsitu.scsi_cmd_blk[4] |= 0x02;
                tucb_ptr.scsitu.scsi_cmd_blk[5] |= 0x01;
                tucb_ptr.scsitu.scsi_cmd_blk[7] |= 0x03;
                tucb_ptr.scsitu.scsi_cmd_blk[8] |= 0x01;
                break;
        case SCATU_MODE_SENSE:
		/* do mode sense first */
                tucb_ptr.scsitu.cmd_timeout = 30;
                tucb_ptr.scsitu.flags |= B_READ;
                tucb_ptr.scsitu.scsi_cmd_blk[2] = page_code;
                tucb_ptr.scsitu.scsi_cmd_blk[4] = 255;
                tucb_ptr.scsitu.data_length = 255;
                tucb_ptr.scsitu.data_buffer = scsi_buffer;
                memset(scsi_buffer, 0x0ff, 255);
                break;
        case SCATU_MODE_SELECT:
		rc = get_sense_data(changeable_mode_data,
			desired_mode_data, &mode_data_length);
		if (rc != 0)
			return (rc);
		set_default_sense(desired_mode_data, mode_data_length,
			current_mode_data, changeable_mode_data,
			scsi_buffer, &sense_length);
		if (( led_num == ATL_CDROM ) || ( led_num == BAND_CDROM ))
			sense_length -= 0x03;
		tucb_ptr.header.tu = SCATU_MODE_SELECT;
		rc = scsitu_init(&tucb_ptr);
		if (rc != SCATU_GOOD)
			return (SCATU_BAD_PARAMETER);
		tucb_ptr.scsitu.ioctl_pass_param = CDIOCMD; 
                tucb_ptr.scsitu.cmd_timeout = 30;
                tucb_ptr.scsitu.flags |= B_WRITE;
                tucb_ptr.scsitu.command_length = 6;  
                tucb_ptr.scsitu.scsi_cmd_blk[1] |= 0x10; 
                tucb_ptr.scsitu.scsi_cmd_blk[4] = sense_length;
                tucb_ptr.scsitu.data_buffer = scsi_buffer;
                tucb_ptr.scsitu.data_length = sense_length;
                break;
        case SCATU_RELEASE_UNIT:
                break;
        default:
                break;
        }
}                               /* end of init_scsi()  */

/*  */
/*
 * NAME: do_ela()
 *
 * FUNCTION:  Error log analysis
 *
 * NOTE: The type attribute defines the type of analysis requested.
 */

do_ela()
{
        int             menu_goal_clean = 0;
        int             rc;
        int             error_found = 0;
        char            srch_crit[80];
	int		op_flg;
        struct  errdata err_data;

        sprintf(srch_crit,"%s -N %s", tm_input.date, tm_input.dname);
	op_flg = INIT;

	do {
                rc = error_log_get( op_flg, srch_crit, &err_data );
                if( rc > 0 ) {
                        switch( err_data.err_id ) {
			case ERRID_DISK_ERR2:  /* Permanent Hardware error */
			case ERRID_DISK_ERR5:
				error_found = 1;
				op_flg = TERMI;
				break;
			case ERRID_DISK_ERR1:  /* Permanent media error */
			case ERRID_DISK_ERR4:  /* Temporary error */
				menu_goal_clean = 1;
				op_flg = TERMI;
				break;
			default: /* Ignore all other errors */
				op_flg = SUBSEQ;
				break;
			}
		}
		else {
			op_flg = TERMI;
		}
	} while (op_flg != TERMI);

        (void) error_log_get(TERMI, srch_crit, &err_data);

	if( error_found == 1) {
                /* Report ELA SRN. */
		SET_FRUB_AND_EXIT(0, msgElaHardwareFailed, 0x0302);
		}

        if ((menu_goal_clean == 1) && (CONSOLE))
                disp_menu(MENU_GOAL_3);

	if (rc < 0)
		clean_up();

}/* end of do_ela()  */

/*  */
/*
 * NAME: bld_srn()
 *
 * FUNCTION: This function is used to build the SRN code.
 *
 * NOTES:     It is called by check_step(), for errors related to the
 *            PD sequence, it is called by check_menu(). For problems
 *            related to the user responses, it is called by chek_menu()
 *            Will not return to the caller, but calls the exit routine
 *            clean_up().
 */
bld_srn(ret_fru_type)
        int             ret_fru_type;
{
        if (CONSOLE)
                disp_menu(CHK_ASL_READ);

        /* basic initialization */
        DA_SETRC_STATUS(DA_STATUS_BAD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        frub[ret_fru_type].sn = led_num;

        /*
         * depending upon the ret_fru_type received from the caller, specific
         * confidence levels, will be returned to the DC.
         */
        switch (ret_fru_type) {
        case 0:         /* Drive, SCSI adapter */
                strncpy(frub[0].frus[0].fname, tm_input.dname, NAMESIZE);
                strncpy(frub[0].frus[1].fname, tm_input.parent, NAMESIZE);
                strncpy(frub[0].frus[0].floc, tm_input.dnameloc, LOCSIZE);
                strncpy(frub[0].frus[1].floc, tm_input.parentloc, LOCSIZE);
                break;

        case 1:         /* Drive only  and FRUB1 only */
                strncpy(frub[1].frus[0].fname, tm_input.dname, NAMESIZE);
                strncpy(frub[1].frus[0].floc, tm_input.dnameloc, LOCSIZE);
                break;
        case 2: /* Drive, SCSI adapter, software */
                strncpy(frub[2].frus[0].fname, tm_input.dname, NAMESIZE);
                strncpy(frub[2].frus[1].fname, tm_input.parent, NAMESIZE);
                strncpy(frub[2].frus[0].floc, tm_input.dnameloc, LOCSIZE);
                strncpy(frub[2].frus[1].floc, tm_input.parentloc, LOCSIZE);
                break;
        case 3:         /* Media error */
                strncpy(frub[3].frus[1].fname, tm_input.dname, NAMESIZE);
                strncpy(frub[3].frus[1].floc, tm_input.dnameloc, LOCSIZE);
                break;
        default:
                break;
        }

        /*
         * call the external function to add the fru bucket info to FRUB1.
         */
        if (insert_frub(&tm_input, &frub[ret_fru_type]) == -1) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
        }
        frub[ret_fru_type].sn = led_num;
        if (addfrub(&frub[ret_fru_type]) == -1) {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
        }

        if (is_other_scsi)
                disp_menu(MSG_UNKNOWN_DEVICE);

        /* exit with return code for Controller */
        clean_up();
}                               /* end of bld_srn()  */


/*  */
/*
 * NAME: clean_up()
 *
 *  This is the only function that allows exit from the DA.
 *  Calling this function, the DA_EXIT parameters are to be defined.
 *  The DA_EXIT parameters will then be passed to the Diag. Controller.
 */
void
clean_up()
{
        int rc;

        if (i_reserved_it) {
                i_reserved_it = 0;	/* needed to break infinite loop WHEN
					   disp_menu() calls clean_up() again */
                rc = tu_test(SCATU_RELEASE_UNIT);
		}

	/* display remove-disk menu only in EXITLM or NOTLM.
	No need for SYSX 'cause SYSX only in ENTERLM or INLM */

        if ((have_disk) && (!(SYSTEM)) && (CONSOLE) &&
                  ((EXITLM) || (NOTLM)))  {
                have_disk = 0;
		if (has_caddy)
                       	disp_menu(REMOVE_THE_DISK);
		else
			disp_menu(REMOVE_THE_DISK_TRAY);
                }

        if (fdes > 0)           /* close the Device Driver if opened */
                close(fdes);

        if (init_config_state >= 0) {
                rc = initial_state(init_config_state, tm_input.dname);
                if (rc == -1) {
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                }
        }
        if (init_adapter_state >= 0) {
                rc = initial_state(init_adapter_state, tm_input.parent);
                if (rc == -1) {
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                }
        }

        term_dgodm();

        if ((CONSOLE) && !(SYSX))
                diag_asl_quit(NULL);

        catclose(catd);

#ifdef DEBUGSCSI
        fprintf(debug_fdes,"end of test\n");
        fclose(debug_fdes);
#endif

        DA_EXIT();
}                               /* end of clean_up()  */

/*  */
/*
 * NAME:  int_handler
 *
 * FUNCTION: Perform clean up on receipt of an interrupt
 *
 * NOTES:
 *
 * RETURNS: None
 *
 */
void int_handler(int sig)
{
        have_disk = 0; 
        clean_up();
}

/*  */
/*
 * NAME:  getIndex
 *
 * FUNCTION: finds index of a key of an integer list
 *
 * NOTES:
 *
 * RETURNS: 
 *	0...list.num-1 : key is found
 *	-1 : key not found
 *
 */
int
getIndex(struct List list, int key)
{
	int	i;

	for (i=0; i < list.num; i++)
		if (list.val[i] == key)
			break;
	if (i >= list.num)
		return(-1);

	return(i);
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
get_sense_data(changeable_mode_data,
desired_mode_data, mode_data_length)
char    *changeable_mode_data;
char    *desired_mode_data;
int     *mode_data_length;
{
        int     rc;
        int     sense_length;
        char                    odm_search_crit[80];
        struct CuDv             *cudv;
        struct listinfo         obj_info;

	/* do mode sense to get changeable values */
	page_code = 0x7f;
        rc = tu_test(SCATU_MODE_SENSE);
        if (rc != 0)
                return(rc);
	bcopy(scsi_buffer, changeable_mode_data, (uchar)scsi_buffer[0]);

	if (is_scsd_device) {
		/* This is a SCSD device */

		bcopy(scsd_mode_data_buffer,desired_mode_data,scsd_mode_data_length);
		*mode_data_length = scsd_mode_data_length;

		
	}
	else {
		/* This is not an SCSD device */
		/* get the uniquetype and then the mode data from PDiagAtt */
		sprintf(odm_search_crit, "name = %s", tm_input.dname);
		cudv = get_CuDv_list(CuDv_CLASS, odm_search_crit, &obj_info, 1, 2);
		if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL))
			return(GENERIC_ERROR);

		sense_length = scsi_buffer[0];
		rc = get_diag_att(cudv->PdDvLn->uniquetype, "mode_data", 'b', &sense_length,
				  desired_mode_data);
		odm_free_list(cudv, &obj_info);
		*mode_data_length=sense_length;
	}
	global_mode_data_length = *mode_data_length;
        return(0);
}

/*  */
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
        SCSI_TUTYPE tucb;              /* scsi command structure  */
        int      rc;                            /* function return code    */

        tucb.header.tu = tnum;            /* put TU number to do in TU struct */
        tucb.header.mfg = 0;             /* always set to 0, only used by mfg */
        rc = scsitu_init (&tucb);
        if (rc != SCATU_GOOD) {
                return (SCATU_BAD_PARAMETER);
        }
        tucb.scsitu.flags = 0;
        tucb.scsitu.ioctl_pass_param = CDIOCMD;
        memset(scsi_buffer, 0x00, 1024);
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
                tucb.scsitu.data_buffer = scsi_buffer;
		rc=exectu(tu_fdes, &tucb);

		return(rc);
	}
	else {
                return (SCATU_BAD_PARAMETER);
	}
}
