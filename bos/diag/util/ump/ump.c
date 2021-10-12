/*  @(#)45 1.2 src/bos/diag/util/ump/ump.c, dsaump, bos411, 9428A410j 4/20/94 08:32:37 */

/*
 * COMPONENT_NAME: DSAUMP
 * 
 * FUNCTIONS: Diagnostic Service Aid - UMP
 *
 *                main
 *                disp_or_chg_processor
 *                display_processor
 *                alter_processor
 *                get_processor
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
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/class_def.h"    /* object class data structures */
#include "ump_msg.h"

#define UMP_MENU	0x802500
#define UMPPRST_MENU	0x802501
#define UMPDSP_MENU	0x802502
#define PROC_DISABLE	0x802503
#define PROC_ENABLE	0x802504

#define CPUSTATE "cpu_state" /* command to execute */

#define PROCLEN	4 /* proc word is 4 characters length */

#define FREE(i) if (i) {free(i);i=NULL;}

#define	disp_or_chg_sel	1
#define	display_sel	1
#define	disable_sel	2
#define	enable_sel	3

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
void disp_or_chg_processor(void);
void display_processor(void);
void alter_processor(int);
int get_processor(int *, int);
int run_command(char *cmd,char *option);
void int_handler(int);
void genexit(int);

/*
 * NAME: main
 *
 * FUNCTION: Display or Change Processors State Menu
 * 
 * NOTES: This unit displays the 'Display or Change Processors State
 *	Selection' Menu and controls execution of the response. It performs
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
static struct		msglist menu1list[] = {
		{ UMP, UMPTITLE },
		{ UMP, UMPOPT1 },
		{ UMP, UMPLASTLINE },
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
	fdes = diag_catopen(MF_UMP,0);

	/* inititialize ODM */
	if ( (odm_flg = init_dgodm()) != 0 )
		genexit(1);
		
	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(UMP_MENU, fdes, menu1list, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch ( selection = DIAG_ITEM_SELECTED(dm_menutype)) {
				case disp_or_chg_sel :
					disp_or_chg_processor();
					break;
			}
		}
	}
	genexit(0);
}

/*
 * NAME: disp_or_chg_processor
 *                                                                    
 * FUNCTION: Display or Change Processor States Menu
 * 
 * NOTES: This function displays the 'Display or Change Processor States
 *	Selection' Menu and controls execution of the response. It performs
 *	the following functions:
 *	1.  Display menu and wait for response. 
 *	2.  Call appropriate function according to response.
 *                                                                    
 * RETURNS: NONE
 *
 */  

void disp_or_chg_processor(void)
{
DIAG_ASL_RC		asl_ret;	/* ASL return code */
struct			sigaction act;
int			selection;
static struct		msglist menu2list[] = {
		{ UMP_PRST, PRSTTITLE },
		{ UMP_PRST, PRSTOPT1 },
		{ UMP_PRST, PRSTOPT2 },
		{ UMP_PRST, PRSTOPT3 },
		{ UMP_PRST, PRSTLASTLINE },
		{ (int )NULL, (int )NULL}
};

	asl_ret = DIAG_ASL_OK;

	/****************************************************************
	* If the user presses CANCEL or EXIT, then stop.  Else, select  *
	* desired item.
	****************************************************************/

	while( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )  {

		asl_ret = diag_display(UMPPRST_MENU, fdes, menu2list, 
			DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
			NULL, NULL);
		if( asl_ret == DIAG_ASL_COMMIT ) {
			diag_asl_clear_screen();
			switch ( selection = DIAG_ITEM_SELECTED(dm_menutype)) {
				case display_sel :
					display_processor();
					break;
				case disable_sel :
				case enable_sel :
					alter_processor(selection);
					break;
			}
		}
	}
}

/*
 * NAME: display_processor
 *                                                                    
 * FUNCTION: Display the processors state (enabled or disabled)
 *                                                                    
 * NOTES: This function invokes the command 'cpu_state' to display the
 *	  enabled or disabled processors (i.e the processors 
 *        which will be active after the next bootstrap).
 *
 * RETURNS: NONE
 *
 */  

void display_processor(void)
{
	int 		rc;
	ASL_RC		asl_ret;	/* ASL return code */
	ASL_SCR_INFO	*menuinfo;
	static ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;
	char		*str; 

	outbuf = NULL;
	errbuf = NULL;

	rc = odm_run_method(CPUSTATE, "-l", &outbuf, &errbuf );

	/* Return error if command failed */
	if (rc != 0) 
	{
		str = diag_cat_gets(fdes, UMP_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(str, "unknown reason");
		}
	}
	else
	{
		/* allocate space for 3 entries */
		menuinfo = (ASL_SCR_INFO *) calloc( 3, sizeof(ASL_SCR_INFO) );
		if (menuinfo == NULL)
		{
        		diag_asl_msg(diag_cat_gets(fdes, UMP_ERR, MEM_ERR));
			genexit(202);
		}

		/* Add title to menu */
		menuinfo[0].text = diag_cat_gets(fdes,UMP_DISPLAY, DSPTITLE);
	
		/* put in the list of devices */
		menuinfo[1].text = outbuf;

		/* remove the last newline */
		str=(char *)strrchr(menuinfo[1].text, '\n');
		if (str!=NULL)
		{
			*str = '\0';
		}

		/* add in the last line */
		menuinfo[2].text = diag_cat_gets( fdes, UMP_DISPLAY, DSPLASTLINE);

		menutype.max_index = 2;
		asl_ret = diag_display( UMPDSP_MENU, fdes, NULL, DIAG_IO,
			   	ASL_DIAG_LIST_CANCEL_EXIT_SC,
			   	&menutype, menuinfo);
		FREE(menuinfo);
	}

	FREE(outbuf);
	FREE(errbuf);
}

/*
 * NAME: alter_processor  
 *
 * FUNCTION: disable or enable a processor for the next bootstrap.
 * 
 * NOTES: This unit performs the following functions:
 *	1. Prompt the user for processor ID
 *      2. Invoke cpu_state -d or -e for the selected processor.
 *
 * RETURNS: NONE
 *
 */

void alter_processor(int selection)
{
	DIAG_ASL_RC	asl_ret;
	int		rc = -1;
	int		processor;
	char            cmd_args[128];
	char		*str; 

	/* prompt for processor number */
	asl_ret = get_processor(&processor, selection);

	/* issue the command to disable or enable the processor */
	if (asl_ret == DIAG_ASL_COMMIT)
	{
		if (selection == disable_sel)
		{
			sprintf(cmd_args,"-d %d", processor);
		}
		else
		{
			sprintf(cmd_args,"-e %d", processor);
		}
		(void)run_command(CPUSTATE, cmd_args);
	}
}


/*
 * NAME: get_processor
 *
 * FUNCTION: display dialog screen and prompt for processor number.
 *
 * RETURNS: 
 *          DIAG_ASL_CANCEL
 *          DIAG_ASL_EXIT
 *          DIAG_ASL_COMMIT
 */

int get_processor(int *processor_id, int selection)
{
	DIAG_ASL_RC	asl_ret;
	char		entered_proc[2];
	ASL_SCR_INFO    *dialoginfo;
	static ASL_SCR_TYPE dialogtype = DM_TYPE_DEFAULTS;
	int		menu_num;
	int		rc;
	char 		*outbuf;
	char 		*errbuf;
	char 		*str1;
	char 		*str2;
	
	outbuf = NULL;
	errbuf = NULL;

	rc = odm_run_method(CPUSTATE, "-l", &outbuf, &errbuf );

	/* Return error if command failed */
	if ((rc != 0) || (outbuf == NULL))
	{
		str1 = diag_cat_gets(fdes, UMP_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str1, errbuf);
		}
		else
		{
			diag_asl_msg(str1, "unknown reason");
		}
		asl_ret = DIAG_ASL_FAIL;
	}
	else
	{
		/* allocate space for 3 entries */
		dialoginfo = (ASL_SCR_INFO *) calloc( 3,
						sizeof(ASL_SCR_INFO) );
		if (dialoginfo == NULL)
		{
			diag_asl_msg(diag_cat_gets(fdes, UMP_ERR, MEM_ERR));
			genexit(202);
		}

		/* Add title to dialog */
		if (selection == disable_sel)
		{
			dialoginfo[0].text =
				diag_cat_gets(fdes ,UMP_ALTER, DISABLETITLE);
			menu_num=PROC_DISABLE;
		}
		else 
		{
			dialoginfo[0].text =
				diag_cat_gets(fdes ,UMP_ALTER, ENABLETITLE);
			menu_num=PROC_ENABLE;
		}

		/* dynamic list of available processors */
		/* limited to one figure per processor */ 
		strtok(outbuf, "\n");
		str2 = outbuf;
		while ((str1 = strtok(NULL, "\n")) != NULL)
		{
			char c;

			if (*str1 == '\t')	str1++;
			/* skip proc characters (4 char) */
			str1=str1+PROCLEN;
			c = *str1;
			if (c >= '0' && c <='9')
			{
				*str2 = c;
				str2++;
				*str2 = ',';
				str2++;
			}
		}
		if (str2 != outbuf)	str2--;
		*str2 = '\0';

		/* processor prompt */
		dialoginfo[1].text = diag_cat_gets(fdes, UMP_ALTER, ALTERPROMPT);
		entered_proc[0] = '0';
		entered_proc[1] = '\0';
		dialoginfo[1].op_type = ASL_RING_ENTRY;
		dialoginfo[1].entry_type = ASL_NUM_ENTRY;
		dialoginfo[1].entry_size = 1;
		dialoginfo[1].required = ASL_YES;
		dialoginfo[1].disp_values = outbuf;
		dialoginfo[1].data_value = entered_proc;

		/* put instruction line */
		dialoginfo[2].text = diag_cat_gets(fdes ,UMP_ALTER,ALTERLASTLINE);

		dialogtype.max_index = 2;

		while ( asl_ret != DIAG_ASL_CANCEL && asl_ret != DIAG_ASL_EXIT )
		{
			asl_ret = diag_display( menu_num, fdes, NULL, DIAG_IO,
	   			ASL_DIAG_DIALOGUE_SC, &dialogtype, dialoginfo);

			if ( asl_ret == DIAG_ASL_COMMIT )
			{
				*processor_id = atoi(dialoginfo[1].data_value);
				break;
			}
		}	
		FREE(dialoginfo);
	}
	FREE(outbuf);
	FREE(errbuf);
	return (asl_ret);
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
		str = diag_cat_gets(fdes, UMP_ERR, CMDERR);
		if (*errbuf)
		{
			diag_asl_msg(str, errbuf);
		}
		else
		{
			diag_asl_msg(str, "unknown reason");
		}
	}
	else {
		/* command successful */
		if (*outbuf) {
			str = diag_cat_gets(fdes, UMP_ERR, CMDOK);
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

