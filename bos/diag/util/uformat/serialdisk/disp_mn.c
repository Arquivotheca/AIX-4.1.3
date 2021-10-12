static char sccsid[] = "@(#)18  1.7.2.11  src/bos/diag/util/uformat/serialdisk/disp_mn.c, dsauformat, bos41J, 9522A_all 5/29/95 22:45:54";
/*
 *   COMPONENT_NAME: DSAUFORMAT
 *
 *   FUNCTIONS: clear_pvid
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


#include <stdio.h>
#include <fcntl.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <nl_types.h>
#include <limits.h>

#include "diag/da.h"            /* FRU Bucket Database */
#include "diag/da_rc.h"
#include "diag/dascsi.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"     /* return codes for DC */
#include "diag/tmdefs.h"        /* diagnostic modes and variables     */
#include "sys/cfgodm.h"
#include "uformat.h"
#include "disp_mn.h"

char	*diag_cat_gets();
extern nl_catd diag_catopen(char *, int);
extern struct da_menu	dmnu;

/*
 * NAME: disp_menu
 *
 * FUNCTION: display a selected menu from the cat file
 *
 * NOTES: display a selected menu from the cat file.
 *
 *
 * DATA STRUCTURES:
 *
 * RETURNS: returns return code from diag_display
 */


int
disp_menu(selected_message)
int                     selected_message;
{
	extern int              progmode;        /* UFORMAT or UCERTIFY */
	extern DRIVE_INFO       drive_info;
	extern struct listinfo  obj_info;        /* struct for ODM retrieve */
	extern struct CuDv      *cudv;           /* struct for ODM retrieve */
	extern struct CuDv      *cudv_selected;  /* struct for ODM retrieve */
	extern int              certify_after;   /* certify after format    */
	/* ( 0 or 1 )              */
	extern int              is_working;      /* set when formatting     */
	/* or certifying drive     */
	int                     response = 0;
	int                     rc;
	int                     temp_i;
	nl_catd                 catd;
	char                    temp_buff[1024];
	ASL_SCR_TYPE            menutype = DM_TYPE_DEFAULTS;
	ASL_SCR_INFO            *menuinfo;
	char                    *sub_action;
	char                    *sub_action2;

	catd = diag_catopen(dmnu.catfile, 0);

	sub_action = (char *)malloc(32);
	sub_action2 = (char *)malloc(32);
	if (progmode == UFORMAT) {
		sub_action = diag_cat_gets(catd,DUFORMAT_SET,SUB_FORMAT);
		sub_action2 = diag_cat_gets(catd,DUFORMAT_SET,SUB_FORMATTED);
	}
	else if (progmode == UDCLASS) {
		sub_action = diag_cat_gets(catd,DUFORMAT_SET,SUB_D_CLASSIFY);
		sub_action2 = diag_cat_gets(catd,DUFORMAT_SET,SUB_D_CLASSIFIED);
	}
	else {
		sub_action = diag_cat_gets(catd,DUFORMAT_SET,SUB_CERTIFY);
		sub_action2 = diag_cat_gets(catd,DUFORMAT_SET,SUB_CERTIFIED);
	}
	menuinfo = (ASL_SCR_INFO *)calloc(8,sizeof(ASL_SCR_INFO));

	switch (selected_message) {
	case MENU_SELECT_DRIVE:
		dmnu.menunum = ( MENU_BASE + 0x03 );
		free(menuinfo);
		menuinfo = (ASL_SCR_INFO *)calloc(obj_info.num + 2,
		    sizeof(ASL_SCR_INFO) );


		/* put in first line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,MENU_SELECT_DRIVE),
		    sub_action2);
		menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[0].text,temp_buff);

		/* put in list of drives */
		for( temp_i = 1; temp_i <= obj_info.num; ++temp_i ) {
			sprintf(temp_buff,
			    diag_cat_gets(catd,DUFORMAT_SET,
			    OPTION_DRIVE_NAME),
			    (cudv +(temp_i-1))->location);
			menuinfo[temp_i].text =
			    (char *)malloc(strlen(temp_buff)+1);
			strcpy(menuinfo[temp_i].text,temp_buff);
		}

		/* put in last line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,OPTION_CHOOSE) );
		menuinfo[temp_i].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[temp_i].text,temp_buff);

		menutype.max_index = temp_i;
		menutype.cur_index = 1;
		rc = diag_display(dmnu.menunum, catd, NULL, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);

		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);

		break;
	case MENU_FORMAT_OR_CERTIFY:
		dmnu.menunum = ( MENU_BASE + 0x00 );

		menutype.max_index = 3;
		menutype.cur_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_format_or_certify, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);

		break;
	case MENU_F_CERTIFY_AFTER:
		dmnu.menunum = ( MENU_BASE + 0x02 );

		menutype.max_index = 4;
		menutype.cur_index = 1;

		rc = diag_display(dmnu.menunum, catd, 
		    menu_f_certify_after, DIAG_IO, 
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);

		break;
	case MENU_F_A_CERTIFY_AFTER:
		dmnu.menunum = ( MENU_BASE + 0x04 );
		menutype.max_index = 4;
		menutype.cur_index = 1;
		rc = diag_display(dmnu.menunum, catd, 
		    menu_f_a_certify_after, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);
		break;

	case MENU_F_WARNING:
		dmnu.menunum = ( MENU_BASE + 0x05 );
		menutype.max_index = 3;
		rc = diag_display(dmnu.menunum, catd, 
		    menu_f_warning, DIAG_MSGONLY,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		menutype.cur_index = 3;
		rc = diag_display(dmnu.menunum, catd, 
		    NULL, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);
		break;
	case MENU_PLEASE_STAND_BY:
		dmnu.menunum = ( MENU_BASE + 0x06 );
		menuinfo = (ASL_SCR_INFO *)calloc(1,sizeof(ASL_SCR_INFO));
		/* title line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,MENU_PLEASE_STAND_BY),
		    sub_action, drive_info.percent_complete );
		menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[0].text,temp_buff);

		menutype.max_index = 0;
		rc = diag_display(dmnu.menunum, catd, NULL, DIAG_IO,
		    ASL_DIAG_OUTPUT_LEAVE_SC,
		    &menutype, menuinfo);
		break;

	case MENU_F_COMPLETE:
		dmnu.menunum = ( MENU_BASE + 0x07 );
		if( certify_after ) {
			menutype.max_index = 0;
			rc = diag_display(dmnu.menunum, catd, 
			    menu_f_complete_nowait, DIAG_IO,
			    ASL_DIAG_OUTPUT_LEAVE_SC,
			    &menutype, menuinfo);
		} else {
			menutype.max_index = 1;
			rc = diag_display(dmnu.menunum, catd, 
			    menu_f_complete, DIAG_IO,
			    ASL_DIAG_KEYS_ENTER_SC,
			    &menutype, menuinfo);
		}
		break;
	case MENU_C_COMPLETE:
		dmnu.menunum = ( MENU_BASE + 0x08 );
		menuinfo = (ASL_SCR_INFO *)calloc(2,sizeof(ASL_SCR_INFO));
		/* title line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,MENU_C_COMPLETE),
		    drive_info.drive_capacity,
		    drive_info.rec_data_errors,
		    drive_info.unrec_data_errors,
		    drive_info.rec_equ_check_errors,
		    drive_info.unrec_equ_check_errors );
		menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[0].text,temp_buff);
		/* last line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,OPTION_ENTER));
		menuinfo[1].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[1].text,temp_buff);

		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, NULL, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_C_COMPLETE_BAD:
		dmnu.menunum = ( MENU_BASE + 0x09 );

		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_c_complete_bad, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_TERMINATED:
		dmnu.menunum = ( MENU_BASE + 0x10 );
		rc = diag_display(dmnu.menunum, catd, menu_terminated, 
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);

		/* title line */
		/* input subaction */
		sprintf(temp_buff,menuinfo[0].text, sub_action2);
		menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[0].text,temp_buff);

		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, NULL, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_SHOULD_FORMAT:
		dmnu.menunum = ( MENU_BASE + 0x11 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_should_format, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_ESC_F_WARNING_2:
		dmnu.menunum = ( MENU_BASE + 0x12 );
		menutype.max_index = 3;
		menutype.cur_index = 2;
		rc = diag_display(dmnu.menunum, catd, menu_esc_f_warning_2, 
		    DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT) {
			response = DIAG_ITEM_SELECTED(menutype);
		} else if (rc == ASL_CANCEL ) {
			/* this is to stop an infinite loop */
			rc = ASL_COMMIT;
			response = 1;
		}
		break;
	case MENU_ESC_C_WARNING_2:
		dmnu.menunum = ( MENU_BASE + 0x12 );
		menutype.max_index = 3;
		menutype.cur_index = 2;
		rc = diag_display(dmnu.menunum, catd, menu_esc_c_warning_2, 
		    DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);

		if (rc == ASL_COMMIT) {
			response = DIAG_ITEM_SELECTED(menutype);
		} else if (rc == ASL_CANCEL ) {
			/* this is to stop an infinite loop */
			rc = ASL_COMMIT;
			response = 1;
		}
		break;
	case MENU_F_USE_FLOPPY_2:
		dmnu.menunum = ( MENU_BASE + 0x13 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_f_use_floppy_2, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_C_USE_FLOPPY_2:
		dmnu.menunum = ( MENU_BASE + 0x13 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_c_use_floppy_2, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;

	case NO_HARD_DISKS:
		dmnu.menunum = ( MENU_BASE + 0x14 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, no_hard_disks, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case FORMAT_IN_PROGRESS:
		dmnu.menunum = ( MENU_BASE + 0x15 );
		menutype.max_index = 0;
		rc = diag_display(dmnu.menunum, catd, format_in_progress, 
		    DIAG_IO, ASL_DIAG_OUTPUT_LEAVE_SC,
		    &menutype, menuinfo);
		break;
	case MENU_NO_ATTR_2:
		dmnu.menunum = ( MENU_BASE + 0x16 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_no_attr_2, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case OS_WARN_CONTINUE:
		dmnu.menunum = ( MENU_BASE + 0x17 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, os_warn_continue, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case OS_WARN_RETURN:
		dmnu.menunum = ( MENU_BASE + 0x18 );
		rc = diag_display(dmnu.menunum, catd, os_warn_continue, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case OS_AND_DATA_WARN:
		dmnu.menunum = ( MENU_BASE + 0x19 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, os_and_data_warn, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case ADAPTER_CONFIGURE:
		dmnu.menunum = ( MENU_BASE + 0x20 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, adapter_configure, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case DEVICE_CONFIGURE:
		dmnu.menunum = ( MENU_BASE + 0x21 );
		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, device_configure, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;

		/* Warning that the operation hasn't completed */
	case MENU_ESC_UD_WARNING:
		dmnu.menunum = ( MENU_BASE + 0x12 );

		/* set up menutype so that asl knows how many options */
		menutype.max_index = 3;
		menutype.cur_index = 2;

		/* display the menu */
		rc = diag_display(dmnu.menunum, catd, menu_esc_ud_warning, 
		    DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT) {
			response = DIAG_ITEM_SELECTED(menutype);
		} else if (rc == ASL_CANCEL) {
			/* this is to stop an infinite loop */
			rc = ASL_COMMIT;
			response =1 ;
		}
		break;
		/* tells the user to use run the erasure from a diskette */
	case MENU_E_USE_FLOPPY:
		dmnu.menunum = ( MENU_BASE + 0x13 );
		rc = diag_display(dmnu.menunum, catd, menu_e_use_floppy, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, &menutype, menuinfo);
		break;

		/* used to warn user that all data will be destroyed */
	case D_CLASS_DISK:
		dmnu.menunum = ( MENU_BASE + 0x22 );

		menutype.max_index = 3;

		rc = diag_display(dmnu.menunum, catd, d_class_disk, DIAG_MSGONLY,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		menutype.cur_index = 3;
		rc = diag_display(dmnu.menunum, catd, NULL, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);


		if(rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);
		break;

		/* used to display amount of operation completed */
	case CDEP_PERCENT_DONE:
		dmnu.menunum = ( MENU_BASE + 0x23 );
		menuinfo = (ASL_SCR_INFO *)calloc(2,sizeof(ASL_SCR_INFO));
		/* title line */
		sprintf(temp_buff,
		    diag_cat_gets(catd,DUFORMAT_SET,CDEP_STAND_BY),
		    operation);
		menuinfo[0].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[0].text,temp_buff);

		/* show percent done if > 0 % */
		/* percent done */
		if(drive_info.percent_complete >= 1)
			sprintf(temp_buff,
			    diag_cat_gets(catd,DUFORMAT_SET,CDEP_PERCENT_DONE),
			    drive_info.percent_complete);
		else /* don't show percent just show stand by msg. */
			sprintf(temp_buff,"\0");

		menuinfo[1].text = (char *)malloc(strlen(temp_buff)+1);
		strcpy(menuinfo[1].text,temp_buff);

		menutype.max_index = 1;

		rc = diag_display(dmnu.menunum, catd, NULL, DIAG_IO,
		    ASL_DIAG_OUTPUT_LEAVE_SC,
		    &menutype, menuinfo);

		break;
		/* Issued when erasure is completed */
	case MENU_DCLASS_COMPLETE:
		dmnu.menunum = ( MENU_BASE + 0x24 );
		rc = diag_display(dmnu.menunum, catd, menu_dclass_complete, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
		/* This is displayed when no bad blocks are found */
	case NO_BAD_BLOCKS:
		dmnu.menunum = ( MENU_BASE + 0x25 );

		rc = diag_display(dmnu.menunum, catd, no_bad_blocks, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case CMD_NOT_SUP:
		dmnu.menunum = ( MENU_BASE + 0x28 );

		menutype.max_index = 3;
		menutype.cur_index = 2;
		rc = diag_display(dmnu.menunum, catd, cmd_not_sup, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT) {
			response = DIAG_ITEM_SELECTED(menutype);
		} else if (rc == ASL_CANCEL ) {
			/* this is to stop an infinite loop */
			rc = ASL_COMMIT;
			response = 1;
		}
		break;
	case UNKNOWN_ERR:
		dmnu.menunum = ( MENU_BASE + 0x29 );

		rc = diag_display(dmnu.menunum, catd, unknown_err, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_WRITE_FAILED:
		dmnu.menunum = ( MENU_BASE + 0x30 );

		rc = diag_display(dmnu.menunum, catd, menu_write_failed,DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
		/* erasure completed and suggest formatting the drive */
	case MENU_SUGGEST_FORMAT:
		dmnu.menunum = ( MENU_BASE + 0x31 );

		rc = diag_display(dmnu.menunum, catd, menu_suggest_format, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case NO_SUCH_BLOCK:
		dmnu.menunum = ( MENU_BASE + 0x32 );

		rc = diag_display(dmnu.menunum, catd, no_such_block, 
		    DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_F_SR_CERTIFY_AFTER:
		dmnu.menunum = ( MENU_BASE + 0x033 );

		menutype.max_index = 3;
		menutype.cur_index = 1;

		rc = diag_display(dmnu.menunum, catd, 
		    menu_f_sr_certify_after, DIAG_IO, 
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);
		break;
	case DATA_ERR:
		dmnu.menunum = ( MENU_BASE + 0x34 );

		rc = diag_display(dmnu.menunum, catd, data_err, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case EQUIP_ERR:
		dmnu.menunum = ( MENU_BASE + 0x35 );

		rc = diag_display(dmnu.menunum, catd, equip_err, DIAG_IO,
		    ASL_DIAG_KEYS_ENTER_SC,
		    &menutype, menuinfo);
		break;
	case MENU_READ_OR_WRITE:
		dmnu.menunum = ( MENU_BASE + 0x36 );

		menutype.max_index = 3;
		menutype.cur_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_read_or_write, DIAG_IO,
		    ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, menuinfo);
		if (rc == ASL_COMMIT)
			response = DIAG_ITEM_SELECTED(menutype);
		break;
	case MENU_C_COMPLETE_WARNING:
		dmnu.menunum = ( MENU_BASE + 0x37 );

		menutype.max_index = 1;
		rc = diag_display(dmnu.menunum, catd, menu_c_complete_warning,
			DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
			&menutype, menuinfo);
		break;
	default:
		break;
	}

	free(menuinfo);

	/* check if the user hit to exit and in case, do it. */
	switch (rc) {
	case ASL_EXIT:
	case ASL_CANCEL:
		if( is_working ) {
			if( progmode == UFORMAT )
				response = disp_menu( MENU_ESC_F_WARNING_2 );
			else if( progmode == UDCLASS )
				response = disp_menu( MENU_ESC_UD_WARNING );
			else
				response = disp_menu( MENU_ESC_C_WARNING_2 );

			if( response == 1 ) {
				clean_up();
			} else {
				response = disp_menu( selected_message );
			}
		} else {
			clean_up();
		}
		break;
	default:
		break;
	}

	catclose(catd);

	return (response);
}
/*  */
/*
 * NAME:  clear_pvid
 *
 * FUNCTION: Perform clean up of data base to clear pvid
 *
 * NOTES: 	This routine must be able to page fault
 *
 * RETURNS: None
 *
*/
void
clear_pvid(device_name)
char	*device_name;
{
	struct	CuAt *pvidattr;
	int	howmany;


	if((pvidattr=(struct CuAt *)getattr(device_name,"pvid", FALSE, &howmany)
	    )==NULL)
		return;
	strcpy(pvidattr->value,"none");
	(void)putattr(pvidattr);
	return;

}
