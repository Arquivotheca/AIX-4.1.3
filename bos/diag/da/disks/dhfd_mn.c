static char sccsid[] = "@(#)49	1.3  src/bos/diag/da/disks/dhfd_mn.c, dadisks, bos411, 9428A410j 6/21/91 17:10:15";

/*
 * COMPONENT_NAME: DAMEDIA
 *
 * FUNCTIONS: Performs the screen display functions for the direct bus
 *            attached fixed-disk diagnostic application.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        <stdio.h>
#include        <nl_types.h>

#include        "dhfd.h"
#include        "dhfd_msg.h"
#include        <diag/tm_input.h>                         /* faking the ODM  */
#include        <diag/diago.h>
#include        <diag/da.h>
#include        <diag/diag_exit.h>

#ifndef         CATD_ERR
#define         CATD_ERR -1
#endif


struct tm_input tm_input;         /* test parameters - from the ODM database */
                                    /* set up what is in the screen messages */
struct msglist tst_menu[] =
{
        { DHFD_SET1, WE_R_TESTING, },
        (int)NULL
};

struct msglist adv_tst_menu[] =
{
        { DHFD_SET1, WE_R_ADVANCED, },
        (int)NULL
};

struct msglist lm_stat_menu[] =
{
        { DHFD_SET1, LOOPMODE_STATUS, },
        (int)NULL
};

struct msglist certify_prompt_menu[] =
{
	{ DHFD_SET1, CERTIFY_PROMPT, },
	{ DHFD_SET1, DM_YES, },
	{ DHFD_SET1, DM_NO, },
	{ DHFD_SET1, CHOOSE_AN_OPTION, },
	(int)NULL
};

struct msglist certifying_disk_menu[] =
{
	{ DHFD_SET1, CERTIFYING_DISK, },
	(int)NULL
};

struct msglist certify_abort_menu[] =
{
	{ DHFD_SET1, CERTIFY_ABORT, },
	(int)NULL         
};

struct msglist certify_completed_menu[] =
{
	{ DHFD_SET1, CERTIFY_COMPLETED, },
	(int)NULL
};

ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;
ASL_SCR_INFO    menu_tst[DIAG_NUM_ENTRIES(tst_menu)];
ASL_SCR_INFO    menu_tst_adv[DIAG_NUM_ENTRIES(adv_tst_menu)];
ASL_SCR_INFO    menu_stat_lm[DIAG_NUM_ENTRIES(lm_stat_menu)];
ASL_SCR_INFO    menu_certify_prompt[DIAG_NUM_ENTRIES(certify_prompt_menu)];
ASL_SCR_INFO	menu_certifying_disk[DIAG_NUM_ENTRIES(certifying_disk_menu)];
ASL_SCR_INFO	menu_abort_certify[DIAG_NUM_ENTRIES(certify_abort_menu)];
ASL_SCR_INFO	menu_completed_certify[DIAG_NUM_ENTRIES(certify_completed_menu)];



/****************************************************************************/
/* NAME: disp_menu()                                                        */
/*                                                                          */
/* FUNCTION: this function is designed to display an informational menu, a  */
/*           user input menu or a message to the user.                      */
/*                                                                          */
/* EXECUTION ENVIRONMENT:                                                   */
/*                                                                          */
/* NOTES:                                                                   */
/*                                                                          */
/* RECOVERY OPERATION:                                                      */
/*                                                                          */
/* DATA STRUCTURES:                                                         */
/*                                                                          */
/* RETURNS: Return code from the ASL routine called  -  ASL_rc              */
/****************************************************************************/

disp_menu(msg_num)
int     msg_num;                           /* pointer to the msg number   */

{
        int     ASL_rc;                      /* return code from a function */
	int 	menu_number;
        char    msgstr[512];

        ASL_rc = -99;
	menu_number = led_value * 0x01000;
        switch( msg_num ) {
        case WE_R_TESTING      :
		menu_number = menu_number + 0x01;
                ASL_rc = diag_display( menu_number, catd, tst_menu, DIAG_MSGONLY,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_tst );
                sprintf( msgstr, menu_tst[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_tst[0].text );
                menu_tst[0].text = ( char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_tst[0].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_tst);
                break;
        case WE_R_ADVANCED     :
		menu_number = menu_number + 0x02;
                ASL_rc = diag_display( menu_number, catd, adv_tst_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_tst_adv );
                sprintf( msgstr, menu_tst_adv[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_tst_adv[0].text );
                menu_tst_adv[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_tst_adv[0].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_tst_adv);
                break;
        case LOOPMODE_STATUS   :
		menu_number = menu_number + 0x04;
                ASL_rc = diag_display( menu_number, catd, lm_stat_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_stat_lm );
                sprintf( msgstr, menu_stat_lm[0].text, tm_input.dname,
                           tm_input.dnameloc, tm_input.lcount,
			   tm_input.lerrors );
                free( menu_stat_lm[0].text );
                menu_stat_lm[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_stat_lm[0].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_OUTPUT_LEAVE_SC,
                                       &menutype, menu_stat_lm);
                break;
	case CERTIFY_PROMPT	:
		menu_number = menu_number + 0x05;
		ASL_rc = diag_display( menu_number, catd, certify_prompt_menu,
					DIAG_MSGONLY,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutype, menu_certify_prompt);
                sprintf( msgstr, menu_certify_prompt[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_certify_prompt[0].text );
                menu_certify_prompt[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_certify_prompt[0].text, msgstr );
		ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
					ASL_DIAG_LIST_CANCEL_EXIT_SC,
					&menutype, menu_certify_prompt);
		menu_return = DIAG_ITEM_SELECTED( menutype );
		break;	
	case CERTIFYING_DISK	:
		menu_number = menu_number + 0x06;
		ASL_rc = diag_display ( menu_number, catd, certifying_disk_menu,
					DIAG_MSGONLY,
					ASL_DIAG_OUTPUT_LEAVE_SC, 
					&menutype, menu_certifying_disk);	
                sprintf( msgstr, menu_certifying_disk[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_certifying_disk[0].text );
                menu_certifying_disk[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_certifying_disk[0].text, msgstr );
		ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
					ASL_DIAG_OUTPUT_LEAVE_SC,
					&menutype, menu_certifying_disk);
		break;
	case CERTIFY_ABORT	:
		menu_number = menu_number + 0x07;
		ASL_rc = diag_display ( menu_number, catd, certify_abort_menu,
					DIAG_MSGONLY,
					ASL_DIAG_KEYS_ENTER_SC,
					&menutype, menu_abort_certify);
                sprintf( msgstr, menu_abort_certify[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_abort_certify[0].text );
                menu_abort_certify[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_abort_certify[0].text, msgstr );
		ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC,
					&menutype, menu_abort_certify);
		break;

	case CERTIFY_COMPLETED	:
		menu_number = menu_number + 0x08;
		ASL_rc = diag_display ( menu_number, catd, certify_completed_menu,
					DIAG_MSGONLY,
					ASL_DIAG_KEYS_ENTER_SC,
					&menutype, menu_completed_certify);
                sprintf( msgstr, menu_completed_certify[0].text, tm_input.dname,
			  tm_input.dnameloc );
                free( menu_completed_certify[0].text );
                menu_completed_certify[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_completed_certify[0].text, msgstr );
		ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
					ASL_DIAG_KEYS_ENTER_SC,
					&menutype, menu_completed_certify);
		break;


        default              :
                break;
        }  /* endswitch */
        return(ASL_rc);
} /* endfunction disp_menu */
