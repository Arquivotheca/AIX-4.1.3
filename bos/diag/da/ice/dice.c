static char sccsid[] = "@(#)48	1.1  src/bos/diag/da/ice/dice.c, daice, bos41J, 9513A_all 3/22/95 12:18:13";
/*
 *   COMPONENT_NAME: DAICE
 *
 *   FUNCTIONS: all_init
 *		check_asl_stat
 *		clean_up
 *		disp_scrn
 *		ela
 *		insert_color
 *		insert_info
 *		intr_alrm
 *		intr_handler
 *		main
 *		report_frub
 *		rgb
 *		set_up_sig
 *		test_tu
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <errno.h>
#include <sys/signal.h> 
#include <sys/errids.h> 
#include <locale.h> 
#include<fcntl.h>
#include<sys/cfgodm.h>

#include "diag/tm_input.h"
#include "diag/tmdefs.h" 
#include "diag/da.h" 
#include "diag/diag.h"
#include "diag/diag_exit.h" 
#include "diag/diago.h" 
#include "dice.h" 

/* Declarations of functions in the C source */
void main();
static char * insert_info(char * ,char * );
static char * insert_color(char * ,int);
static void rgb(void);
static int disp_scrn(int);
static void set_up_sig(void);
static void ela(void);
static void intr_handler(int);	
static void intr_alrm(int);	
static void all_init(void);
static void check_asl_stat(int);
static void clean_up(void);
static int test_tu(int *);
static void report_frub(struct fru_bucket *);
static void display_pd(void);

extern 	nl_catd diag_catopen();
extern 	getdainput ();
extern 	int exectu();		/* test unit interface 			*/

struct 	tm_input da_input;	/* initialized by getdainput 		*/
static nl_catd catd;			/* pointer to message catalog  		*/
static short	odm_flag=0;		/* return code from initialize odm	*/
static int asl_flag = ASL_NOT_INITIALIZED;
static int initialized = NO;
static char * name = NULL;  /* Name of card gotten from database */
char size = 1; /* default to 1 for 1M */
int disp_attached=NO;
/*
 * NAME: main
 *
 * FUNCTION: 
 *
 * EXECUTION ENVIRONMENT: Invoked by the diagnostic controller.
 *
 * RETURNS:		DA_STATUS_GOOD
 * 					No problems were found.
 *				DA_STATUS_BAD
 *					A FRU Bucket or a Menu Goal was
 *					reported.
 *				DA_USER_NOKEY
 *					No special function keys were entered.
 *				DA_USER_EXIT
 *					The Exit Key was entered by the user.
 *				DA_USER_QUIT
 *					The Quit Key was entered by the user.
 *				DA_ERROR_NONE
 *					No errors were encountered performing a
 *					normal operation such as displaying a
 *					menu, accessing the object repository,
 *					allocating memory, etc.
 *				DA_ERROR_OPEN
 *					Could not open the device.
 *				DA_ERROR_OTHER 
 *					An error was encountered performing
 *					a normal operation.
 *				DA_TEST_NOTEST
 *					No tests were executed.
 *				DA_TEST_FULL
 *					The full-tests were executed.
 *				DA_TEST_SUB
 *					The sub-tests were executed.
 *				DA_TEST_SHR
 *					The shared tests were executed.
 *				DA_MORE_NOCONT
 *					The isolation process is complete.
 *				DA_MORE_CONT
 *					The path to the device should be
 *					tested.
 */
void main()
{
	int rc;		/* return code */
	rc = 0;  /* set return code to 0 */

	setlocale(LC_ALL,""); 
	all_init(); 

	/* determine what type of card we are running.  This is according to
 	 * the DB */
	/* get the name of the card from the catalog file */
	if(name == NULL)
		name = (char *) get_dev_desc(da_input.dname);

	if(CONSOLE_INSTALLED)
	{
			if(NOTLM || ENTERLM)
			{
				disp_scrn(TITLE);
			}
			else
				disp_scrn(LOOPMODE_MSG_ID);
		
			sleep(2);

	}
	
	/* if running in LOOPMODE_EXITLM then 		*/
	/* clean up and exit						*/
	if( EXITLM )
	{
		clean_up();
	}

	if( PD_MODE || REPAIR_MODE )
	{

		check_asl_stat((int)READ_KBD); 


		/* set up signal handlers */
		set_up_sig(); 

		rc = test_tu(TU_OPEN_DEX);
		if(rc == 0)
			initialized = YES; 
		else 	
			clean_up(); 

		if(IPL)
			test_tu(TU_ORD_IPL);
		else
		{
			test_tu(TU_ORD_ADV);

			initialized = NO; 

			rc = test_tu(TU_CLOSE_DEX);
			if(rc != 0)
				clean_up(); 

                  /* if any of these modes 
                   * then run rgb
                   */
			if(NOTLM && (! SYSTEM) && CONSOLE_INSTALLED && 
				(DA_CHECKRC_STATUS() == DA_STATUS_GOOD) )
			{
				rgb();
			} 
		}
				
	}
	 
	/* cleaning up and exit DA	*/
	clean_up ();
	
	return;
} /* main */



/*
 * NAME: clean_up
 *
 * FUNCTION: close file descripters and clean up ASL, ODM, etc. and then exit
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void clean_up(void)
{
	char goal[512];

	if(initialized)
	{
		initialized = NO;
		test_tu(TU_CLOSE_DEX);
	}


	/* terminate odm */
	if(odm_flag != ERROR_FOUND) 
		term_dgodm();

	if(catd != CATD_ERR)
			catclose (catd);

	if ( asl_flag == ASL_INITIALIZED)
	{
		diag_asl_clear_screen();
		diag_asl_quit(NULL);
	}


	DA_EXIT ();
} /* clean_up */


/*
 * NAME: ela
 *
 * FUNCTION: perform error log analysis and take action if needed
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void ela(void)
{
	int ela_rc , ela_get_rc;
	char crit[128];
	struct errdata errdata_str;
	int index;

	/* initialize index */
	index = 0;
	
	while(errids[index] !=  (int) NULL)
	{
		/*..........................................................*/
		/* Look for an occurrence of any of the following errors in */
		/* the error log.  If one exists, exit_with_fru for ELA.    */
		/*..........................................................*/
		sprintf(crit,"%s -N %s -j %X", da_input.date, da_input.dname,
				errids[index]); 

		ela_get_rc = error_log_get(INIT,crit,&errdata_str);
		ela_rc = error_log_get(TERMI,crit,&errdata_str);

		if (ela_get_rc >0)
		{
			/* if no FRU with that criteria get generic HW FRU callout */
       	 	report_frub(&frus[5]);
        		clean_up();
		}

		index++;
	}

	return;
} /* ela */



/*
 * NAME: report_frub
 *
 * FUNCTION: Adds FRU bucket to database
 *
 * EXECUTION ENVIRONMENT:
 *
 * INPUTS:  The address of the FRU structure to add to the FRU DB.
 *
 * RETURNS: NONE
 */

static void report_frub(struct fru_bucket *frub_addr)
{
	int	rc;

	/* copy device name into the field device name of fru bucket */
	strcpy ( frub_addr->dname, da_input.dname);

	/* update FRU Bucket with parent device name 		*/
	rc = insert_frub(&da_input, frub_addr);
	if (rc != NO_ERROR)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		return ;
	}

	/* add a FRU bucket; 					*/
	rc = addfrub(frub_addr);
	if (rc != NO_ERROR)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		return;
	}

   	DA_SETRC_STATUS (DA_STATUS_BAD);
	return;
} /* report_frub */


/*
 * NAME: check_asl_stat
 *
 * FUNCTION: checks ASL status and/or return code
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void check_asl_stat(int rc)
{
	char buf[64];

	if( (! CONSOLE_INSTALLED ) )
	{
		return;
	}

	if(rc == READ_KBD)
		rc = diag_asl_read ( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,buf) ;

	switch ( rc)
	{
		case ASL_CANCEL:
			DA_SETRC_USER (DA_USER_QUIT);
			clean_up();
			break;

		case ASL_EXIT:
			DA_SETRC_USER (DA_USER_EXIT);
			clean_up();
			break;
		default:
			break;
		
	}
	return;
}  /* check_asl_stat 	*/


/*
 * NAME: test_tu
 *
 * FUNCTION: Run the specified test unit
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static int test_tu(int * numb)
{
	long int rc;
	TU_TYPE tucb;
	int index;
	ERROR_DETAILS data;

	/* initialize rc and index to 0 */
	rc = 0;
	index = 0;


	while((numb[index] != (int) NULL) && (rc == NO_ERROR))
	{
		/* initialize tucb structure */
		tucb.loop = 1;
		tucb.error_log_count = 1;
		tucb.error_log = &data;
		tucb.tu = numb[index];

		alarm(180);
		rc = exectu (da_input.dname, &tucb);     
		alarm(0);

		/* check for hardware error */
		if(rc != NO_ERROR)
		{
			report_frub (&frus[0]);

		}

 		check_asl_stat ((int)READ_KBD);
		index++;
	}


    return(rc);
} /* test_tu */

/*
 * NAME: all_init
 *
 * FUNCTION: Initialize things like ODM, ASL, and sets default exit values.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
static void all_init(void)
{

	int rc;

	/*..............................*/
	/* General Initialization	*/
	/*..............................*/

	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);
	DA_SETRC_USER (DA_USER_NOKEY);

	/* initialize odm and get dainput	*/
	odm_flag = init_dgodm();
	if (odm_flag == ERROR_FOUND || getdainput (&da_input) != 0)
	{
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		clean_up ();
	}


	/* running under console */
	if ( CONSOLE_INSTALLED )
	{
		/* initialize asl 		*/
		if(INLM)
			rc = diag_asl_init ( "DEFAULT" );
		else 
			rc = diag_asl_init("NO_TYPE_AHEAD");

		if(rc == ERROR_FOUND)
		{
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up ();
		}
		else
			asl_flag = ASL_INITIALIZED;
			
		catd = diag_catopen (CATALOG,0);  

	}

} /* all_init */


/*
 * NAME: set_up_sig
 *
 * FUNCTION: Set up signal handler calls
 *
 * RETURNS: NONE
 */
void set_up_sig(void)
{
	struct  sigaction   invec;  /* interrupt handler structure     */

	/* initializing interrupt handler */    
	invec.sa_handler =  intr_handler;
	sigaction( SIGINT, &invec, (struct sigaction *) NULL );
	sigaction( SIGTERM, &invec, (struct sigaction *) NULL );
	sigaction( SIGQUIT, &invec, (struct sigaction *) NULL );
	sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );
	invec.sa_handler =  intr_alrm;
	sigaction( SIGALRM, &invec, (struct sigaction *) NULL ); 

	return;
}   /* end of set_up_sig() */


/*
 * NAME: disp_scrn
 *
 * FUNCTION: displays menus
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  the return code is the choice which was made, either 1 or 0.
 * 			 This is only valid for the screens that ask a question but the 
 *			 DIAG_ITEM_SELECTED(menutypes) is returned regardless and it should
 * 			 be ignored by the calling function if it is not needed.
 *
 */

#define SUBS 4

static int disp_scrn(int menu_type)
{
	ASL_SCR_INFO uinfo[10];
	ASL_SCR_TYPE menutypes= DM_TYPE_DEFAULTS;
	int rc,i;
	char msgstr[512];
	char mode_str[64];
	int base_num;
	int new_num = 3;
	int menu_num;
	char  * subs[SUBS];
	static first = TRUE;
	int color;

	if( asl_flag == ASL_NOT_INITIALIZED)
	{
		return(0);
	}

	for(i=0;i<SUBS;i++)
	{
		subs[i] = (char *) malloc(128);
	}

	/* put name and location info into substitution variables */
	strcpy(subs[0],name);	
	strcpy(subs[1],da_input.dname);
	strcpy(subs[2],da_input.dnameloc); 

	base_num = FFC_S3 * 0x1000;
	memset(msgstr,0,sizeof(msgstr));
	memset(uinfo,0,sizeof(uinfo));

	if(ADVANCED)
	{
		strcpy(mode_str,diag_cat_gets(catd,MSG_SET,ADVANCED_MSG_ID));	
		menu_num = base_num + 1;
	}
	else
	{
		strcpy(mode_str,diag_cat_gets(catd,MSG_SET,CUSTOMER_MSG_ID));	
		menu_num = base_num + 2;
	}

	switch(menu_type) 
	{
		case TITLE:
			
			/* display title/standby menu */
			if(ADVANCED)
				rc = diag_display_menu(ADVANCED_TESTING_MENU,menu_num, 
								subs,da_input.lcount,da_input.lerrors);
			else
				rc = diag_display_menu(CUSTOMER_TESTING_MENU,menu_num,
								subs,da_input.lcount,da_input.lerrors);

			check_asl_stat(rc);
			break;

		case LOOPMODE_MSG_ID:
			menu_num = base_num + 3;


			rc = diag_display_menu(LOOPMODE_TESTING_MENU,menu_num, 
								subs,da_input.lcount,da_input.lerrors);
			check_asl_stat(rc);
			break;

		case RED:
			new_num --;
		case GREEN:
			new_num --;
		case BLUE:
			new_num --;
			/* if this is the first time, put up warning screen */
			if(first == TRUE)
			{
				/* toggle first variable to false so the next
				   time the user is asked if the correct pattern
				   was displayed. */
				first = FALSE;

      	             menu_num = base_num + 9;
     	             	/* display msg. about colors */
     	             	diag_display(0x0,catd, f3_exit,DIAG_MSGONLY,
     	                            ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo);
				/* insert name, location, and color */
				uinfo[0].text = insert_info(uinfo[0].text,mode_str);
				uinfo[1].text = insert_color(uinfo[1].text,menu_type);

				/* display message */
     	             	rc = diag_display(menu_num, catd, NULL, DIAG_IO,
     	                      ASL_DIAG_KEYS_ENTER_SC, &menutypes, uinfo);
     	             	check_asl_stat(rc);

			}
			else
			{
				/* toggle first variable to true */
				first = TRUE;
				/* redscreen is base_num + 6 */
				menu_num = base_num + new_num + 6;
				menutypes.cur_index = 1;

				diag_display(0x0,catd, color_screen,DIAG_MSGONLY,
							ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo);

				/* 
				 * insert mode,e.g. Advanced, and color that you are looking for.
				 */
				uinfo[0].text = insert_info(uinfo[0].text,mode_str);
				uinfo[3].text = insert_color(uinfo[3].text,menu_type);

				rc = diag_display(menu_num, catd, NULL, DIAG_IO,
					   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);

				free(uinfo[0].text);
				free(uinfo[3].text);
			}

			/* 
			 * if you get a return or cancel, return a 3 so that rgb()
			 * knows to clean_up
			 */
			if(rc == ASL_CANCEL || rc == ASL_EXIT)
				return(3);

			break;

		case HAVEDISP:

			menu_num = base_num + 5;
			menutypes.cur_index = 1;

			/* display msg. asking if display is attached */
			diag_display(0x0,catd, havedisp,DIAG_MSGONLY,
							ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,uinfo);
			/* 
			 * insert mode,e.g. Advanced
			 */
			uinfo[0].text = insert_info(uinfo[0].text,mode_str);

			rc = diag_display(menu_num, catd, NULL, DIAG_IO,
				   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);


			free(uinfo[0].text);
			check_asl_stat(rc);

			break;
		case DISP_PD:

			menu_num = base_num + 5;

			rc = diag_display(menu_num, catd, builtin, DIAG_MSGONLY,
				   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);
			/* 
			 * insert mode,e.g. Advanced
			 */
			uinfo[0].text = insert_info(uinfo[0].text,mode_str);

			menutypes.cur_index = 2;
			rc = diag_display(menu_num, catd, NULL, DIAG_IO,
				   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);

			check_asl_stat(rc);
			free(uinfo[0].text);

			break;

		default:
			break;
	} 
	check_asl_stat((int)READ_KBD);


	/* free memory */
	for(i=0;i<SUBS;i++)
		free(subs[i]);
	/* 
	 * if item chosen, return which item was selected.
	 */
	return(DIAG_ITEM_SELECTED(menutypes));
} /* disp_scrn */

/*
 * NAME: insert_info
 *
 * FUNCTION:  Inserts data into text message for diag_display_menu to use.
 *
 * INPUTS:
 			char * text - This argument is a pointer to text that is getting
						  substituted into.

			char * mode - This is either ADVANCED or empty.
 *
 * RETURNS:  none
 */
char * insert_info(char * text,char * mode)
{
	char * str;
	str = (char *)malloc(512);

	sprintf(str,text,name,da_input.dname,da_input.dnameloc,mode,"");
	return(str);
} /* end of insert_info() */

/*
 * NAME: insert_color
 *
 * FUNCTION: Gets red,green, or blue message to stick into message asking user
			 if they see this color.
 *
 * INPUT:
		char * text - text of the message that you need to have another
					  message inserted into.

		int  color  - The message number of the message to insert into text. 
 *
 * RETURNS:  none
 */
char * insert_color(char * text,int color)
{
	char * str;
	str = (char *)malloc(512);
	sprintf(str,text,diag_cat_gets(catd,MSG_SET,color));
	return(str);
} /* insert_color */

/*
 * NAME: rgb
 *
 * FUNCTION: displays rgb screens
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  none
 */
static void rgb(void)
{
	int rc;
	int i;
	int * tu_color[] = {RED_TU,GREEN_TU,BLUE_TU};

	switch(disp_scrn(HAVEDISP)) 
	{
		case 1 : 
			break;
		case 2 : 
			if(disp_attached)
				display_pd();
		default: 
			return;
	}


	/* Run RGB since display is attached */
	/* ASL should handle all of the keyboard input */
    	for (i=0; i<3; ++i) 
	{
		/* First time disp_scrn is called with REDSCRN, the
		   warning screen is put up. */
		switch (i) 
		{
			case 0 : /* RED */
				disp_scrn(RED);
	 			break;
			case 1 : /* GREEN */
				disp_scrn(GREEN);
				break;
			case 2 : /* BLUE */
				disp_scrn(BLUE);
				break;
			default: 
				break;
		}

		/* initialize TU's */
		rc = test_tu(TU_OPEN_DEX);
		if(rc == 0)
			initialized = YES; 
		else 	
			clean_up(); 

		test_tu(tu_color[i]);

		disp_scrn(TITLE);
		sleep(7);
		
		/* run close TU */
		rc = test_tu(TU_CLOSE_DEX);
		initialized = NO; 


		/* Put up screen asking if the screen was the right color */ 
		switch (i) 
		{
			case 0 : /* RED */
				rc = disp_scrn(RED);
	 			break;
			case 1 : /* GREEN */
				rc = disp_scrn(GREEN);
				break;
			case 2 : /* BLUE */
				rc = disp_scrn(BLUE);
				break;
			default: 
				break;
		}

		/* check answer either yes(1), or no(2) */
		switch(rc) 
		{
			case 1 : 
				break;
			case 2 : 
				/* report fru */
				report_frub (&frus[1]); 
				/* let this fall thru exit */
			default: 
				clean_up();
				break;

		}
	}

	display_pd();
	return;
} /* end rgb() */


void display_pd(void)
{
	int rc;
	/* inform the user to run built in display tests */
	rc = disp_scrn(DISP_PD);
	switch (rc) 
	{
		case 1 : /* report fru */
			report_frub (&frus[2]); 
			clean_up();
			break;
		case 2 :
		default: 
			return;
	}
}

/*
 * NAME: intr_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
void intr_handler(int sig)
{
    clean_up();
} /* intr_handler */

/*
 * NAME: intr_alrm
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 */
void intr_alrm(int sig)
{
	
	alarm(0);
	report_frub (&frus[0]);

	clean_up();
} /* intr_alrm */
