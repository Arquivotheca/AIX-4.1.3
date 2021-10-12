static char sccsid[] = "@(#)93	1.7  src/bos/diag/da/scsi/dscsia_mn.c, dascsi, bos411, 9428A410j 12/10/92 09:24:41";
/*
 *   COMPONENT_NAME: DASCSI
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		disp_menu
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1992
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include        <stdio.h>
#include        <nl_types.h>

#include        "dscsia.h"
#include        "dscsia_msg.h"
#include        <diag/tm_input.h>                         /* faking the ODM  */
#include        <diag/diago.h>
#include        <diag/da.h>
#include        <diag/diag_exit.h>

#ifndef         CATD_ERR
#define         CATD_ERR -1
#endif


struct msglist unplug_plug_menu[] =
{
        { DSCSIA_SET1, UNPLUG_PLUG, },
        { DSCSIA_SET1, UPC, },
        { DSCSIA_SET1, PIEST, },
        { DSCSIA_SET1, PRESS_ENTER, },
        NULL
};

struct msglist replug_menu[] =
{
        { DSCSIA_SET1, REPLUG_DEV, },
        { DSCSIA_SET1, UNPLUG_TERM, },
        { DSCSIA_SET1, REPLUG, },
        { DSCSIA_SET1, PRESS_ENTER, },
        NULL
};
struct msglist ext_terminator[]=
{
	{ DSCSIA_SET1, EXT_TERMINATOR, },
	{ DSCSIA_SET1, YES_OPTION, },
	{ DSCSIA_SET1, NO_OPTION, },
	{ DSCSIA_SET1, CHOOSE_AN_OPTION, },
	NULL
};   
struct msglist unplug_plug_int_menu[] =
{
        { DSCSIA_SET1, UNPLUG_PLUG_INT, },
        { DSCSIA_SET1, UPCI, },
        { DSCSIA_SET1, PRESS_ENTER, },
        NULL
};
struct msglist replug_int_menu[] =
{
        { DSCSIA_SET1, REPLUG_INT_DEV, },
        { DSCSIA_SET1, REPLUG_INT, },
        { DSCSIA_SET1, PRESS_ENTER, },
        NULL
};
ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;

ASL_SCR_INFO    menu_plug_unplug[DIAG_NUM_ENTRIES(unplug_plug_menu)];
ASL_SCR_INFO    menu_replug[DIAG_NUM_ENTRIES(replug_menu)];
ASL_SCR_INFO    menu_term[DIAG_NUM_ENTRIES(ext_terminator)];
ASL_SCR_INFO    menu_plug_unplug_int[DIAG_NUM_ENTRIES(unplug_plug_int_menu)];
ASL_SCR_INFO    menu_replug_int_dev[DIAG_NUM_ENTRIES(replug_int_menu)];
struct          tm_input tm_input;
extern          nl_catd catd;           /* pointer to the catalog file */

/****************************************************************************/
/* NAME: disp_mn()                                                          */
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
        char    msgstr[512];
	long	menu_number;

        ASL_rc = -99;
	menu_number = (failing_function_code * 0x1000) + msg_num;
        switch( msg_num ) {
        case UNPLUG_PLUG       :
                ASL_rc = diag_display( menu_number, catd, unplug_plug_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_plug_unplug );
                NLsprintf( msgstr, menu_plug_unplug[0].text, tm_input.dname,
			 tm_input.dnameloc );
                free( menu_plug_unplug[0].text );
                menu_plug_unplug[0].text = (char *)malloc( strlen( msgstr )+1 );
                strcpy ( menu_plug_unplug[0].text, msgstr );
                NLsprintf( msgstr, menu_plug_unplug[1].text, tm_input.dnameloc);
		free( menu_plug_unplug[1].text );
                menu_plug_unplug[1].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_plug_unplug[1].text, msgstr );
		NLsprintf( msgstr, menu_plug_unplug[2].text,tm_input.dnameloc);
                free( menu_plug_unplug[2].text );
		menu_plug_unplug[2].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_plug_unplug[2].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_plug_unplug );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case REPLUG_DEV       :
                ASL_rc = diag_display( menu_number, catd, replug_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_replug );
                NLsprintf( msgstr, menu_replug[0].text, tm_input.dname,
			 tm_input.dnameloc );
		free ( menu_replug [0].text );
                menu_replug[0].text = (char *)malloc( strlen( msgstr )+1 );
                strcpy ( menu_replug[0].text, msgstr );
                NLsprintf( msgstr, menu_replug[1].text,tm_input.dnameloc );
		free ( menu_replug [1].text );
                menu_replug[1].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_replug[1].text, msgstr );
                NLsprintf( msgstr, menu_replug[2].text, tm_input.dnameloc );
		free ( menu_replug [2].text );
                menu_replug[2].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_replug[2].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_replug );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
	case EXT_TERMINATOR	:
                ASL_rc = diag_display( menu_number, catd, ext_terminator,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, menu_term );
                NLsprintf( msgstr, menu_term[0].text, tm_input.dname,
			 tm_input.dnameloc );
                free( menu_term[0].text );
                menu_term[0].text = (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_term[0].text, msgstr );
		ASL_rc = diag_display( menu_number, catd, NULL,
				DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
				&menutype, menu_term );
		menu_return = DIAG_ITEM_SELECTED( menutype );
		break;
        case UNPLUG_PLUG_INT     :
                ASL_rc = diag_display( menu_number, catd, unplug_plug_int_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_plug_unplug_int );
                NLsprintf( msgstr, menu_plug_unplug_int[0].text, tm_input.dname,
			 tm_input.dnameloc );
		free( menu_plug_unplug[0].text );
                menu_plug_unplug_int[0].text = (char *)malloc
			( strlen( msgstr )+1 );
                strcpy ( menu_plug_unplug_int[0].text, msgstr );
                NLsprintf( msgstr, menu_plug_unplug_int[1].text, 
			tm_input.dnameloc);
		free( menu_plug_unplug[1].text );
                menu_plug_unplug_int[1].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_plug_unplug_int[1].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_plug_unplug_int );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        case REPLUG_INT_DEV     :
                ASL_rc = diag_display( menu_number, catd, replug_int_menu,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_replug_int_dev );
                NLsprintf( msgstr, menu_replug_int_dev[0].text, tm_input.dname,
			 tm_input.dnameloc );
		free ( menu_replug_int_dev [0].text );
                menu_replug_int_dev[0].text = (char *)malloc
			( strlen( msgstr )+1 );
                strcpy ( menu_replug_int_dev[0].text, msgstr );
                NLsprintf( msgstr, menu_replug_int_dev[1].text,
			tm_input.dnameloc );
		free ( menu_replug_int_dev [1].text );
                menu_replug_int_dev[1].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_replug_int_dev[1].text, msgstr );
                ASL_rc = diag_display( menu_number, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_replug_int_dev );
                menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
        default                :
                break;
        }  /* endswitch */
        return(ASL_rc);
} /* endfunction disp_menu */
