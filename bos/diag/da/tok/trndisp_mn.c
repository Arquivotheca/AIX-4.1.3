static char sccsid[] = "@(#)29	1.6  src/bos/diag/da/tok/trndisp_mn.c, datok, bos411, 9428A410j 1/5/93 13:33:46";
/*
 *   COMPONENT_NAME: DATOK
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
#include "dtoken_msg.h"
#include "trn.h"

extern nl_catd catd;			/* pointer to message catalog  */ 
extern int fdes;			/* device driver file descriptor */
extern struct tm_input da_input;	/* holds input from ODM */
extern long da_mode;			/* diagnostic modes */
extern nl_catd diag_catopen();

/* The following structures are the message lists for the token ring menus. */
struct msglist Tlist[] = {
	{ 1, STANDBY, },
	NULL
};

struct msglist TAlist[] = {
	{ 1, STANDBY_ADVANCED, },
	NULL
};

struct msglist WTlist[] = {
	{ 1, TESTING, },
	{ 1, NONTELE_CABLE, },
	{ 1, TELE_CABLE, },
	{ 1, UNKNOWN_CABLE, },
	{ 1, WIRING_TYPE, },
	NULL
};

struct msglist NUlist[] = {
	{ 1, TESTING, },
	{ 1, YES_OPTION, },
	{ 1, NO_OPTION, },
	{ 1, NETWORK_UP, },
	NULL
};

struct msglist CClist[] = {
	{ 1, TESTING, },
	{ 1, CHECK_CABLE, },
	NULL
};

struct msglist CCTlist[] = {
	{ 1, TESTING, },
	{ 1, CHECK_CABLE_TELE, },
	NULL
};

struct msglist HWlist[] = {
	{ 1, TESTING, },
	{ 1, YES_OPTION, },
	{ 1, NO_OPTION, },
	{ 1, HAVE_WRAP, },
	NULL
};

struct msglist DClist[] = {
	{ 1, TESTING, },
	{ 1, DETACH_CABLE, },
	NULL
};

struct msglist AWlist[] = {
	{ 1, TESTING, },
	{ 1, ATTACH_WRAP, },
	NULL
};

struct msglist NBlist[] = {
	{ 1, NET_BAD, },
	NULL
};

struct msglist DWlist[] = {
	{ 1, TESTING, },
	{ 1, DETACH_WRAP, },
	NULL
};

struct msglist AClist[] = {
	{ 1, TESTING, },
	{ 1, ATTACH_CABLE, },
	NULL
};

ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

ASL_SCR_INFO asiNU[DIAG_NUM_ENTRIES(NUlist)];
ASL_SCR_INFO asiWT[DIAG_NUM_ENTRIES(WTlist)];
ASL_SCR_INFO asiCC[DIAG_NUM_ENTRIES(CClist)];
ASL_SCR_INFO asiCCT[DIAG_NUM_ENTRIES(CCTlist)];
ASL_SCR_INFO asiHW[DIAG_NUM_ENTRIES(HWlist)];
ASL_SCR_INFO asiDC[DIAG_NUM_ENTRIES(DClist)];
ASL_SCR_INFO asiDW[DIAG_NUM_ENTRIES(DWlist)];
ASL_SCR_INFO asiAC[DIAG_NUM_ENTRIES(AClist)];
ASL_SCR_INFO asiAW[DIAG_NUM_ENTRIES(AWlist)];
ASL_SCR_INFO asiNB[DIAG_NUM_ENTRIES(NBlist)];

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

int msgnum;			/* message number of menu to display */
{
   
int rc = 0;			/* return code from a function */
int setnum = 1;			/* set number within catalog */
char msgstr[512]; 		/* holds message for substitution */
	
#ifdef NOTUS 
	/* open catalog file.  If it won't open set error flag,
	 * terminate ODM and exit to diagnostic controller.
	*/
	catd = diag_catopen(CAT_NAME, 0);
	/* initialize the asl */
	diag_asl_init(NULL);
#endif NOTUS

	switch(msgnum)
	{
	case STANDBY:
		/* display "testing" message */
		rc = diag_msg_nw(S_MENU_NUM, catd, 1, STANDBY,
		    	da_input.dnameloc);
		sleep(1);
		break;
	case STANDBY_ADVANCED:
		/* display "testing in advanced mode */
		rc = diag_msg_nw(SA_MENU_NUM, catd, 1,
		   STANDBY_ADVANCED, da_input.dnameloc);
		sleep(1);
		break;
	case TESTING_INLM:
		/* display "testing multiple times */
		rc = diag_msg_nw(TI_MENU_NUM, catd, 1, TESTING_INLM,
		    da_input.dnameloc, da_input.lcount, da_input.lerrors);
		sleep(2);
		break;
	case NETWORK_UP:
		/* ask user whether network is up */
		if (da_mode & DA_ADVANCED_TRUE)
			NUlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(NU_MENU_NUM, catd, NUlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiNU);
		sprintf(msgstr, asiNU[0].text, da_input.dnameloc);
		free(asiNU[0].text);
		asiNU[0].text = msgstr;
		rc = diag_display(NU_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiNU);
		break;
	case WIRING_TYPE:
		/* ask user whether telephone wiring is used */
		if (da_mode & DA_ADVANCED_TRUE)
			WTlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(WT_MENU_NUM, catd, WTlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiWT);
		sprintf(msgstr, asiWT[0].text, da_input.dnameloc);
		free(asiWT[0].text);
		asiWT[0].text = msgstr;
		rc = diag_display(WT_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiWT);
		break;
	case CHECK_CABLE:
		/* ask user to check cable connection */
		if (da_mode & DA_ADVANCED_TRUE)
			CClist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(CC_MENU_NUM, catd, CClist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiCC);
		sprintf(msgstr, asiCC[0].text, da_input.dnameloc);
		free(asiCC[0].text);
		asiCC[0].text = msgstr;
		rc = diag_display(CC_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiCC);
		break;
	case CHECK_CABLE_TELE:
		/* ask user to check cable connection */
		if (da_mode & DA_ADVANCED_TRUE)
			CCTlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(CCT_MENU_NUM, catd, CCTlist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiCCT);
		sprintf(msgstr, asiCCT[0].text, da_input.dnameloc);
		free(asiCCT[0].text);
		asiCCT[0].text = msgstr;
		rc = diag_display(CCT_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiCCT);
		break;
	case HAVE_WRAP:
		if (da_mode & DA_ADVANCED_TRUE)
			HWlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(HW_MENU_NUM, catd, HWlist,
		    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
		    &menutype, asiHW);
		sprintf(msgstr, asiHW[0].text, da_input.dnameloc);
		free(asiHW[0].text);
		asiHW[0].text = msgstr;
		rc = diag_display(HW_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiHW);
		break;
	case DETACH_CABLE:
		/* ask user to detach cable from network, if attached */
		if (da_mode & DA_ADVANCED_TRUE)
			DClist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(DC_MENU_NUM, catd, DClist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiDC);
		sprintf(msgstr, asiDC[0].text, da_input.dnameloc);
		free(asiDC[0].text);
		asiDC[0].text = msgstr;
		rc = diag_display(DC_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiDC);
		break;
	case ATTACH_WRAP:
		/* ask user to attach telephone wrap plug */
		if (da_mode & DA_ADVANCED_TRUE)
			AWlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(AW_MENU_NUM, catd, AWlist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiAW);
		sprintf(msgstr, asiAW[0].text, da_input.dnameloc);
		free(asiAW[0].text);
		asiAW[0].text = msgstr;
		rc = diag_display(AW_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiAW);
		break;
	case NET_BAD:
		/* pass goal menu to controller */
		/*rc = diag_display(NB_MENU_NUM, catd, NBlist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiNB);
		sprintf(msgstr, asiNB[0].text, da_input.dnameloc);
		menugoal(asiNB[0].text);*/
		sprintf(msgstr, (char *)
			diag_cat_gets(catd, setnum, NET_BAD));
		menugoal(msgstr);
		break;
	case DETACH_WRAP:
		/* ask user to remove telephone wrap plug */
		if (da_mode & DA_ADVANCED_TRUE)
			DWlist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(DW_MENU_NUM, catd, DWlist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiDW);
		sprintf(msgstr, asiDW[0].text, da_input.dnameloc);
		free(asiDW[0].text);
		asiDW[0].text = msgstr;
		rc = diag_display(DW_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiDW);
		break;
	case ATTACH_CABLE:
		/* ask user to attach cable to network, if detached */
		if (da_mode & DA_ADVANCED_TRUE)
			AClist[0].msgid = TESTING_ADVANCED;
		rc = diag_display(AC_MENU_NUM, catd, AClist,
		    DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC, &menutype, asiAC);
		sprintf(msgstr, asiAC[0].text, da_input.dnameloc);
		free(asiAC[0].text);
		asiAC[0].text = msgstr;
		rc = diag_display(AC_MENU_NUM, catd, NULL, DIAG_IO,
			ASL_DIAG_KEYS_ENTER_SC, &menutype, asiAC);
		break;
	}

	if (msgnum != NET_BAD)
	switch(rc)
		{
		case ASL_EXIT:
		case ASL_CANCEL:
			/* if user hit EXIT or CANCEL key, exit DA */
			user_quit(rc);
			break;
		case ASL_COMMIT:
			switch(msgnum)
				{
				case WIRING_TYPE:
				case NETWORK_UP:
				case HAVE_WRAP:
					/* get user input */
					rc = DIAG_ITEM_SELECTED(menutype);
					break;
				}
		}
 
#ifdef NOTUS
	/* quit asl */
	diag_asl_quit(NULL);
	/* close the token ring catalog file */
	catclose(catd);
#endif NOTUS
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

user_quit(rc)
int rc;
{
	/* quit asl */
	diag_asl_quit(NULL);
	/* close catalog file */
	catclose(catd);
#ifndef NOTUS
	/* close device driver file */
	close(fdes);
#endif NOTUS
	switch(rc)
	{
	case ASL_EXIT:
		/* set exit code to tell diagnostic controller
		 * that EXIT key was pressed
		*/
		DA_SETRC_USER(DA_USER_EXIT);
		break;
	case ASL_CANCEL:
		/* set exit code to tell diagnostic controller
		 * that CANCEL key was pressed
		*/
		DA_SETRC_USER(DA_USER_QUIT);
		break;
	default:	
		/* don't do anything if ended up here by mistake */
		return(rc);
		break;
	}
		/* terminate the ODM */
		term_dgodm();
		/* exit to DC */
		DA_EXIT();
}
