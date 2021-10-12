/*  @(#)47 1.6 src/bos/diag/util/ubump/ubump.c, dsaubump, bos41B, 412_41B_sync 12/21/94 11:05:39 */

/*
 * COMPONENT_NAME: DSAUBUMP
 * 
 * FUNCTIONS: Diagnostic Service Aid - UBUMP
 *
 *                main
 *                dsp_chg_flag_cfg
 *                display_bump
 *                change_bump
 *		  change_passwd_bump
 *                sav_rest
 *                feprom
 *                ask_confirm
 *                ask_rolling
 *                ask_password
 *                check_password
 *                check_mode
 *                run_command
 *                int_handler
 *                genexit
 *                 
 * 
 * ORIGINS: 83 
 * 
 * (C) COPYRIGHT International Business Machines Corp.1993
 * All Rights Reserved
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <nl_types.h>
#include <locale.h>
#include <sys/access.h>
#include <sys/param.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/class_def.h"	/* object class data structures */
#include "ubump_msg.h"

#define UBUMP_MENU	0x802520
#define DSP_CHG_FLAG_CFG_MENU	0x802521
#define DSP_FLAG_MENU	0x802522
#define DSP_SERV_MENU	0x802523
#define DSP_MODEM_MENU	0x802524
#define DSP_PH_NB_MENU	0x802525
#define CHG_FLAG_MENU	0x802526
#define CHG_SERV_MENU	0x802527
#define CHG_MODEM_MENU	0x802528
#define CHG_PH_NB_MENU	0x802529
#define CHG_PASSWD_MENU	0x802530
#define SAV_REST_MENU	0x802531
#define FEPROM_DIALOG	0x802532
#define ACK_MENU	0x802533
#define CUSTOMER_DIALOG 0x802534
#define GENERAL_DIALOG	0x802535
#define ASKPASSWD_DIALOG	0x802536

#define BUMP_SERV_AID "mpcfg" /* command to execute */
#define FEPROM "feprom_update" /* command to execute */

#define ACK_YES		1
#define ACK_NO		0
#define NUMENTRYSIZE	5 /* -1 < . < 32767 => 5 figures max */
#define MODEMFILELEN	80
#define ROLLPASSLEN	7	/* # + 6 figures */
#define PASSLEN		16
#define FLAGLEN		1
#define PHONELEN	20
#define SEPARLEN	4		/* 2 spaces + 2 quotes */
#define INDLEN		SEPARLEN + 2	/* separators + 2 figures */

#define SERVICE		"service"
#define KEYLOCK		"keylock"
#define SYSOBJ		"sys0"
#define FEPROM_OPT	"-f "
#define FEPROM_OPT_NB	(sizeof(FEPROM_OPT) - 1)

#define SERVSUPMODE	"-cS"
#define MAINTENANCE	"-cw"
#define ROLLING		1
#define CUSTOMER	2
#define GENERAL		3

#define NOTYPE		0
#define OLDPASSWD	1
#define NEWPASSWD	2
#define AGAINPASSWD	3

#define VALIDATIONFLAG	3
#define MODEMCONF	1 /* first flag = modem configuration */
#define LINESPEED	2 /* Service Line Speed */
#define SITECONFINT	5

#define DIAGPASSKEY	"diagbumpkey=jwkq%374!"
#define DIAGROLL	"diagbumproll="
#define DIAGOLDP	"diagbumpop="
#define DIAGNEWP	"diagbumpnp="

#define FREE(i) if (i) {free(i);i=NULL;}

#define dspchgflagcfg_sel	1
#define savrest_sel	2
#define feprom_sel	3

#define dspflag_sel	1
#define dspserv_sel	2
#define dspmodem_sel	3
#define dspphnb_sel	4
#define chgflag_sel	5
#define chgserv_sel	6
#define chgmodem_sel	7
#define chgphnb_sel	8
#define chgpasswd_sel	9

#define save_sel	1
#define restore_sel	2

#define customer_sel	1
#define general_sel	2

#define ack_yes_sel	1
#define ack_no_sel	2

/* Global variables */
nl_catd		fdes = CATD_ERR;		/* catalog file descriptor */
int		odm_flg;
char 		*outbuf;
char 		*errbuf;

/* Extern variables */
extern ASL_SCR_TYPE dm_menutype;
extern nl_catd diag_catopen(char *, int);

/* EXTERNAL FUNCTIONS */
extern char	*diag_cat_gets();

/* FUNCTION PROTOTYPES */
void dsp_chg_flag_cfg(void);
void display_bump(char *serv_aid_opt, int title, int menu_nb);
void change_bump(char *s_a_dsp, char *s_a_chg, int title, int menu_nb);
void sav_rest(void);
void feprom(void);
int ask_confirm(void);
int ask_password(int, int, char *);
int check_mode(void);
int check_password(char *password1, char *password2);
int run_command(char *cmd,char *option);
void change_passwd_bump(void);
void int_handler(int);
void genexit(int);

/* NAME: main
 *
 * FUNCTION: BUMP service aid
 *
 * NOTES: This unit displays the 'BUMP service aid' Selection Menu
 *       and controls execution of the response. It performs
 *      the following functions:
 *      1.  Initialize the ODM.
 *      2.  Display menu and wait for response.
 *      3.  Call appropriate function according to response.
 *
 * RETURNS: 0
 *
 */

main(int argc, char *argv[])
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
struct			sigaction act;
static struct		msglist menulist1[] = {
		{ UBUMP, UBUMPTITLE },
		{ UBUMP, UBUMPOPT10 },
		{ UBUMP, UBUMPOPT13 },
		{ UBUMP, UBUMPOPT4 },
		{ UBUMP, UBUMPLASTLINE },
		{ (int )NULL, (int )NULL}
		};

	setlocale(LC_ALL,"");

	/* set up interrupt handler */
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, (struct sigaction *)NULL);

	/* inititialize ASL */
	asl_ret = diag_asl_init(ASL_INIT_DEFAULT);
	if (asl_ret != DIAG_ASL_OK)
		genexit(1);

	/* open the catalog file containing the menus */
	fdes = diag_catopen(MF_UBUMP,0);

	/* inititialize ODM */
	if ( (odm_flg = init_dgodm()) != 0 )
		genexit(1);
		
	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(UBUMP_MENU, fdes, menulist1, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch ( DIAG_ITEM_SELECTED(dm_menutype)) {
				case dspchgflagcfg_sel :
					dsp_chg_flag_cfg();
					break;
				case savrest_sel :
					sav_rest();
					break;
				case feprom_sel :
					feprom();
					break;
			}
		}
	}
	genexit(0);
}

/* NAME: dsp_chg_phnb
 *
 * FUNCTION: Display or Change Remote Support Phone Number
 *		or Modem Configuration
 *
 * NOTES: This unit displays the 'Display or Change Remote ...' Selection Menu
 *       and controls execution of the response. It performs
 *      the following functions:
 *      1.  Display menu and wait for response.
 *      2.  Call appropriate function according to response.
 *
 * RETURNS: NONE
 *
 */


/* NAME: dsp_chg_flag_cfg
 *
 * FUNCTION: Display or Change Diagostics Modes
 *
 * NOTES: This unit displays the 'Display or Change Flags and Configuration'
 *		Selection Menu and controls execution of the response.
 *	It performs the following functions:
 *      1.  Display menu and wait for response.
 *      2.  Call appropriate function according to response.
 *
 * RETURNS: NONE
 *
 */

void dsp_chg_flag_cfg(void)
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
static struct		msglist menulist2[] = {
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGTITLE },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT1 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT2 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT3 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT4 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT5 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT6 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT7 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT8 },
		{ UBUMP_DSPCHG_FLAG_CFG, DSPCHGFLAGCFGOPT9 },
		{ UBUMP, UBUMPLASTLINE },
		{ (int )NULL, (int )NULL}
		};

	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(DSP_CHG_FLAG_CFG_MENU, fdes, menulist2, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch (DIAG_ITEM_SELECTED(dm_menutype)) {
				case dspflag_sel :
					display_bump("-df", DSPFLAGTITLE,
						DSP_FLAG_MENU);
					break;
				case dspserv_sel :
					display_bump("-dS", DSPSERVTITLE,
						DSP_SERV_MENU);
					break;
				case dspmodem_sel :
					display_bump("-dm", DSPMODEMTITLE,
						DSP_MODEM_MENU);
					break;
				case dspphnb_sel :
					display_bump("-dp", DSPPHNBTITLE,
						DSP_PH_NB_MENU);
					break;
				case chgflag_sel :
					change_bump("-df","-cf", CHGFLAGTITLE,
						CHG_FLAG_MENU);
					break;
				case chgserv_sel :
					change_bump("-dS","-cS", CHGSERVTITLE,
						CHG_SERV_MENU);
					break;
				case chgmodem_sel :
					change_bump("-dm","-cm", CHGMODEMTITLE,
						CHG_MODEM_MENU);
					break;
				case chgphnb_sel :
					change_bump("-dp","-cp", CHGPHNBTITLE,
						CHG_PH_NB_MENU);
					break;
				case chgpasswd_sel :
					change_passwd_bump();
					break;
			}
		}
	}
}

/*
 * NAME: display_bump
 *
 * FUNCTION: display Diagnostic Flags, Service Support Mode,
 *           Modem Configuration or  Phone Numbers
 *
 * NOTES: This function performs the following:
 *        1. invoke the 'BUMP_service_aid' command
 *        2. display either the Diagnostic Flags, the Service Support Mode,
 *	     the Modem Configuration or the Phone Numbers
 *                                                                    
 * RETURNS: NONE
 *
 */  

void display_bump(char *serv_aid_opt, int title, int menu_nb)
{
	int 		rc;
	ASL_RC		asl_ret;	/* ASL return code */
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	char		*str; 

	outbuf = NULL;
	errbuf = NULL;

	rc = odm_run_method(BUMP_SERV_AID, serv_aid_opt, &outbuf, &errbuf );

	/* Return error if command failed */
	if (rc != 0) 
	{
		str = diag_cat_gets(fdes, UBUMP_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(str,
				diag_cat_gets(fdes, UBUMP_ERR, UNKNOWN));
		}
	}
	else
	{
		/* allocate space for 3 entries */
		menuinfo = (ASL_SCR_INFO *) calloc( 3, sizeof(ASL_SCR_INFO) );
		if (menuinfo == NULL)
		{
        		diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
			genexit(202);
		}

		/* Add title to menu */
		menuinfo[0].text = diag_cat_gets(fdes,
			UBUMP_DISPLAY, title);
	
		/* put in the list of phone numbers */
		menuinfo[1].text = outbuf;

		/* remove the last newline */
		str=(char *)strrchr(menuinfo[1].text, '\n');
		if (str!=NULL)
		{
			*str = '\0';
		}

		/* add the last line */
		menuinfo[2].text = diag_cat_gets( fdes, UBUMP_DISPLAY, DSPLASTLINE);

		menutype.max_index = 2;
		asl_ret = diag_display( menu_nb, fdes, NULL, DIAG_IO,
			   	ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   	&menutype, menuinfo);
		FREE(menuinfo);
	}

	FREE(outbuf);
	FREE(errbuf);
}

/*
 * NAME: change_bump
 *
 * FUNCTION: Change Diagnostic Flags, Service Support Modes,
 *           Modem Configuration or Phone Numbers
 *
 * NOTES: This unit displays the 'Diagnostic Flags', 'Service Support Modes',
 *	  ' Modem Configuration'  or ' Phone Numbers' update dialog
 *       and controls execution of the response. It performs
 *      the following functions:
 *      1.  Display dialog and wait for response.
 *      2.  Call BUMP_service_aid command for each modified value
 *
 * RETURNS: NONE
 *
 */

void change_bump(char *serv_aid_dsp, char *serv_aid_chg, int title, int menu_nb)
{
	int		rc;
	int		nblines;
	int		noline;
	int		strchanged; /* size of used str without '\0' */
	int		strsize; /* size of str with '\0' */
	char		*str;
	int 		error;
	ASL_RC		asl_ret;
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE	menutype = DM_TYPE_DEFAULTS;
	static		char valid_flag[]={"0,1"};
	static		char valid_speed[]={"75,150,300,1200,2400,4800,9600,19200,38400"};
	static		int valid_speed_int[]={75,150,300,1200,2400,4800,9600,19200,38400};
#define NBVALIDSPEED 	sizeof(valid_speed_int)/sizeof(int)
	static		int siteconflen[]={128,6,2,2,2,12,12,16};

	outbuf = NULL;
	errbuf = NULL;

	rc = odm_run_method(BUMP_SERV_AID, serv_aid_dsp, &outbuf, &errbuf );
	/* Return error if command failed */
	if (rc != 0) 
	{
		str = diag_cat_gets(fdes, UBUMP_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(str,
				diag_cat_gets(fdes, UBUMP_ERR, UNKNOWN));
		}
	}
	else
	{
		str = outbuf;
		nblines = 0;
		while (*str)
		{
			if (*str == '\n')	nblines++;
			str++;
		}
		/* allocate space for nblines entries */
		menuinfo = (ASL_SCR_INFO *)calloc( nblines+1,
			sizeof(ASL_SCR_INFO) );
		if (menuinfo == NULL)
		{
        		diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
			genexit(202);
		}

		/* Add title to menu */
		menuinfo[0].text = diag_cat_gets(fdes,
			UBUMP_CHANGE, title);
	
		/* put in the list */
		
		strsize = 5;
		strtok(outbuf, "\n");
		noline=1;
		while (((str = strtok(NULL, "\n")) != NULL)
			&& noline<(nblines))
		{
			char *str2;

			/* select the last field */
			/* after the last tab ('\t') */
			str2 = str + strlen(str) - 1;
			while (*str2 != '\t' && *str2 != '\0')
			{
				str2--;
			}
			*str2 = '\0';
			str2++;
			if (str[3] == '\t') str[3] = ' ';
			/* to avoid last index not displayed */
			menuinfo[noline].text = str;
			menuinfo[noline].disp_values = str2;
			switch (serv_aid_chg[2]) {
			case 'f':
			    strsize += (FLAGLEN+INDLEN);
			    menuinfo[noline].entry_type = ASL_NUM_ENTRY;
			    menuinfo[noline].op_type = ASL_RING_ENTRY;
			    menuinfo[noline].entry_size = FLAGLEN;
			    menuinfo[noline].disp_values = valid_flag;
			    if (str2[0] == '1') {
				menuinfo[noline].cur_value_index = 1;
				menuinfo[noline].default_value_index = 1;
			    }
			    break;
			case 'S':
			    menuinfo[noline].entry_type = ASL_NUM_ENTRY;
			    menuinfo[noline].entry_size = FLAGLEN;
			    strsize += (FLAGLEN+INDLEN);
			    if (noline < VALIDATIONFLAG) {
				menuinfo[noline].op_type = ASL_RING_ENTRY;
				menuinfo[noline].disp_values = valid_flag;
				if (str2[0] == '1') {
				    menuinfo[noline].cur_value_index = 1;
				    menuinfo[noline].default_value_index = 1;
				}
			    }
			    else if (noline > VALIDATIONFLAG) {
			        menuinfo[noline].entry_type = ASL_TEXT_ENTRY;
			    }
			    else {
			        menuinfo[noline].entry_size = NUMENTRYSIZE;
				strsize += NUMENTRYSIZE;
			    }
			    break;
			case 'm':
			    if (noline > MODEMCONF && noline <= SITECONFINT) {
				menuinfo[noline].entry_type = ASL_NUM_ENTRY;
				if (noline == LINESPEED) {
				    int ind, speed;

				    speed = atoi(str2);
				    for (ind = NBVALIDSPEED-1;ind > 0; ind--) {
					if (speed >= valid_speed_int[ind]) {
					    break;
					}
				    }
				    menuinfo[noline].op_type = ASL_RING_ENTRY;
				    menuinfo[noline].disp_values = valid_speed;
				    menuinfo[noline].cur_value_index = ind;
				    menuinfo[noline].default_value_index = ind;
				}
			    }
			    else {
				menuinfo[noline].entry_type = ASL_TEXT_ENTRY;
			    }
			    menuinfo[noline].entry_size = siteconflen[noline-1];
			    strsize += (siteconflen[noline-1] + INDLEN);
			    break;
			case 'p':
			    menuinfo[noline].entry_type = ASL_TEXT_ENTRY;
			    menuinfo[noline].entry_size = PHONELEN;
			    strsize += (PHONELEN+INDLEN);
			    break;
			}
			menuinfo[noline].data_value =
			    (char *) calloc(1, menuinfo[noline].entry_size+1);
			if (menuinfo[noline].data_value == NULL)
			{
        		 diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
			 genexit(202);
			}
			noline++;
		}

		/* add the last line */
		nblines = noline; /* it should be true before */
		menuinfo[noline].text = diag_cat_gets( fdes, UBUMP_CHANGE,
			(serv_aid_chg[2]=='f') ? CHGFLAGLASTLINE : CHGLASTLINE);

		menutype.max_index = noline;
		asl_ret = DIAG_ASL_OK;
		while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT
			&& asl_ret != DIAG_ASL_COMMIT )
		{
			asl_ret = diag_display( menu_nb, fdes, NULL, DIAG_IO,
			   	ASL_DIAG_DIALOGUE_SC, &menutype, menuinfo);
		}

		/* alloc a string for mpcfg parameters as if all change */
		str = (char *)calloc(1, strsize);
		if (str == NULL)
		{
        		diag_asl_msg(diag_cat_gets(fdes,
				UBUMP_ERR, MEM_ERR));
			genexit(202);
		}
		strcpy(str, serv_aid_chg);
		strchanged = strlen(serv_aid_chg);
		if (serv_aid_chg[2] == 'm')
		{
			/* to force init of modem configuration */
			menuinfo[MODEMCONF].changed = ASL_YES;
		}

		for (noline=1; noline<(nblines); noline++)
		{
			if (asl_ret == DIAG_ASL_COMMIT
			&& menuinfo[noline].changed == ASL_YES)
			{
				char *str2;

				/* search and select index (the first field) */
				str2 = menuinfo[noline].text;
				while (*str2 == ' ' || *str2 == '\t')
				{
					str2++; /* before index */
				}
				while (*str2 != ' ' && *str2 != '\t'
					&& *str2 != '\0')
				{
					str2++;
				}
				*str2 = '\0';

				/* build options */
				if ((strchanged + strlen(menuinfo[noline].text)
				   + strlen(menuinfo[noline].data_value)
				   + SEPARLEN)
				   >= strsize)
				{
					/* should not append */
        				diag_asl_msg(diag_cat_gets(fdes,
						UBUMP_ERR, MEM_ERR));
					genexit(202);
				}
				sprintf(str+strchanged," %s '%s'",
					menuinfo[noline].text,
					menuinfo[noline].data_value);
				strchanged = strchanged
					+ strlen(menuinfo[noline].text)
					+ strlen(menuinfo[noline].data_value)
					+ SEPARLEN ;
			}
			FREE(menuinfo[noline].data_value);
		}
		if ( strchanged > strlen(serv_aid_chg))
		{
			/* check if it is change Service Support Mode */
			if (!strcmp(serv_aid_chg,SERVSUPMODE))
			{
				/* ask rolling password */
				asl_ret = ask_rolling();
				if ( asl_ret == DIAG_ASL_EXIT
				|| asl_ret == DIAG_ASL_CANCEL )
					return;
			}

			/* change the value */
			(void)run_command(BUMP_SERV_AID, str);
		}
		FREE(str);
		FREE(menuinfo);
	}

	FREE(outbuf);
	FREE(errbuf);
}

/* NAME: sav_rest
 *
 * FUNCTION: Save or Restore Flags and Configuration
 *
 * NOTES: This unit displays the 'Save or Restore Flags and Configuration, ... '
 *		Selection Menu and controls execution of the response.
 *	It performs the following functions:
 *      1.  Display menu and wait for response.
 *      2.  Call appropriate function according to response.
 *
 * RETURNS: NONE
 *
 */

void sav_rest(void)
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
static struct		msglist menulist3[] = {
		{ UBUMP_SAV_REST, SAVERESTTITLE },
		{ UBUMP_SAV_REST, SAVERESTOPT1 },
		{ UBUMP_SAV_REST, SAVERESTOPT2 },
		{ UBUMP, UBUMPLASTLINE },
		{ (int )NULL, (int )NULL}
		};

	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(SAV_REST_MENU, fdes, menulist3, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch (DIAG_ITEM_SELECTED(dm_menutype)) {
				case save_sel :
					(void)run_command(BUMP_SERV_AID, "-s");
					break;
				case restore_sel :
					/* ask rolling password */
					/* not to restore contract flags
					*  => not to ask rolling password
					* asl_ret = ask_rolling();
					* if ( asl_ret == DIAG_ASL_EXIT
					* || asl_ret == DIAG_ASL_CANCEL )
					* 	return;
					*/
					(void)run_command(BUMP_SERV_AID, "-r");
					break;
			}
		}
	}
}

/*
 * NAME: feprom
 *
 * FUNCTION: update the feprom.
 *
 * NOTES: This function performs the following:
 *        1. no check if service mode done in the command
 *        2. prompt for file name
 *        3. display the dialog and wait for response
 *        4. ask confirmation
 *        5. check if file accessible
 *        6. execute the feprom command
 *                                                                    
 * RETURNS: NONE
 *
 */  

void feprom(void)
{
DIAG_ASL_RC	asl_ret;
char		*options; 
ASL_SCR_INFO    *dialoginfo;
static ASL_SCR_TYPE dialogtype = DM_TYPE_DEFAULTS;


	/* no check if service mode */
	/*if ( check_mode() == FALSE)
	{
		diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MODERR));
		return;
	}*/


	/* prompt for file name */
	/* allocate space for 3 entries */
	dialoginfo = (ASL_SCR_INFO *) calloc( 3, sizeof(ASL_SCR_INFO) );
	if (dialoginfo == NULL)
	{
        	diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
		genexit(202);
	}
	
	/* Add title to dialog */
	dialoginfo[0].text = diag_cat_gets(fdes ,UBUMP_FEPROM, FEPROMTITLE);
	
	/* filename prompt */
	dialoginfo[1].text = diag_cat_gets(fdes, UBUMP_FEPROM, FEPROMPROMPT);
	dialoginfo[1].entry_type = ASL_FILE_ENTRY;
	dialoginfo[1].entry_size = MAXPATHLEN;
	dialoginfo[1].required = ASL_YES_NON_EMPTY;
	options = (char *)calloc(1, (MAXPATHLEN + FEPROM_OPT_NB + 1));
	if (options == NULL)
	{
        	diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
		genexit(202);
	}
	strcpy(options,FEPROM_OPT);
	dialoginfo[1].data_value = options + FEPROM_OPT_NB;
	/* dialoginfo[1].data_value -> Null character */
	
	/* put instruction line */
	dialoginfo[2].text = diag_cat_gets(fdes ,UBUMP_FEPROM,FEPROMLASTLINE);
	
	dialogtype.max_index = 2;

	asl_ret = DIAG_ASL_OK;
	while ( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT ) {
		asl_ret = diag_display( FEPROM_DIALOG, fdes, NULL, DIAG_IO,
	   			ASL_DIAG_DIALOGUE_SC,
	   			&dialogtype, dialoginfo);
	
		/* issue the command to download the feprom */
		if (asl_ret == DIAG_ASL_COMMIT)
		{
			/* ask confirmation */
			if ( ask_confirm() == ACK_YES)
			{
				if (access(options + FEPROM_OPT_NB,E_ACC) != 0)
				{	/* file does not exist */
					diag_asl_msg(diag_cat_gets(fdes,
						UBUMP_ERR, MISSING),
						options + FEPROM_OPT_NB);
				}
				else  {
					(void)run_command(FEPROM, options);
				}
			}
			break;
	
	
		}
	}
	
	FREE(options);
	FREE(dialoginfo);
}


/*
 * NAME: change_passwd_bump
 *
 * FUNCTION: allows the customer maintenance password and 
 *           maintemance menu password modification.
 *
 *                                                                    
 * RETURNS: NONE
 *
 */  

void change_passwd_bump(void)
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
int			rc;
int			password_type;
int			error;
static struct		msglist menulist4[] = {
		{ UBUMP_PASSWD, CHGPASSWDTITLE },
		{ UBUMP_PASSWD, CHGPASSWDOPT1 },
		{ UBUMP_PASSWD, CHGPASSWDOPT2 },
		{ UBUMP, UBUMPLASTLINE },
		{ (int )NULL, (int )NULL}
		};
static char	option[]="-cw 1";
static char	oldpass[sizeof(DIAGOLDP)+PASSLEN] = { DIAGOLDP };
static char	newpass[sizeof(DIAGNEWP)+PASSLEN] = { DIAGNEWP };
char		new2pass[PASSLEN+1];
	

	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(CHG_PASSWD_MENU, fdes, menulist4, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch (DIAG_ITEM_SELECTED(dm_menutype)) {
				case customer_sel :
					password_type = CUSTOMER;
					option[4] = '1';
					break;
				case general_sel :
					password_type = GENERAL;
					option[4] = '2';
					break;
			}
			rc = ask_password(password_type, OLDPASSWD,
				&oldpass[sizeof(DIAGOLDP)-1]);
			if (rc == DIAG_ASL_COMMIT)
			{
			   do {
				rc = ask_password(password_type, NEWPASSWD,
					&newpass[sizeof(DIAGNEWP)-1]);
				if (rc != DIAG_ASL_COMMIT) break;
				rc = ask_password(password_type, AGAINPASSWD,
					&new2pass[0]);
				 if (rc != DIAG_ASL_COMMIT) break;
			   } while (check_password(&newpass[sizeof(DIAGNEWP)-1],
					 new2pass));
			}
			if (rc == DIAG_ASL_COMMIT)
			{
				if (putenv(oldpass)
				|| putenv(newpass)
				|| putenv(DIAGPASSKEY)) {
        				diag_asl_msg(diag_cat_gets(fdes,
						UBUMP_ERR, MEM_ERR));
					genexit(202);
				}
				
				(void)run_command(BUMP_SERV_AID, option);
			}
		}
	}
}

/*
 * NAME: run_command
 *
 * FUNCTION: executes odm_run_method.
 *
 * NOTES:
 *        1. executes the odm_run_method
 *        2. displays error message
 *
 * RETURNS: odm_run_method return code
*/
	
int run_command(char *cmd,char *option)
{
	int 		rc;
	char		*str;
	
	outbuf = NULL;
	errbuf = NULL;
	
	rc = odm_run_method(cmd, option, &outbuf, &errbuf);

	/* Show errors or result via popup window */
	if (rc != 0)
	{
		str = diag_cat_gets(fdes, UBUMP_ERR, CMDERR);
		/* an error occurs */
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(str,
				diag_cat_gets(fdes, UBUMP_ERR, UNKNOWN));
		}
	}
	else {
		/* command successful */
		if (*outbuf) {
			str = diag_cat_gets(fdes, UBUMP_ERR, CMDOK);
			diag_asl_msg(str, outbuf);
		}
	}
	
	FREE(outbuf);
	FREE(errbuf);
	return(rc);
}

/*
 * NAME: ask_confirm
 *
 * FUNCTION: ask confirmation and wait for response.
 *
 * NOTES: displays the confirmation dialog and wait for response.
 *
 * RETURNS: 
 *          DIAG_ASL_CANCEL
 *          DIAG_ASL_EXIT
 *          ACK_YES : if user selects 'yes'
 *          ACK_NO : if user selects 'no"
 */

int ask_confirm (void)
{
DIAG_ASL_RC	asl_ret;
	
static struct msglist confirm[] = {
	{ UBUMP_ASK, ASKTITLE },
	{ UBUMP_ASK, ASKYES },
	{ UBUMP_ASK, ASKNO },
	{ UBUMP_ASK, ASKLASTLINE },
	{ (int )NULL, (int )NULL}
	};

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		/* ask confirmation */
		asl_ret = diag_display(ACK_MENU, fdes, confirm, DIAG_IO,
				ASL_DIAG_LIST_CANCEL_EXIT_SC,
				NULL, NULL );
		if( asl_ret == DIAG_ASL_COMMIT ) {
			switch (DIAG_ITEM_SELECTED(dm_menutype)) {
				case ack_yes_sel :
					return (ACK_YES);
					break;
				case ack_no_sel :
					 return (ACK_NO);
					break;
			}
		}
	}
	return (asl_ret);

}

/*
 * NAME: ask_rolling
 *
 * FUNCTION: ask rolling password and wait for response.
 *
 * NOTES: load env variables and call ask_password
 *
 * RETURNS: 
 *          DIAG_ASL_CANCEL
 *          DIAG_ASL_EXIT
 *          DIAG_ASL_COMMIT
 */

int ask_rolling()
{
	int asl_ret;
	static	char pass[sizeof(DIAGROLL)+ROLLPASSLEN] = { DIAGROLL };

	/* ask rolling password */
	asl_ret = ask_password(ROLLING, NOTYPE,
		&pass[sizeof(DIAGROLL)-1]);
	if ( asl_ret == DIAG_ASL_EXIT
	|| asl_ret == DIAG_ASL_CANCEL )
		return(asl_ret);
	if (putenv(pass)
	|| putenv(DIAGPASSKEY)) {
        	diag_asl_msg(diag_cat_gets(fdes,
			UBUMP_ERR, MEM_ERR));
		genexit(202);
	}
	return(asl_ret);
}
/*
 * NAME: ask_password
 *
 * FUNCTION: ask password and wait for response.
 *
 * NOTES: displays the password dialog and wait for response.
 *
 * RETURNS: 
 *          DIAG_ASL_CANCEL
 *          DIAG_ASL_EXIT
 *          DIAG_ASL_COMMIT
 */

int ask_password(int type, int passwd_type, char *passwd)
{
	int			plen;
	int dialog, title, field, lastline;
	DIAG_ASL_RC	        asl_ret;
	ASL_SCR_INFO	        *passwdinfo;
	static ASL_SCR_TYPE dialogtype = DM_TYPE_DEFAULTS;
	
	/* allocate space for 3 entries */
	passwdinfo = (ASL_SCR_INFO *) calloc( 3,
		sizeof(ASL_SCR_INFO) );
	if (passwdinfo  == NULL)
	{
		diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, MEM_ERR));
		genexit(202);
	}

	/* password prompt */
	lastline = ASKPASSWDLASTLINE;
	if (type == ROLLING) {
		field = ASKROLLING;
		dialog = ASKPASSWD_DIALOG;
		title = ASKPASSWDTITLE;
		plen = ROLLPASSLEN;
	}
	else {
		plen = PASSLEN;
		if (type == CUSTOMER) {
	   		title = CHGPASSWDTITLE1;
			dialog = CUSTOMER_DIALOG;
		}
		else {
	   		title = CHGPASSWDTITLE2;
			dialog = GENERAL_DIALOG;
		}
		switch(passwd_type) {
		case NOTYPE:
			break;
		case OLDPASSWD:
			if (type == CUSTOMER) {
				field =  PASSWDOPT0;
			}
			else {
				field =  PASSWDOPT1;
			}
	   		break;
		case NEWPASSWD:
			field =  PASSWDOPT2;
	   		break;
		case AGAINPASSWD:
	    		field =  PASSWDOPT3;
	   		break;
		}
	}

	/* Add title to dialog */
	passwdinfo[0].text =
		diag_cat_gets(fdes ,UBUMP_PASSWD, title);

 	passwdinfo[1].text = diag_cat_gets(fdes, UBUMP_PASSWD, field);
	passwdinfo[1].entry_type = ASL_INVISIBLE_ENTRY;
	passwdinfo[1].entry_size = plen;	
	passwdinfo[1].required = ASL_YES;
	passwdinfo[1].data_value = (char *) calloc(1, plen+1);
	passwdinfo[1].disp_values = '\0';
	
        /* put instruction line */
	passwdinfo[2].text = diag_cat_gets(fdes ,UBUMP_PASSWD,lastline);

	dialogtype.max_index = 2;

	asl_ret = DIAG_ASL_OK;

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		/* ask password */
		asl_ret = diag_display(dialog, fdes, NULL, DIAG_IO,
				ASL_DIAG_DIALOGUE_SC,
				&dialogtype, passwdinfo );

		if (asl_ret == DIAG_ASL_COMMIT) {
			strcpy(passwd, passwdinfo[1].data_value);
			break;
		}
	}
	FREE(passwdinfo);
	return (asl_ret);

}

/*
 * NAME: check_mode
 *
 * FUNCTION: chek if mode service.
 *
 * NOTES: search the value of keylock attribute of sys0 object in ODM
 *
 * RETURNS:
 *           TRUE : if service mode
 *           FALSE : if not service mode
 */

int check_mode(void)
{
int 		how_many;
int		service_mode;
int 		rc;
struct CuAt	*cuat;
	

	/* search ODM customized device attribute file */
	cuat = (struct CuAt *)getattr(SYSOBJ,KEYLOCK,FALSE,&how_many);
	
	rc = strcmp(cuat->value,SERVICE);
	if (rc == 0) 
		service_mode = TRUE;
	else
		service_mode = FALSE;

	return (service_mode);
}

/*
 * NAME: check_password
 *
 * FUNCTION: check if passwords are identical.
 *
 * NOTES:    display a message if they are not identical
 *
 * RETURNS:
 *           <> 0 : if password  does not match
 */

int check_password(char *password1, char *password2)
{
	int rc;

	if (rc = strcmp(password1, password2)) {
		diag_asl_msg(diag_cat_gets(fdes, UBUMP_ERR, PASSWDNOTMATCH));
	}
	return (rc);
}

/*
 * NAME: int_handler
 *                                                                    
 * FUNCTION: Perform general clean up on receipt on an interrupt.
 *                                                                    
 * RETURNS: NONE
 *
 */  

void int_handler(int sig)
{
	genexit(1);
}

/*
 * NAME: genexit
 *                                                                    
 * FUNCTION: Perform general clean up (exit ASL menu mode, close ODM,
 *           close the catalog, and then exit with the status code.
 *                                                                    
 * RETURNS: NONE
 *
 */  

void genexit(int exitcode)
{
	/* Close ODM */
	 term_dgodm();

	/* close message catalog */
	if ( fdes != CATD_ERR )
		catclose(fdes);

	/* terminate asl */
	diag_asl_quit();

	exit(exitcode);
}

