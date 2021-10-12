static char sccsid[] = "@(#)71 1.1 src/bos/diag/util/ukey/ukey.c, dsaukey, bos411, 9430C411a 7/22/94 04:29:17";

/*
 * COMPONENT_NAME: DSAUKEY
 * 
 * FUNCTIONS: Diagnostic Service Aid - UKEY
 *
 *                main
 *                display_key
 *                alter_key
 *                run_command
 *                int_handler
 *                genexit
 *                 
 * 
 * ORIGINS: 83 
 * 
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <nl_types.h>
#include <locale.h>
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/class_def.h"    /* object class data structures */
#include "ukey_msg.h"

#define UKEY_MENU	0x802710
#define DSP_MENU	0x802711
#define CHG_MENU	0x802712

#define KEYCFG "keycfg" /* command to execute */

#define FREE(i) if (i) {free(i);i=NULL;}

#define	disp_sel	1
#define	chg_sel		2

#define MAXMODELEN	7


/* Global variables */
nl_catd		fdes = CATD_ERR;		/* catalog file descriptor */
int		odm_flg;
char 		*outbuf;
char 		*errbuf;

static          char KeyMode[]={"normal,secure,service"};

/* Extern variables */
extern ASL_SCR_TYPE dm_menutype;
extern nl_catd diag_catopen(char *, int);

/* EXTERNAL FUNCTIONS */
extern char	*diag_cat_gets();

/* FUNCTION PROTOTYPES */
void display_key(void);
void alter_key(void);
int run_command(char *cmd,char *option);
void int_handler(int);
void genexit(int);

/*
 * NAME: main
 *
 * FUNCTION: Display or Change Electronic Mode Switch
 * 
 * NOTES: This unit displays the 'Display Mode Switch' 
 *      and 'Change Electronic Mode Switch' menus
 *	and controls execution of the response. It performs
 *	the following functions:
 *	1.  Initialize the ODM.
 *	2.  Display menu and wait for response. 
 *	3.  Call appropriate function according to response.
 *
 * RETURNS: 0
 *
 */

main(int argc, char *argv[])
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
struct			sigaction act;
int			selection;
static struct		msglist menulist[] = {
		{ UKEY, UKEYTITLE },
		{ UKEY, UKEYOPT1 },
		{ UKEY, UKEYOPT2 },
		{ UKEY, UKEYLASTLINE },
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
	fdes = diag_catopen(MF_UKEY,0);

	/* inititialize ODM */
	if ( (odm_flg = init_dgodm()) != 0 )
		genexit(1);
		
	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(UKEY_MENU, fdes, menulist, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch ( selection = DIAG_ITEM_SELECTED(dm_menutype)) {
				case disp_sel :
					display_key();
					break;
				case chg_sel:
					alter_key();
					break;
			}
		}
	}
	genexit(0);
}

/*
 * NAME: display_key
 *                                                                    
 * FUNCTION: Display the Mode Switch
 *                                                                    
 * NOTES: This function invokes the command 'keycfg' to display the
 *	  Mode Switch.
 *
 * RETURNS: NONE
 *
 */  

void display_key(void)
{
	int 		rc;
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	char		*str; 

	outbuf = NULL;
	errbuf = NULL;

	rc = odm_run_method(KEYCFG, "-d", &outbuf, &errbuf );

	/* Return error if command failed */
	if (rc != 0) 
	{
		str = diag_cat_gets(fdes, UKEY_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(diag_cat_gets(fdes, UKEY_ERR, UNKNOWN));
		}
	}
	else
	{
		/* allocate space for 3 entries */
		menuinfo = (ASL_SCR_INFO *) calloc( 3, sizeof(ASL_SCR_INFO) );
		if (menuinfo == NULL)
		{
        		diag_asl_msg(diag_cat_gets(fdes, UKEY_ERR, MEM_ERR));
			genexit(202);
		}

		/* Add title to menu */
		menuinfo[0].text = diag_cat_gets(fdes,DSP_UKEY, DSPUKEYTITLE);
	
		/* put in the list of devices */
		menuinfo[1].text = outbuf;

		/* remove the last newline */
		str=(char *)strrchr(menuinfo[1].text, '\n');
		if (str!=NULL)
		{
			*str = '\0';
		}

		/* add in the last line */
		menuinfo[2].text = diag_cat_gets( fdes, DSP_UKEY, DSPUKEYLASTLINE);

		menutype.max_index = 2;
		(void)diag_display( DSP_MENU, fdes, NULL, DIAG_IO,
			   	ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   	&menutype, menuinfo);
		FREE(menuinfo);
	}

	FREE(outbuf);
	FREE(errbuf);
}

/*
 * NAME: alter_key  
 *
 * FUNCTION: change the Electronic Mode Switch.
 * 
 * NOTES: This unit performs the following functions:
 *	1. Prompt the user for Electronic Mode Switch value.
 *      2. Invoke keycfg -c <EModeSwitch>.
 *
 * RETURNS: NONE
 *
 */

void alter_key(void)
{
	DIAG_ASL_RC	asl_ret;
	ASL_SCR_INFO    *dialoginfo;
	static ASL_SCR_TYPE dialogtype = DM_TYPE_DEFAULTS;
	char            cmd_args[128];

	/* allocate space for 3 entries */
	dialoginfo = (ASL_SCR_INFO *) calloc( 3,
					sizeof(ASL_SCR_INFO) );
	if (dialoginfo == NULL)
	{
		diag_asl_msg(diag_cat_gets(fdes, UKEY_ERR, MEM_ERR));
		genexit(202);
	}

	/* Add title to dialog */
	dialoginfo[0].text =
			diag_cat_gets(fdes ,CHG_UKEY, CHGUKEYTITLE);

	/* electronic mode switch prompt */
	dialoginfo[1].text = diag_cat_gets(fdes, CHG_UKEY, ALTERPROMPT);
	dialoginfo[1].op_type = ASL_RING_ENTRY;
	dialoginfo[1].entry_type = ASL_NO_ENTRY;
	dialoginfo[1].entry_size = MAXMODELEN;
	dialoginfo[1].required = ASL_YES;
	dialoginfo[1].disp_values = KeyMode;
	dialoginfo[1].data_value = (char *) calloc(1, MAXMODELEN+1);

	/* put instruction line */
	dialoginfo[2].text = diag_cat_gets(fdes ,CHG_UKEY,ALTERLASTLINE);

	dialogtype.max_index = 2;

	asl_ret = DIAG_ASL_OK;

	while ( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )
	{
		asl_ret = diag_display( CHG_MENU, fdes, NULL, DIAG_IO,
   			ASL_DIAG_DIALOGUE_SC, &dialogtype, dialoginfo);

		if ( asl_ret == DIAG_ASL_COMMIT )
		{
		/* issue the command to alter Electronic Mode Switch */
		sprintf(cmd_args,"-c %s", dialoginfo[1].data_value);
		(void)run_command(KEYCFG, cmd_args);
		}
	}	
	FREE(dialoginfo);
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

	if (rc != 0) {
		/* an error occurs */
		str = diag_cat_gets(fdes, UKEY_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(diag_cat_gets(fdes, UKEY_ERR, UNKNOWN));
		}
	}
	else {
		/* command successful */
		if (*outbuf) {
			str = diag_cat_gets(fdes, UKEY_ERR, CMDOK);
			diag_asl_msg(str, outbuf);
		}
	}

	FREE(outbuf);
	FREE(errbuf);
	return(rc);
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
  struct sigaction act;

	genexit(1);
}

/*
 * NAME: genexit
 *                                                                    
 * FUNCTION: Perform general clean up (exit ASL menu mode, close ODM,
 *           close the catalog, and then exit with the status code.)
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

