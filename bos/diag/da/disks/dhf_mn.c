static char sccsid[] = "@(#)45  1.9.1.5  src/bos/diag/da/disks/dhf_mn.c, dadisks, bos41J, 9522A_all 5/29/95 23:35:58";
/*
 *   COMPONENT_NAME: DADISKS
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		disp_menu
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


/* #define BAD_2P */               /* uncomment when Kazusa 2P drive is added */

#include        <stdio.h>
#include        <nl_types.h>

#include        "dhf.h"
#include        "dhf_msg.h"
#include        "diag/tm_input.h"                         /* faking the ODM  */
#include        "diag/diago.h"
#include        "diag/da.h"
#include	"diag/dascsi.h"
#include        "diag/diag_exit.h"

                                                    /* menu num = 0xXXX001.   */
                                                    /* menu number = 0xXXX004 */
struct msglist start_motor_menu[] =
{
        { DHF_SET1, START_MOTOR,   },
        (int)NULL
};
                                                    /* menu number = 0xXXX005 */
struct msglist bad_format_msg[] =
{
        { DHF_SET1, BAD_FORMAT,    },
        { DHF_SET1, PRESENTER,     },
        (int)NULL
};
                                                    /* menu number = 0xXXX006 */
struct msglist write_protect[] =
{
        { DHF_SET1, WRITE_PROTECT,       },
        { DHF_SET1, PROTECT_INFO,        },
        { DHF_SET1, DM_YES,              },
        { DHF_SET1, DM_NO,               },
        { DHF_SET1, CHOOSE_AN_OPTION,    },
        (int)NULL
};
                                                    /* menu number = 0xXXX007 */
struct msglist do_we_certify[] =
{
        { DHF_SET1, DO_WE_CERTIFY2,       },
        { DHF_SET1, DM_YES,              },
        { DHF_SET1, DM_NO,               },
        { DHF_SET1, CHOOSE_AN_OPTION,    },
        (int)NULL
};
                                                    /* menu number = 0xXXX008 */
struct msglist dec_repair[] =
{
        { DHF_SET1, REPAIR_DEC,    },
        { DHF_SET1, PRESENTER,     },
        (int)NULL
};
                                                    /* menu number = 0xXXX009 */
struct msglist hda_repair[] =
{
        { DHF_SET1, REPAIR_HDA,    },
        { DHF_SET1, PRESENTER,     },
        (int)NULL
};
                                                    /* menu number = 0xXXX010 */
struct msglist backup_drive[] =
{
        { DHF_SET1, BACKUP_DRIVE,    },
        { DHF_SET1, PRESENTER,     },
        (int)NULL
};

struct msglist medium_error[] =			    /* menu number = 0xXXX011 */
{
	{ DHF_SET1, NR_MEDIUM_ERROR,  },
	{ DHF_SET1, PRESENTER,	      },
	(int)NULL
};

struct msglist microcode_cmp_err[] =		    /* menu number = 0xXXX012 */
{
	{ DHF_SET1, MICROCODE_MISCMP,  },
	{ DHF_SET1, PRESENTER,	       },
	(int)NULL
};

struct msglist please_stand_by[] =		    /* menu number = 0xXXX013 */
{
	{ DHF_SET1, PLEASE_STAND_BY, },
	(int)NULL
};

struct msglist certify_terminated[] =		    /* menu number = 0xXXX014 */
{
	{ DHF_SET1, CERTIFY_TERMINATED, },
	{ DHF_SET1, PRESENTER,		},
	(int)NULL
};

struct msglist certify_completed[]=		    /* menu number = 0xXXX015 */
{
	{ DHF_SET1, CERTIFY_COMPLETED, },
	{ DHF_SET1, PRESENTER,	       },
	(int)NULL
};

struct msglist certify_completed_bad[] = 	     /* menu_number = 0xXXX016 */
{
	{ DHF_SET1, CERTIFY_COMPLETED_BAD, },
	{ DHF_SET1, PRESENTER,              },
	(int)NULL
};

struct msglist certify_completed_warning[] = 	     /* menu_number = 0xXXX020 */
{
	{ DHF_SET1, CERTIFY_COMPLETED_WARNING, },
	{ DHF_SET1, PRESENTER,              },
	(int)NULL
};

struct msglist use_format_sa[] =
{
        { DHF_SET1, RUN_FORMAT_SA,    },
        { DHF_SET1, PRESENTER,     },
        (int)NULL
};
                                                    /* menu number = 0xXXX010 */
ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;

ASL_SCR_INFO    menu_motor_start[DIAG_NUM_ENTRIES(start_motor_menu)];
ASL_SCR_INFO    msg_format_bad[DIAG_NUM_ENTRIES(bad_format_msg)];
ASL_SCR_INFO    protect_write[DIAG_NUM_ENTRIES(write_protect)];
ASL_SCR_INFO    certify_we_do[DIAG_NUM_ENTRIES(do_we_certify)];
ASL_SCR_INFO    repair_hda[DIAG_NUM_ENTRIES(hda_repair)];
ASL_SCR_INFO    repair_dec[DIAG_NUM_ENTRIES(dec_repair)];
ASL_SCR_INFO    drive_backup[DIAG_NUM_ENTRIES(backup_drive)];
ASL_SCR_INFO    nr_medium_error[DIAG_NUM_ENTRIES(medium_error)];
ASL_SCR_INFO    miscmp_microcode[DIAG_NUM_ENTRIES(microcode_cmp_err)];
ASL_SCR_INFO    certify_standby[DIAG_NUM_ENTRIES(please_stand_by)];
ASL_SCR_INFO	terminated_certify[DIAG_NUM_ENTRIES(certify_terminated)];
ASL_SCR_INFO	completed_certify[DIAG_NUM_ENTRIES(certify_completed)];
ASL_SCR_INFO	completed_certify_warning[DIAG_NUM_ENTRIES(certify_completed_warning)];
ASL_SCR_INFO    run_format_sa[DIAG_NUM_ENTRIES(use_format_sa)];
 	

struct          tm_input tm_input;
extern		DRIVE_INFO	drive_info;
extern		ODM_INFO	odm_info;
extern          nl_catd catd;
/*  */
/******************************************************************************/
/* NAME: disp_mn()                                                            */
/*                                                                            */
/* FUNCTION: this function is designed to display an informational menu, a    */
/*           user input menu or a message to the user.                        */
/*                                                                            */
/* EXECUTION ENVIRONMENT:                                                     */
/*                                                                            */
/* NOTES:                                                                     */
/*                                                                            */
/* RECOVERY OPERATION:                                                        */
/*                                                                            */
/* DATA STRUCTURES:                                                           */
/*                                                                            */
/* RETURNS: Return code from the ASL routine called  -  ASL_rc                */
/******************************************************************************/

int disp_menu(msg_num, failing_function_code)
int     msg_num;                               /* pointer to the msg number   */
int     failing_function_code;
{
        int     ASL_rc;                        /* return code from a function */
        long    menu_nmbr;                     /* displayed on each menu      */
        int     cert_time;                     /* amount of time to do certify.
                                                * Calculated, based on size
                                                * of the drive.
                                                */
        char    msgstr[512];
	char 	*msgptr;

        ASL_rc = -99;
        menu_nmbr = 0;
        cert_time = 0;
        /*
	  Put Function/Fru name in string var
	*/
	menu_nmbr = failing_function_code * 0x1000;
        switch( msg_num ) {
        case START_MOTOR        :
		menu_nmbr = menu_nmbr + 4;
                ASL_rc = diag_display( menu_nmbr, catd, start_motor_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_motor_start );
                sprintf( msgstr, menu_motor_start[0].text, tm_input.dname,
			   tm_input.dnameloc );
                free( menu_motor_start[0].text );
                menu_motor_start[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_motor_start[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_motor_start );
                break;
        case BAD_FORMAT        :
 		menu_nmbr = menu_nmbr + 5;
                ASL_rc = diag_display( menu_nmbr, catd, bad_format_msg,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, msg_format_bad );
                sprintf( msgstr, msg_format_bad[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( msg_format_bad[0].text );
                msg_format_bad[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( msg_format_bad[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, msg_format_bad );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case WRITE_PROTECT     :
		menu_nmbr = menu_nmbr + 6;
                ASL_rc = diag_display( menu_nmbr, catd, write_protect,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, protect_write );
                sprintf( msgstr, protect_write[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( protect_write[0].text );
                protect_write[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( protect_write[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, protect_write );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case DO_WE_CERTIFY2     :
		menu_nmbr = menu_nmbr + 7;
                ASL_rc = diag_display( menu_nmbr, catd, do_we_certify,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, certify_we_do );
                                                   /* put dname in title line */
                sprintf( msgstr, certify_we_do[0].text, tm_input.dname,
			  tm_input.dnameloc, odm_info.certify_time);
                free( certify_we_do[0].text );
                certify_we_do[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( certify_we_do[0].text, msgstr );
                                    /* put time to do certify in message line */
                                       /* display menu with the substitutions */
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, certify_we_do );
             	menu_return = DIAG_ITEM_SELECTED( menutype );
		free(certify_we_do[0].text);
                break;
        case REPAIR_DEC     :
		menu_nmbr = menu_nmbr + 8;
                ASL_rc = diag_display( menu_nmbr, catd, dec_repair,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, repair_dec );
                sprintf( msgstr, repair_dec[0].text, tm_input.dname,
			   tm_input.dnameloc );
                free( repair_dec[0].text );
                repair_dec[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( repair_dec[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, repair_dec);
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case REPAIR_HDA     :
		menu_nmbr = menu_nmbr + 9;
                ASL_rc = diag_display( menu_nmbr, catd, hda_repair,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, repair_hda );
                sprintf( msgstr, repair_hda[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( repair_hda[0].text );
                repair_hda[0].text = (char * ) malloc ( strlen( msgstr ) +1 );
                strcpy ( repair_hda[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, repair_hda);
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case BACKUP_DRIVE   :
		menu_nmbr = menu_nmbr + 0x10;
                ASL_rc = diag_display( menu_nmbr, catd, backup_drive,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, drive_backup );
                sprintf( msgstr, drive_backup[0].text, tm_input.dname,
                                 tm_input.dnameloc, tm_input.dname );
                free( drive_backup[0].text );
                drive_backup[0].text = (char * ) malloc ( strlen( msgstr ) +1 );
                strcpy ( drive_backup[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, drive_backup);
                break;
 
	case NR_MEDIUM_ERROR:
		menu_nmbr = menu_nmbr + 0x11;
                ASL_rc = diag_display( menu_nmbr, catd, medium_error,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, nr_medium_error);
                sprintf( msgstr, nr_medium_error[0].text, tm_input.dname,
                                                            tm_input.dnameloc );
                free( nr_medium_error[0].text );
                nr_medium_error[0].text = (char * )malloc(strlen(msgstr) +1);
                strcpy ( nr_medium_error[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, nr_medium_error);
                break;
 
	case MICROCODE_MISCMP:
		menu_nmbr = menu_nmbr + 0x12;
                ASL_rc = diag_display( menu_nmbr, catd, microcode_cmp_err,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, miscmp_microcode);
                sprintf( msgstr, miscmp_microcode[0].text, tm_input.dname,
                                                            tm_input.dnameloc );
                free( miscmp_microcode[0].text );
                miscmp_microcode[0].text = (char *)malloc(strlen(msgstr) +1 );
                strcpy ( miscmp_microcode[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, miscmp_microcode);
                break;
 
	case PLEASE_STAND_BY:
		menu_nmbr = menu_nmbr + 0x13;
                ASL_rc = diag_display( menu_nmbr, catd, please_stand_by,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, certify_standby );
                sprintf( msgstr, certify_standby[0].text, tm_input.dname,
			   tm_input.dnameloc, drive_info.percent_complete );
		free(certify_standby[0].text);
		certify_standby[0].text = (char *) malloc (strlen(msgstr)+1);
		strcpy(certify_standby[0].text, msgstr);
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, certify_standby );
                break;
	case CERTIFY_TERMINATED:
		menu_nmbr = menu_nmbr + 0x14;
                ASL_rc = diag_display( menu_nmbr, catd, certify_terminated,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, terminated_certify);
                sprintf( msgstr, terminated_certify[0].text, tm_input.dname,
                                                            tm_input.dnameloc );
                free( terminated_certify[0].text );
                terminated_certify[0].text = (char * )malloc(strlen(msgstr) +1);
                strcpy ( terminated_certify[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, terminated_certify);
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
	case CERTIFY_COMPLETED:
		menu_nmbr = menu_nmbr + 0x15;
                ASL_rc = diag_display( menu_nmbr, catd, certify_completed,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, completed_certify);
                sprintf( msgstr, completed_certify[0].text, tm_input.dname,
                     tm_input.dnameloc, drive_info.drive_capacity,
		     drive_info.rec_data_errors, drive_info.unrec_data_errors,
	             drive_info.rec_equ_check_errors, drive_info.unrec_equ_check_errors);
                free( completed_certify[0].text );
                completed_certify[0].text = (char * )malloc(strlen(msgstr) +1);
                strcpy ( completed_certify[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, completed_certify);
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
	case CERTIFY_COMPLETED_BAD:
		menu_nmbr = menu_nmbr + 0x16;
                ASL_rc = diag_display( menu_nmbr, catd, certify_completed_warning,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype,completed_certify_warning);
                sprintf( msgstr, completed_certify_warning[0].text, tm_input.dname,
                     tm_input.dnameloc);
                free( completed_certify_warning[0].text );
                completed_certify_warning[0].text = (char * )malloc(strlen(msgstr) +1);
                strcpy ( completed_certify_warning[0].text, msgstr );
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, completed_certify_warning);
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;

	case 	MISSING_ATTRIBUTES:
		menu_nmbr = menu_nmbr + 0x17;
		sprintf( msgstr, (char *) diag_cat_gets(catd,
			 DHF_SET1, MISSING_ATTRIBUTES), menu_nmbr,
			 tm_input.dname, tm_input.dnameloc);
		menugoal(msgstr); 
		ASL_rc = DIAG_ASL_OK;
		break;
	case 	BAD_FORMAT_MENUGOAL:
		menu_nmbr += 0x18;
		sprintf(msgstr, "%6X ", menu_nmbr);
		msgptr = (char *)calloc(1, 512);
		sprintf(msgptr, (char *)diag_cat_gets(catd, DHF_SET1,
				  	              BAD_FORMAT), 
			tm_input.dname, tm_input.dnameloc);
		strcat(msgstr, msgptr);
		menugoal(msgstr); 
		ASL_rc = DIAG_ASL_OK;
		break;
        case RUN_FORMAT_SA:
		menu_nmbr += 0x19;
		sprintf(msgstr, "%6X ", menu_nmbr);
		msgptr = (char *)calloc(1, 512);
		sprintf(msgptr, (char *)diag_cat_gets(catd, DHF_SET1,
				  	              RUN_FORMAT_SA), 
			tm_input.dname, tm_input.dnameloc);
		strcat(msgstr, msgptr);
		menugoal(msgstr); 
		ASL_rc = DIAG_ASL_OK;
		break;
        case CERTIFY_COMPLETED_WARNING:
		menu_nmbr += 0x20;
		sprintf(msgstr, "%6X ", menu_nmbr);
		msgptr = (char *)calloc(1, 512);
		sprintf(msgptr, (char *)diag_cat_gets(catd, DHF_SET1,
				  	              CERTIFY_COMPLETED_WARNING), 
			tm_input.dname, tm_input.dnameloc);
		strcat(msgstr, msgptr);
		menugoal(msgstr); 
		ASL_rc = DIAG_ASL_OK;
		break;
        default:
                break;
        }  /* endswitch */
        return(ASL_rc);
} /* endfunction disp_menu */
