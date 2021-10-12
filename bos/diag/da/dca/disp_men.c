static char sccsid[] = "@(#)32	1.6  src/bos/diag/da/dca/disp_men.c, dadca, bos411, 9428A410j 1/5/93 09:16:58";
/*
 *   COMPONENT_NAME: DADCA
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		disp_menu
 *		user_quit
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define NEW_DIAG_ASL	1
#define NEW_DIAG_CONTROLLER	1

#include <stdio.h>
#include <nl_types.h>
#include "diag/da.h"
#ifndef CATD_ERR
#define CATD_ERR -1
#endif
#include "diag/diag_exit.h"
#include "diag/diago.h"
#include "diag/tm_input.h"
#include "diag/bit_def.h"
#include "ddca_msg.h"
#include "dca.h"

extern nl_catd catd;			/* pointer to message catalog  */
extern int fdes;			/* device driver file descriptor */
extern struct tm_input da_input;	/* input from diagnostic controller */
extern long da_mode;			/* diagnostic modes */

/* The following structures define the message lists for the DCA menus */

struct msglist SBClist[] = {
	{ 1, TITLE, },
	{ 1, YES_OPTION, },
	{ 1, NO_OPTION, },
	{ 1, SHOULD_BE_COMM, },
	0   
};

struct msglist ICClist[] = {
	{ 1, TITLE, },
	{ 1, YES_OPTION },
	{ 1, NO_OPTION, },
	{ 1, IS_CABLE_CONNECTED, },
	0    
};

struct msglist ICDlist[] = {
	{ 1, TITLE, },
	{ 1, YES_OPTION },
	{ 1, NO_OPTION, },
	{ 1, IS_CABLE_DAMAGED, },
	0   
};

struct msglist ICAlist[] = {
	{ 1, TITLE, },
	{ 1, YES_OPTION },
	{ 1, NO_OPTION, },
	{ 1, IS_CABLE_AVAILABLE, },
	0    
};

struct msglist ICPlist[] = {
	{ 1, TITLE, },
	{ 1, YES_OPTION },
	{ 1, NO_OPTION, },
	{ 1, IS_CU_PROBLEM, },
	0   
};


ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

ASL_SCR_INFO asiSBC[DIAG_NUM_ENTRIES(SBClist)];
ASL_SCR_INFO asiICC[DIAG_NUM_ENTRIES(ICClist)];
ASL_SCR_INFO asiICD[DIAG_NUM_ENTRIES(ICDlist)];
ASL_SCR_INFO asiICA[DIAG_NUM_ENTRIES(ICAlist)];
ASL_SCR_INFO asiICP[DIAG_NUM_ENTRIES(ICPlist)];

ASL_SCR_INFO menu_info;

/***********************************************************************/
/*
 * NAME: disp_menu
 *
 * FUNCTION: This function displays the requested menu.
 *
 * EXECUTION ENVIRONMENT:
 * 
 * RETURNS: Item selected by user where required; otherwise 0.
 *
 */

int disp_menu(msgnum)

int msgnum;		/* The message number of the menu to be displayed */
{
	int rc = 0;	/* user input returned to DA */ 
	int mrc = 0;	/* returned from asl menu display routine */
	int setnum = 1;	/* set number within catalog file */

	char msgstr[512]; 		/* holds message for substitution */
	char buffer[512]; 		/* holds menu number for substitution */

	if 	/* da mode is not console, do not display */
	    ( !( da_mode & DA_CONSOLE_TRUE))
	{
		return (0);
	}

	switch(msgnum)
	{
	case STANDBY:
		/* Display menu if testing in customer mode and not loop mode */
		mrc = diag_msg_nw(S_MENU_NUM, catd, setnum, STANDBY,
		    da_input.dnameloc);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		break;
	case STANDBY_ADVANCED:
		/* Display menu if testing in advanced mode and not loop mode */
		mrc = diag_msg_nw(SA_MENU_NUM, catd, setnum,
		    STANDBY_ADVANCED, da_input.dnameloc);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		break;
	case TESTING_INLM:
		/* Display menu if testing in loop mode */
		mrc = diag_msg_nw(TI_MENU_NUM, catd, setnum,
		    TESTING_INLM, da_input.dnameloc, da_input.lcount,
	   	    da_input.lerrors);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		break;
	case SHOULD_BE_COMM:
		/* Ask user if adapter and CU should be communicating. */
		if (da_mode & DA_ADVANCED_TRUE)
			SBClist[0].msgid = TITLE_ADVANCED;
		mrc = diag_display(SBC_MENU_NUM, catd, SBClist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiSBC);
		sprintf(msgstr, asiSBC[0].text, da_input.dnameloc);
		free(asiSBC[0].text);
		asiSBC[0].text = msgstr;
		mrc = diag_display(SBC_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiSBC);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		/* Get user input. */
		if (mrc == ASL_COMMIT)
			rc = DIAG_ITEM_SELECTED(menutype);
		break;
	case IS_CABLE_CONNECTED:
		/* Ask user if there is a cable connecting adapter and CU. */
		if (da_mode & DA_ADVANCED_TRUE)
			ICClist[0].msgid = TITLE_ADVANCED;
		mrc = diag_display(ICC_MENU_NUM, catd, ICClist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiICC);
		sprintf(msgstr, asiICC[0].text, da_input.dnameloc);
		free(asiICC[0].text);
		asiICC[0].text = msgstr;
		mrc = diag_display(ICC_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiICC);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		/* Get user input. */
		if (mrc == ASL_COMMIT)
			rc = DIAG_ITEM_SELECTED(menutype);
		break;
	case IS_CABLE_DAMAGED:
		/* Ask user if cable is damaged. */
		if (da_mode & DA_ADVANCED_TRUE)
			ICDlist[0].msgid = TITLE_ADVANCED;
		mrc = diag_display(ICD_MENU_NUM, catd, ICDlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiICD);
		sprintf(msgstr, asiICD[0].text, da_input.dnameloc);
		free(asiICD[0].text);
		asiICD[0].text = msgstr;
		mrc = diag_display(ICD_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiICD);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		/* Get user input. */
		if (mrc == ASL_COMMIT)
			rc = DIAG_ITEM_SELECTED(menutype);
		break;
	case IS_CABLE_AVAILABLE:
		/* Ask user if a good cable is available. */
		if (da_mode & DA_ADVANCED_TRUE)
			ICAlist[0].msgid = TITLE_ADVANCED;
		mrc = diag_display(ICA_MENU_NUM, catd, ICAlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiICA);
		sprintf(msgstr, asiICA[0].text, da_input.dnameloc);
		free(asiICA[0].text);
		asiICA[0].text = msgstr;
		mrc = diag_display(ICA_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiICA);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		/* Get user input. */
		if (mrc == ASL_COMMIT)
			rc = DIAG_ITEM_SELECTED(menutype);
		break;
	case IS_CU_PROBLEM:
		/* Ask user if there is a problem with CU or system. */ 
		if (da_mode & DA_ADVANCED_TRUE)
			ICPlist[0].msgid = TITLE_ADVANCED;
		mrc = diag_display(ICP_MENU_NUM, catd, ICPlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiICP);
		sprintf(msgstr, asiICP[0].text, da_input.dnameloc);
		free(asiICP[0].text);
		asiICP[0].text = msgstr;
		mrc = diag_display(ICP_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiICP);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		/* Get user input. */
		if (mrc == ASL_COMMIT)
			rc = DIAG_ITEM_SELECTED(menutype);
		break;
	case PLEASE_CONNECT:
		/* Ask user to connect cable. */
		if (da_mode & DA_ADVANCED_TRUE)
			msgnum = PLEASE_CONNECT_ADVANCED;
		mrc = diag_msg(PC_MENU_NUM, catd, setnum, msgnum,
		    da_input.dnameloc);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		break;
	case CONNECT_CABLE:
		/* Get menu that tells user a cable might fix the problem.
		 * Pass the menu to the controller to be displayed as a 
		 * goal menu.
		 * NOTE: menu number is 85400A, though not listed in dca.h
		*/
		sprintf(msgstr, (char *) diag_cat_gets(catd, setnum, 
			CONNECT_CABLE), da_input.dnameloc);
		menugoal(msgstr);
		break;
	case CHECK_KBD:
		/* Check for ESC key entered by user. */
		mrc = diag_asl_read(ASL_DIAG_LIST_CANCEL_EXIT_SC, FALSE, 
			NULL);
		if ((mrc == ASL_EXIT) || (mrc == ASL_CANCEL))
			user_quit(mrc);
		break;
		
	}

	return(rc);

}                                   /* end of display menu            */

/**********************************************************************/
/*
 * NAME: user_quit
 *
 * FUNCTION: This function processes an ESC/CANCEL entered by the user.
 *
 * EXECUTION ENVIRONMENT:
 * 
 * RETURNS: NONE - If properly called, function causes EXIT from application.
 *		 - Otherwise it will simply return its input.
 *
 */
user_quit(mrc)

int mrc;
{
	/* Quit ASL. */
	diag_asl_quit(NULL);

	/* Close catalog file */
	catclose(catd);

	/* Close device driver. */
	close(fdes);

	/* if ASL_EXIT or ASL_CANCEL is returned by asl from menu routine,
	 * set exit code.  Otherwise return.
	*/
	switch(mrc)
	{
	case ASL_EXIT:
		DA_SETRC_USER(DA_USER_EXIT);
		break;
	case ASL_CANCEL:
		DA_SETRC_USER(DA_USER_QUIT);
		break;
	default:
		return(mrc);
	}
	/* Terminate ODM. */
	term_dgodm();

	/* Exit to diagnostic controller. */
	DA_EXIT();
}
