static char sccsid[] = "@(#)80	1.5  src/bos/diag/da/sdisk/dh2disp_mn.c, dasdisk, bos411, 9428A410j 8/6/93 14:13:15";
/*
 *   COMPONENT_NAME: DASDISK
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		chk_asl_status
 *		chk_terminate_key
 *		disp_menu
 *		display_title
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include	"dh2.h"
#include        "dh2_msg.h"
#include        <diag/bit_def.h>
#include        <diag/diago.h>
#include        <diag/da.h>
#include        <diag/diag_exit.h>
#include        <diag/tm_input.h> 
#include	<diag/dcda_msg.h>
#include        <nl_types.h>
#include        <stdio.h>

struct msglist do_we_continue[] =
{
        { DH2_SET1, DO_WE_CONTINUE2,       },
        { DH2_SET1, DM_NO,               },
        { DH2_SET1, DM_YES,              },
        { DH2_SET1, CHOOSE_AN_OPTION,    },
	{ NULL,NULL}
};

struct msglist no_reset[] =
{
        { DH2_SET1, NO_RESET,       },
        { DH2_SET1, DM_PRESS_ENTER,    },
	{ NULL,NULL}
};

ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;
ASL_SCR_INFO	menu_do_we_continue[DIAG_NUM_ENTRIES(do_we_continue)];
ASL_SCR_INFO	menu_no_reset[DIAG_NUM_ENTRIES(no_reset)];

struct          tm_input tm_input;
extern          nl_catd catd;
/*  */
/******************************************************************************/
/* NAME: disp_menu()                                                            */
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

int disp_menu(msg_num)
int     msg_num;                               /* pointer to the msg number   */
{
        int     ASL_rc;                        /* return code from a function */
        long    menu_nmbr;                     /* displayed on each menu      */
        char    msgstr[512];
	char	*sub[3];

        ASL_rc = -99;
        menu_nmbr = 0x870 * 0x1000;
	switch(msg_num) {
	case	DO_WE_CONTINUE2	:
		menu_nmbr = menu_nmbr + 0x10;
                ASL_rc = diag_display( menu_nmbr, catd, do_we_continue,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, menu_do_we_continue );
                                                   /* put dname in title line */
                sprintf( msgstr, menu_do_we_continue[0].text, tm_input.dname,
			  tm_input.dnameloc);
                free( menu_do_we_continue[0].text );
                menu_do_we_continue[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_do_we_continue[0].text, msgstr );
                                       /* display menu with the substitutions */
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                       &menutype, menu_do_we_continue );
             	menu_return = DIAG_ITEM_SELECTED( menutype );
                break;

	case	NO_RESET	:
		menu_nmbr = menu_nmbr + 0x14;
                ASL_rc = diag_display( menu_nmbr, catd, no_reset,
                                       DIAG_MSGONLY,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_no_reset );
                                                   /* put dname in title line */
                sprintf( msgstr, menu_no_reset[0].text, tm_input.dname,
			  tm_input.dnameloc);
                free( menu_no_reset[0].text );
                menu_no_reset[0].text =
                                       (char * ) malloc ( strlen( msgstr )+1 );
                strcpy ( menu_no_reset[0].text, msgstr );
                                       /* display menu with the substitutions */
                ASL_rc = diag_display( menu_nmbr, catd, NULL, DIAG_IO,
                                       ASL_DIAG_KEYS_ENTER_SC,
                                       &menutype, menu_no_reset );
             	menu_return = DIAG_ITEM_SELECTED( menutype );
                break;
	case 	NO_MICROCODE_TITLE	:
		sub[0] = (char *)diag_cat_gets(catd, DH2_SET1,
				SERIAL_DISK_SUBSYSTEM);
		sub[1] = (char *)diag_cat_gets(catd, DH2_SET1,
				SERIAL_DISK_SUBSYSTEM1);
		ASL_rc = diag_display_menu(NO_MICROCODE_MENU, 0x870015, sub,
				0, 0);
		break;
 	default	:
		break;
	}
}

/*  */
/*
 * NAME: chk_asl_status()
 *
 * FUNCTION: Checks the return code from the ASL routines.
 *           Returns appropriate value to the diagnostic controller.
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

void
chk_asl_status( asl_rc )
long asl_rc;
	{
		switch ( asl_rc ) {
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
	} /* end chk_asl_status */

/*  */
/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: checks whether a user has asked to terminate the application
 *           by hitting ESC or CANCEL function key.
 *
 * EXECUTION ENVIRONMENT: This function can page fault.
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */

void
chk_terminate_key()
	    {
		if( damode & DA_CONSOLE_TRUE )
			chk_asl_status(diag_asl_read( ASL_DIAG_KEYS_ENTER_SC, NULL, NULL ));

	} /* end chk_terminate_key */

/*  */
/*
 * NAME: display_title()
 *
 * FUNCTION: Display the menu title based on diagnostic mode chosen by user.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: none.
 */

void
display_title(device_type)
int	device_type;
	    {
		char	*sub[3];

		if(damode & DA_CONSOLE_TRUE){
			sub[1] = tm_input.dname;
			sub[2] = tm_input.dnameloc;
			switch(device_type){
			case DISK:
				sub[0] = (char *)diag_cat_gets(catd, 1,
					SERIAL_DISK);
				break;
			case CONTROLLER:
				sub[0] = (char *)diag_cat_gets(catd, 1,
					SERIAL_DISK_CONTROLLER);
				break;
			case ADAPTER:
				sub[0] = (char *)diag_cat_gets(catd, 1,
					SERIAL_DISK_ADAPTER);
				break;
			default:
				return;
			}

		/*
          	Show start test menu
        	*/
			if((damode & DA_LOOPMODE_NOTLM) &&
				(damode & DA_ADVANCED_FALSE))
				     chk_asl_status(diag_display_menu
					   (CUSTOMER_TESTING_MENU,
					   0x870001, sub, tm_input.lcount, 
					   tm_input.lerrors) );
		/*
	          Show start test menu - entering loop mode
        	*/
			if ((damode & DA_ADVANCED_TRUE) && 
				(damode & DA_LOOPMODE_NOTLM))
				     chk_asl_status( diag_display_menu
					(ADVANCED_TESTING_MENU,
			   		0x870002, sub, tm_input.lcount,
					tm_input.lerrors) );
		/*
          	show status screen for loop mode
        	*/
			if ((damode & DA_ADVANCED_TRUE) &&
			    ( ( damode & DA_LOOPMODE_ENTERLM ) ||
			    ( damode & DA_LOOPMODE_INLM ) ) )
				     chk_asl_status( diag_display_menu
					 (LOOPMODE_TESTING_MENU,
				    	  0x870003, sub, tm_input.lcount,
					  tm_input.lerrors) );
		}

	} /* end display_title */

