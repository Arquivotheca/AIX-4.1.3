static char sccsid[] = "@(#)35  1.1  src/bos/diag/da/mpa/dampa_asl.c, mpada, bos411, 9428A410j 4/30/93 12:18:25";
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: diag_action
 *		diag_asl_stat
 *		diag_stdby
 *		diag_y_n
 *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include        "dampa.h"

/*
 * NAME: diag_asl_stat
 *
 * FUNCTION:  Checks keyboard input by user during diagnostics.  The escape
 *      or F3 keys will cause the DA to return to the controller with no
 *      further action.  The enter key will be treated as a command to continue
 *      testing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     diag_asl_stat (aslrc)
long    aslrc;
{
	switch (aslrc)
	{
	case ASL_ERR_SCREEN_SIZE:
	case ASL_FAIL:
	case ASL_ERR_NO_TERM:
	case ASL_ERR_NO_SUCH_TERM:
	case ASL_ERR_INITSCR:
	        aslrc = SW_ERR;
	        if (fru_set == TRUE)
	                aslrc = TU_BAD;
	        break;
	case ASL_EXIT:
	        aslrc = USR_EXIT;
	        break;
	case ASL_CANCEL:
	        aslrc = USR_QUIT;
	        break;
	case ASL_OK:
	case ASL_ENTER:
	default:
	        aslrc = TU_GOOD;
	        break;
	}
	return (aslrc);
}  /* end diag_asl_stat */

/*
 * NAME: diag_stdby
 *
 * FUNCTION:  Displays a title line that describes the adapter/device being
 *            tested, a "please standby" message in option checkout.
 *            In loop mode the standby message is replaced by the loop
 *            count, error count and cancel message.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     diag_stdby (menu_num, msg_id)
long    menu_num;
int     msg_id;
{
	int     aslrc;
	char    *msgstr;
	struct  msglist da_menu[] =
	{
		{ DMPA_DA, DMMPA_1, },
	        (int)NULL
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];
	static  ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;

	memset (menu_da, 0, sizeof (menu_da));
	da_menu[0].msgid = msg_id;

	diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
	    ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);

	if (da_menu[0].msgid != DMMPA_3) {
	     msgstr = (char *) malloc ( (strlen(menu_da[0].text))+
		 (sizeof(da_input.dname))+(sizeof(da_input.dnameloc)));

	     sprintf (msgstr, menu_da[0].text, da_input.dname, da_input.dnameloc);
	     free (menu_da[0].text);
	     menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
	     strcpy (menu_da[0].text, msgstr);

	     free (msgstr);
	}
	else {
	     msgstr = (char *) malloc ( (strlen(menu_da[0].text))+
		 (sizeof(da_input.dname))+(sizeof(da_input.dnameloc))+
		 (sizeof(da_input.lcount))+(sizeof(da_input.lerrors)));

	     sprintf (msgstr, menu_da[0].text, da_input.dname,
		     da_input.dnameloc, da_input.lcount, da_input.lerrors);

	     free (menu_da[0].text);
	     menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
	     strcpy (menu_da[0].text, msgstr);
	     free (msgstr);

	} /* endif */
	aslrc = diag_display (menu_num, catd, NULL, DIAG_IO,
	    ASL_DIAG_OUTPUT_LEAVE_SC, &menutype, menu_da);
	free (menu_da[0].text);
	return (aslrc);
}  /* end diag_stdby */

/*
 * NAME: diag_action
 *
 * FUNCTION:  Displays a title line that describes the adapter/device being
 *            test.  Prompts the user to perform some action before continuing.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     diag_action (menu_num, msg_id)
long    menu_num;
int     msg_id;
{
	int     aslrc;
	char    *msgstr;
	struct  msglist da_menu[] =
	{
		{ DMPA_DA, DMMPA_4, },
		{ DMPA_DA, DMMPA_9, },
	        (int)NULL
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];
	static  ASL_SCR_TYPE    menutype = DM_TYPE_DEFAULTS;

	memset (menu_da, 0, sizeof (menu_da));
	da_menu[1].msgid = msg_id;
	diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
	    ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
	msgstr = (char *) malloc ((strlen(menu_da[0].text))+
	    (sizeof(da_input.dname))+(sizeof(da_input.dnameloc)));
	sprintf (msgstr, menu_da[0].text, da_input.dname, da_input.dnameloc);
	free (menu_da[0].text);
	menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
	strcpy (menu_da[0].text, msgstr);
	free (msgstr);
	aslrc = diag_display (menu_num, catd, NULL, DIAG_IO,
	    ASL_DIAG_KEYS_ENTER_SC, &menutype, menu_da);
	aslrc = diag_asl_stat (aslrc);
	free (menu_da[0].text);
	return (aslrc);
}  /* end diag_action */

/*
 * NAME: diag_y_n
 *
 * FUNCTION:  Displays a title line that describes the adapter/device being
 *            tested.  Allows the user to make a choice from a menu.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: NONE
 */

int     diag_y_n (menu_num, msg_id, cursor)
long    menu_num;
int     msg_id;
int     *cursor;
{
	int     aslrc;
	char    *msgstr;
	struct  msglist da_menu[] =
	{
		{ DMPA_DA, DMMPA_4, },
		{ DMPA_DA, DMMPA_5, },
		{ DMPA_DA, DMMPA_6, },
		{ DMPA_DA, DMMPA_7, },
	        (int)NULL
	};
	ASL_SCR_INFO    menu_da[DIAG_NUM_ENTRIES(da_menu)];
	ASL_SCR_TYPE    menutype = {ASL_DIAG_LIST_CANCEL_EXIT_SC, 3, 1};

	memset (menu_da, 0, sizeof (menu_da));
/*      menutype.screen_code = ASL_DIAG_LIST_CANCEL_EXIT_SC;
	menutype.max_index = 3;
	menutype.cur_index = 1; */
	da_menu[3].msgid = msg_id;
	diag_display (menu_num, catd, da_menu, DIAG_MSGONLY,
	    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
	msgstr = (char *) malloc ((strlen(menu_da[0].text))+
	    (sizeof(da_input.dname))+(sizeof(da_input.dnameloc)));
	sprintf (msgstr, menu_da[0].text, da_input.dname, da_input.dnameloc);
	free (menu_da[0].text);
	menu_da[0].text = (char *) malloc (strlen(msgstr)+1);
	strcpy (menu_da[0].text, msgstr);
	free (msgstr);
	aslrc = diag_display (menu_num, catd, NULL, DIAG_IO,
	    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, menu_da);
	aslrc = diag_asl_stat (aslrc);
	*cursor = menutype.cur_index;
	free (menu_da[0].text);
	return (aslrc);
}  /* end diag_y_n */
