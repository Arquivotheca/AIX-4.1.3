static char sccsid[] = "@(#)35	1.4  src/bos/diag/util/ueth/eth/sa_disp_mn.c, dsaueth, bos41J, 9523B_all 6/6/95 16:14:16";
/*
 *   COMPONENT_NAME: DSAUETH
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		clean_up
 *		create_list
 *		disp_menu
 *		find_all_adaps
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include <stdio.h>
#include <nl_types.h>
#include "sys/cfgdb.h"
#include "diag/diago.h"
#include "diag/class_def.h"
#include "uenet_msg.h"
#include "uenet.h"

extern nl_catd catd;			/* pointer to message catalog  */
extern int fdes;

int setnum = 1;

struct msglist Dlist[] = {
	{ 1, DESCRIP_TITLE , },
	{ 1, DESCRIP, },
	{ 1, NO_ERR , },
	{ 1, ADAP_ERR , },
	{ 1, TX_TIME , },
	{ 1, TX_ERR , },
	{ 1, RX_TIME , },
	{ 1, RX_ERR , },
	{ 1, SYS_ER , },
	{ 1, WRAP_ERR , },
	{ 1, UNID_ERR , },
	{ 1, NO_ADAPS , },
	{ 1, ADAP_BUSY , },
	{ 1, ENTER , },
	NULL
};

struct msglist WClist[] = {
	{ 1, WC_TITLE, },
	{ 1, DIX_OPTION, },
	{ 1, BNC_OPTION, },
	{ 1, WHICH_CON, },
	NULL
};

struct msglist NAlist[] = {
	{ 1, RESULTS, },
	{ 1, NO_ADAPS, },
	NULL
};

struct msglist Rlist[] = {
	{ 1, RESULTS , },
	{ 1, NO_ERR, },
	{ 1, ENTER, },
	NULL
};

ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
ASL_SCR_INFO *entinfo;
ASL_SCR_TYPE enttype = DM_TYPE_DEFAULTS;
ASL_SCR_INFO asiWC[DIAG_NUM_ENTRIES(WClist)];
char *dname[8];
extern char *dnameloc[8];
extern int adap_num;

/***************************************************************************/
/*
 * NAME: disp_menu
 *
 * FUNCTION: handles menu displays
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: rc = user input from menu displayed
 */

int disp_menu(msgnum)

int msgnum;
{
   
	int 	rc = 0; 	/* return code from a function */
	char 	msgstr[512]; 
	int 	num_adaps;

	switch(msgnum)
		{
		case DESCRIP:
			rc = diag_display(DESCRIP_NUM, catd, Dlist, DIAG_IO,
			    ASL_DIAG_NO_KEYS_ENTER_SC, NULL, NULL);
			break;
		case ACTION:
			num_adaps = create_list();
			if (num_adaps <= 0)		/* no adapters */
				return(num_adaps);
			rc = diag_display(ACTION_NUM, catd, NULL, DIAG_IO,
	    		    ASL_DIAG_LIST_CANCEL_EXIT_SC, &enttype, entinfo);
			break;
		case WHICH_CON:
			rc = diag_display(WC_NUM, catd, WClist,
		    	    DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			    &menutype, asiWC);
			sprintf(msgstr, asiWC[3].text, dname[adap_num],
			    dnameloc[adap_num]);
			free(asiWC[3].text);
			asiWC[3].text = msgstr;
			rc = diag_display(WC_NUM, catd, NULL, DIAG_IO,
			    ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, asiWC);
			break;
		case STANDBY:
			rc = diag_msg_nw(STANDBY_NUM, catd, setnum, STANDBY,
			    dname[adap_num], dnameloc[adap_num]);
			break;
		case ADAP_ERR:
			Rlist[1].msgid = ADAP_ERR;
			break;
		case TX_TIME:
			Rlist[1].msgid = TX_TIME;
			break;
		case TX_ERR:
			Rlist[1].msgid = TX_ERR;
			break;
		case RX_TIME:
			Rlist[1].msgid = RX_TIME;
			break;
		case RX_ERR:
			Rlist[1].msgid = RX_ERR;
			break;
		case SYS_ER:
			Rlist[1].msgid = SYS_ER;
			break;
		case WRAP_ERR:
			Rlist[1].msgid = WRAP_ERR;
			break;
		case UNID_ERR:
			Rlist[1].msgid = UNID_ERR;
			break;
		case NO_ADAPS:
			Rlist[1].msgid = NO_ADAPS;
			break;
		case ADAP_BUSY:
			Rlist[1].msgid = ADAP_BUSY;
			break;
		}

	switch(msgnum)
		{
		case NO_ERR:
		case ADAP_ERR:
		case TX_TIME:
		case TX_ERR:
		case RX_TIME:
		case RX_ERR:
		case SYS_ER:
		case WRAP_ERR:
		case UNID_ERR:
		case NO_ADAPS:
		case ADAP_BUSY:
			rc = diag_display(RESULTS_NUM, catd, Rlist, DIAG_IO,
			    ASL_DIAG_KEYS_ENTER_SC, NULL, NULL);
			break;
		}

	switch(rc)
		{
		case ASL_EXIT:
		case ASL_CANCEL:
			clean_up();
			break;
		case ASL_COMMIT:
			switch(msgnum)
				{
				case ACTION:
					rc = DIAG_ITEM_SELECTED(enttype);
					adap_num = rc - 1;	
					break;
				case WHICH_CON:
					rc = DIAG_ITEM_SELECTED(menutype);
					break;
				}
			break;
		}

	return(rc);
}
/*************************************************************************/
/*
 * NAME: create_list
 *
 * FUNCTION: creates menu structure for ACTION menu according to which
 *	     ethernet adapters are in the system
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: num_adaps = number of ethernet adapters in system
 */
create_list()
{
	int num_adaps;
	int	index;
	int	line = 0;
	char	*string;
	char	*string2[8];

	num_adaps = find_all_adaps();
	if ((!entinfo) && (num_adaps > 0))
		{
		entinfo = (ASL_SCR_INFO *)
		    calloc(1, (num_adaps+2)*sizeof(ASL_SCR_INFO));
		string = (char *)malloc(132);
		string = (char *)diag_cat_gets(catd, setnum,
			TRANSMIT);
		entinfo[line++].text = (char *) diag_cat_gets(catd, setnum, 
			ACTION);
		
		for(index = 0; index < num_adaps; index++)
			{
			string2[index] = (char *)malloc(132);
			sprintf(string2[index], string, dname[index],
			    dnameloc[index]);
			entinfo[line].text = string2[index];
			entinfo[line].non_select = ASL_NO;
			line++;
			}
		entinfo[line].text = (char *) diag_cat_gets(catd, setnum, 
			SELECT);
		enttype.max_index = line;
		}
	return(num_adaps);
}
/********************************************************************/
/*
 * NAME: find_all_adaps
 *
 * FUNCTION: finds out what adapters are in the system by looking in the cudv 
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: num_found = number of ethernet adapters in system
 *			      	     -1 = error occurred
 */
int find_all_adaps()
{
	char criteria[40];
	struct listinfo obj_info;
	struct CuDv *cudv;
	int count, num_found;

	sprintf(criteria, "name LIKE ent? AND chgstatus != %d", MISSING);
	cudv = get_CuDv_list(CuDv_CLASS, criteria, &obj_info, 8, 1);
	if (cudv == (struct CuDv *) -1)
		return(-1);

	num_found = obj_info.num;

	for (count = 0; count < num_found; count ++)
		{
		dname[count] = (char *) malloc(13);
		strcpy(dname[count], cudv[count].name);
		dnameloc[count] = (char *) malloc(13);
		strcpy(dnameloc[count], cudv[count].location);
		}
	return(num_found);
}
/********************************************************************/
/*
 * NAME: clean_up
 *
 * FUNCTION: cleans up and exits
 *
 * NOTES:
 *
 * RETURN VALUE DESCRIPTIONS: none
 */
clean_up()
{
	int	result ;

	if (fdes > 0)
		close(fdes);
	configure_lpp_device ();
	term_dgodm();
	diag_asl_quit(NULL);
	catclose(catd);
	exit (0);
}
/************************************************************************/ 
