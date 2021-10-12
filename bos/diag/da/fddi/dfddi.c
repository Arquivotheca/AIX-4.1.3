static char sccsid[] = "@(#)18	1.7.1.13  src/bos/diag/da/fddi/dfddi.c, dafddi, bos41J, 9523B_all 6/5/95 17:08:44";
/*
 *   COMPONENT_NAME: DAFDDI
 *
 *   FUNCTIONS: card_determination
 *		check_asl_stat
 *		clean_up
 *		display_menu
 *		error_log_analysis
 *		extender_card_exist
 *		extender_card_in_database
 *		extender_card_wrap_test
 *		fddi_davar_op
 *		fddi_display
 *		foxcroft_unplug_wrap_plug_all
 *		foxcroft_unplug_wrap_plug_main_card
 *		interactive_test
 *		intr_handler
 *		main
 *		non_interactive_tests_A
 *		non_interactive_tests_B
 *		open_device
 *		performing_test
 *		report_frub
 *		restore_normal_adapter
 *		test_tu
 *		timeout
 *		unplug_wrap_plug_all
 *		unplug_wrap_plug_main_card
 *		verify_rc
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <sys/signal.h> 
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include "fddi_errids.h"
#include <diag/tm_input.h>
#include <asl.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_exit.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <diag/bit_def.h>
#include "dfddi_msg.h"
#include "dfddi.h"
#include "fdditst.h"
#include <locale.h>

TUTYPE 	tucb;


extern  int	diag_exec_source();
extern	invoke_method();
extern 	int	exectu();
extern 	nl_catd diag_catopen();
extern 	getdainput ();
extern 	disp_menu();		/* displays menus 			*/
extern 	user_quit();		/* cleanup routine 			*/
extern 	int errno;		/* reason device driver won't open 	*/
void 	intr_handler(int);	/* da interrupt handler 		*/
struct 	tm_input da_input;	/* initialized by getdainput 		*/
long 	da_mode;		/* diagnostic modes 			*/
int 	filedes=ERROR_FOUND;	/* file descriptor from device driver 	*/
int 	mdd_fdes=ERROR_FOUND;	/* machine device drivers file desciptor*/
nl_catd catd;			/* pointer to message catalog  		*/
char	dname[NAMESIZE]; 	/* device name to be tested 		*/
short	wrap_plug;	
short	network_cable_main_card;
short	network_cable_extender_card;
short	extender_card;
char	extender_loc[LOCSIZE];	/* location of extender card	*/
char	extender_card_name[]="fddix0";
short	odm_flag=0;		/* return code from initialize odm	*/
short	card_type;		
int    	alarm_timeout=FALSE;

int	prior_state= NOT_CONFIG;
char	option[256];
char	*new_out, *new_err;
int	diag_device_configured=FALSE;
int	diag_device_diagnose=FALSE;


void error_log_analysis ();
void display_menu();
void fddi_display();
void fddi_davar_op ();
void restore_normal_adapter ();
void unplug_wrap_plug_main_card ();
void foxcroft_unplug_wrap_plug_main_card ();
void unplug_wrap_plug_extender_card ();
void unplug_wrap_plug_all ();
void foxcroft_unplug_wrap_plug_all ();
void card_determination();
void timeout(int);



ASL_SCR_TYPE	menutypes=DM_TYPE_DEFAULTS;
char msgstr[1024];
char msgstr1[1024];


/*......................................................................*/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the  Fiber Distributed Data Interface
 *           Diagnostic Application. 
 *
 * EXECUTION ENVIRONMENT: Invoked by the diagnostic controller.
 *
 * RETURNS:			DA_STATUS_GOOD
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
 *
 ..........................................................................*/

main ()
{
 	struct  sigaction  invec;          /* interrupt handler structure  */	
	int	rc=0;			/* return code 			*/
	int	diskette_mode;
	int	diskette_based;
	struct	stat stat_buf;		/* stat buffer defined in stat.h */
	int	asl_rc;
	char	microcode_path[128];
	int	diag_source;
	char	mountcdrfs[128];
	char    odm_search_crit[40];


	/* initializing interrupt handler */    
	(void)signal (SIGINT, intr_handler);
	/* initializing alarm timeout	*/
        invec.sa_handler = timeout;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );

	setlocale (LC_ALL, "");

	/*..............................*/
	/* General Initialization	*/
	/*..............................*/

	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);
	DA_SETRC_USER (DA_USER_NOKEY);



	odm_flag = init_dgodm();
	if (odm_flag == ERROR_FOUND || getdainput (&da_input) != 0)
	{
		DA_SETRC_STATUS(DA_ERROR_OTHER);
		clean_up ();
	}

        strcpy (extender_loc, da_input.dnameloc);
        extender_loc[4] -= 1;


	if 	/* running under console */
	   ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl 		*/
		if (diag_asl_init ( "NO_TYPE_AHEAD") == ERROR_FOUND) 
		{
			DA_SETRC_STATUS(DA_ERROR_OTHER);
			clean_up ();
		}
		card_determination();
		catd = diag_catopen (MF_DFDDI,0); 
		fddi_display ();
 		check_asl_stat ();
	}


	/* if running in LOOPMODE_EXITLM then 		*/
	/* check if the cable is hooked before running  */
	/* DA. If yes, then ask the user to put the   	*/
	/* cable back into the adapter			*/

	if ( da_input.loopmode == LOOPMODE_EXITLM)
	{

		/* get davar from the database	*/
		fddi_davar_op (FDDI_GET_OP);

		/* if the cable in main card is hooked up before */
		/* running the DA */
		if (((network_cable_main_card) && (network_cable_extender_card))
			&& ( da_input.console == CONSOLE_TRUE))
		{
			if (card_type == FOXCROFT)
		 		foxcroft_unplug_wrap_plug_all ();
			else
		 		unplug_wrap_plug_all ();
			fddi_display();
		}	
		/* if the cable in extender card is hooked up before */
		/* running the DA */
		else if ((network_cable_main_card) 
			&& ( da_input.console == CONSOLE_TRUE))
		{
			if (card_type == FOXCROFT)
		 		foxcroft_unplug_wrap_plug_main_card ();
			else
		 		unplug_wrap_plug_main_card ();
			fddi_display();
		}	

		clean_up ();
	}	/* LOOPMODE_EXITLM	*/

	/* executing test if running problem determination	*/
	/* or repair mode					*/
	/* or running regression				*/

	else if ((da_input.exenv == EXENV_REGR) || 
		(( da_input.dmode == DMODE_PD) || 
		 ( da_input.dmode == DMODE_REPAIR)))
	{
	
		/* check if diskette package is running */
  		diskette_mode = ipl_mode(&diskette_based);
		diag_source = diag_exec_source(&mountcdrfs);
               	if ( (diskette_based == DIAG_FALSE) &&
		     (diag_source == 0) )
		{
			/* checking operational microcode	*/ 
			/* checking diagnostics microcode */
			rc = stat ("/usr/lib/microcode/fddi.diag", &stat_buf);
			if (( rc == ERROR_FOUND) || ( stat_buf.st_size == 0 ))
			{


				if  ((da_input.console == CONSOLE_TRUE) &&
				    ((da_input.loopmode == LOOPMODE_ENTERLM)
				   || (da_input.loopmode == LOOPMODE_NOTLM)))
				{
				      sprintf(msgstr,(char *) 
					diag_cat_gets (catd, 1,
				        FDDI_DIAG_MICROCODE_FILE_BAD, 
					NULL), NULL);
				      menugoal (msgstr);
				}
				DA_SETRC_STATUS (DA_STATUS_BAD);
				clean_up();
			}

			findmcode ("8ef4m", &microcode_path, 0, "");
			rc = stat (&microcode_path[0], &stat_buf);
			if (( rc == ERROR_FOUND) || ( stat_buf.st_size == 0 ))
			{

				if  ((da_input.console == CONSOLE_TRUE) &&
				    ((da_input.loopmode == LOOPMODE_ENTERLM)
				   || (da_input.loopmode == LOOPMODE_NOTLM)))
				{
					sprintf (msgstr, (char *)
						diag_cat_gets (catd, 
						1, FDDI_OP_MICROCODE_FILE_BAD, 
						NULL), NULL);
					menugoal (msgstr);
				}
				DA_SETRC_STATUS (DA_STATUS_BAD);
				clean_up();
			}
		}

 		prior_state=get_device_status(da_input.dname);
		rc = unconfigure_lpp_device();
	
		if (prior_state != -1)
		{
			/* Perform testing and/or handle wrap plugs.
		 	*  rc = 0 means good path.
			*/
			rc = open_device ();
			check_asl_stat ();
			if ( rc == NO_ERROR) 
			{
				rc = performing_test ();
			}
 			check_asl_stat ();
		}
	}
	 
	/* if running problem determination, or error log analyis 	*/
	/* running error log analysis					*/

	if  ((rc != ERROR_FOUND) &&(( da_input.dmode ==  DMODE_PD) 
		|| (da_input.dmode == DMODE_ELA ))
		&& (( da_input.loopmode == LOOPMODE_NOTLM ) 
		||  ( da_input.loopmode == LOOPMODE_ENTERLM ))
		&& (da_input.exenv != EXENV_REGR)) 
	{

		error_log_analysis ();
	}

	/* cleaning up and exit DA	*/

	clean_up ();
	
} /* main */

/*......................................................*/
/*      Procedure description : This procedure will 	*/
/*				call test units to test */
/*				the fddi adapter	*/
/*......................................................*/
int performing_test ()
{
	int	rc =NO_ERROR;

	tucb.header.loop =1;
	tucb.header.mfg =0;
	tucb.mdd_fd = mdd_fdes;
	tucb.slot = (int) da_input.dnameloc[4];
	tucb.bus_no = (int) da_input.dnameloc[3];


	if (da_input.loopmode & ( LOOPMODE_ENTERLM | LOOPMODE_NOTLM ) ) 
	{
                fddi_davar_op (FDDI_INIT_OP); /* initialize davars 	*/
	}
	else	
		fddi_davar_op (FDDI_GET_OP);  /* read davars		*/

	rc =  non_interactive_tests_A (group_tests_A);
	check_asl_stat ();

	/* system true means non-interactive test 	*/
	if (( rc == NO_ERROR) && ( da_input.advanced == ADVANCED_TRUE) 
			&& ( da_input.console == CONSOLE_TRUE)
			&& ( da_input.system == SYSTEM_FALSE))
	{ rc = interactive_test ();
		check_asl_stat ();
	}
	if ( rc != NO_ERROR)
	{
		restore_normal_adapter();
		check_asl_stat ();
	}
	else
	{
		rc = non_interactive_tests_B (group_tests_B);
		check_asl_stat ();
		if ( rc != NO_ERROR)
			restore_normal_adapter ();
		check_asl_stat ();
	}
	if ((da_input.loopmode == LOOPMODE_NOTLM) && 
		(da_input.console == CONSOLE_TRUE))
	{
		if ((network_cable_main_card) && (network_cable_extender_card))
		{
			/* ask the user to unplug the wrap plug and 	*/
			/* put the network cables back into main adapter*/ 
			/* and extender adapter				*/

			if (card_type == FOXCROFT)
		 		foxcroft_unplug_wrap_plug_all ();
		 	else
				unplug_wrap_plug_all ();
			fddi_display();
		}
		else if (network_cable_main_card)
		{
			/* ask the user to unplug the wrap plug and 	*/
			/* put the network cables back into main   	*/ 
			/* adapter					*/

			if (card_type == FOXCROFT)
		 		foxcroft_unplug_wrap_plug_main_card();
			else
				unplug_wrap_plug_main_card();
			fddi_display();
		}
		check_asl_stat ();
	}
	return (rc);
}
/*......................................................*/
/*      Procedure interactive_test:  This procedure     */
/*                                   will call the port */
/*                                   wrap test          */
/*      Requisite                    The wrap plug      */
/*                                   needs to be plugged*/
/*                                   the tested port    */
/*......................................................*/

int interactive_test ()
{
        int     rc=NO_ERROR;
        ASL_SCR_TYPE    menutypes;


        /* if there is no wrap plug, do not run this test       */
        /* Default value is TRUE                                */

        if (!wrap_plug )
                return (NO_ERROR);

        /* have to run test 1 and 21 before running test 17     */
        rc = test_tu (1, FRU_101);
        if (rc != NO_ERROR) return (ERROR_FOUND);
        rc = test_tu (21, FRU_121);
        if (rc != NO_ERROR) return (ERROR_FOUND);

        if ( da_input.loopmode == LOOPMODE_INLM)
        {
                /* test unit 17 : data wrap test for main adapter */
		alarm_timeout = FALSE;
		alarm (300);
                tucb.header.tu = 17;
                rc = exectu (filedes, &tucb);
		alarm (0);
                if (rc != NO_ERROR)
                {
                        if ((verify_rc(17,rc) == TRUE ) || alarm_timeout)
                        {
                                report_frub (&fddi_frus[FRU_117]);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                        else
                                DA_SETRC_ERROR (DA_ERROR_OTHER);
                        return (ERROR_FOUND);
                }
                return (NO_ERROR);
        }
        else    /* NOTLM or ENTERLM     */
        {
                /* ask if the user has the wrap plug */
                if (card_type == FOXCROFT)
                {
                   if ((rc = extender_card_exist ()) == TRUE )
                   {
			/* display message of two wrap plugs and two cables  */
			/* requirement for this tests			     */
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                                foxcroft_two_wrap_yes_no, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_foxcroft_two_wrap_yes_no);
                        sprintf(msgstr,asi_foxcroft_two_wrap_yes_no[0].text,
                                da_input.dname, da_input.dnameloc,
				extender_card_name, extender_loc);
                        free (asi_foxcroft_two_wrap_yes_no[0].text);
                        asi_foxcroft_two_wrap_yes_no[0].text = msgstr;

                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                           NULL, DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes,asi_foxcroft_two_wrap_yes_no);
	   	   }	
		   else	/* just display one wrap plug requirement */
		   {
			
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                                foxcroft_wrap_yes_no, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_foxcroft_wrap_yes_no);
                        sprintf(msgstr,asi_foxcroft_wrap_yes_no[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_foxcroft_wrap_yes_no[0].text);
                        asi_foxcroft_wrap_yes_no[0].text = msgstr;

                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                           NULL, DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes,asi_foxcroft_wrap_yes_no);
		   }
                }
                else	/* ask the user for wrap plug FRONT ROYAL & SCARBOR */
                {
                   if ((rc = extender_card_exist ()) == TRUE )
                   {
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                                two_wrap_yes_no, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_two_wrap_yes_no);
                        sprintf(msgstr, asi_two_wrap_yes_no[0].text,
                                da_input.dname, da_input.dnameloc,
				extender_card_name, extender_loc);
                        free (asi_two_wrap_yes_no[0].text);
                        asi_two_wrap_yes_no[0].text = msgstr;

                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                           NULL, DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes,asi_two_wrap_yes_no);
		   }
		   else
		   {
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                                wrap_yes_no, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes, asi_wrap_yes_no);
                        sprintf(msgstr, asi_wrap_yes_no[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_wrap_yes_no[0].text);
                        asi_wrap_yes_no[0].text = msgstr;

                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(FDDI_MENU_NUMBER_4, catd,
                           NULL, DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                &menutypes,asi_wrap_yes_no);
		   }
                }
                check_asl_stat();

                rc = DIAG_ITEM_SELECTED (menutypes);
                if      /* if user does not have the wrap NTF */
                        ( rc != YES)
                {
			fddi_display();
                        wrap_plug = FALSE;
                        putdavar(da_input.dname, "wrap_plug",
                                DIAG_SHORT, &wrap_plug);
                        return (NO_ERROR);

                }
                check_asl_stat();
                if (card_type == FOXCROFT)
                {
                        if ((rc = extender_card_exist ()) == TRUE )
                        {
                           network_cable_extender_card = TRUE;
                           putdavar(da_input.dname,
                                "network_cable_extender_card",
                           DIAG_SHORT, &network_cable_extender_card);
			   /* ask the user to put wrap plug for	*/
		           /* both extender adapter and main	*/

                            rc = diag_display(FDDI_MENU_NUMBER_5,
                                        catd, foxcroft_wrap_msg_all,
                                        DIAG_MSGONLY,
                                        ASL_DIAG_KEYS_ENTER_SC,
                                        &menutypes,
                                        asi_foxcroft_wrap_msg_all);
                            sprintf(msgstr,
                                      asi_foxcroft_wrap_msg_all[0].text,
                                      da_input.dname, da_input.dnameloc,
				      extender_card_name, extender_loc);
                            free(asi_foxcroft_wrap_msg_all[0].text);
                            asi_foxcroft_wrap_msg_all[0].text = msgstr;
                            sprintf(msgstr1,
                                asi_foxcroft_wrap_msg_all[1].text,
                                        da_input.dnameloc,extender_loc);
                            free (asi_foxcroft_wrap_msg_all[1].text);
                            asi_foxcroft_wrap_msg_all[1].text = msgstr1;

                            rc = diag_display(FDDI_MENU_NUMBER_5,
                                        catd, NULL, DIAG_IO,
                                        ASL_DIAG_KEYS_ENTER_SC,
                                        &menutypes,
                                        asi_foxcroft_wrap_msg_all);
                        }
                        else
                        {
				/* just the main adapter only	*/
                                rc = diag_display(FDDI_MENU_NUMBER_5,
                                        catd, foxcroft_wrap_msg,
                                        DIAG_MSGONLY,
                                        ASL_DIAG_KEYS_ENTER_SC,
                                        &menutypes,
                                        asi_foxcroft_wrap_msg);
                                sprintf(msgstr,
                                        asi_foxcroft_wrap_msg[0].text,
                                        da_input.dname,
                                        da_input.dnameloc);
                                free (asi_foxcroft_wrap_msg[0].text);
                                asi_foxcroft_wrap_msg[0].text = msgstr;
                                sprintf(msgstr1,
                                        asi_foxcroft_wrap_msg[1].text,
                                        da_input.dnameloc);
                                free (asi_foxcroft_wrap_msg[1].text);
                                asi_foxcroft_wrap_msg[1].text = msgstr1;

                                rc = diag_display(FDDI_MENU_NUMBER_5,
                                        catd, NULL, DIAG_IO,
                                        ASL_DIAG_KEYS_ENTER_SC,
                                        &menutypes,
                                        asi_foxcroft_wrap_msg);
                        }
                }
                else 	/* frontroyal or scarborough 	*/
                {
                        if ((rc = extender_card_exist ()) == TRUE )
                        {
                           network_cable_extender_card = TRUE;
                           putdavar(da_input.dname,
                                "network_cable_extender_card",
                           DIAG_SHORT, &network_cable_extender_card);

                            rc = diag_display(FDDI_MENU_NUMBER_5, catd, 
					fddi_wrap_msg_all, DIAG_MSGONLY, 
					ASL_DIAG_KEYS_ENTER_SC, &menutypes, 
					asi_fddi_wrap_msg_all);
                            sprintf(msgstr, asi_fddi_wrap_msg_all[0].text, 
					da_input.dname, da_input.dnameloc,
				      	extender_card_name, extender_loc);
                            free(asi_fddi_wrap_msg_all[0].text);
                            asi_fddi_wrap_msg_all[0].text = msgstr;
                            sprintf(msgstr1, asi_fddi_wrap_msg_all[1].text, 
					da_input.dnameloc,extender_loc);
                            free (asi_fddi_wrap_msg_all[1].text);
                            asi_fddi_wrap_msg_all[1].text = msgstr1;

                            rc = diag_display(FDDI_MENU_NUMBER_5, catd, NULL, 
					DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, 
					&menutypes, asi_fddi_wrap_msg_all);

			}
			else
			{
				/* ask the user to put the cable into 	*/
				/* the main adapter only		*/

                        	rc = diag_display(FDDI_MENU_NUMBER_5, catd,
                                	wrap_msg, DIAG_MSGONLY,
                                	ASL_DIAG_KEYS_ENTER_SC,
                                	&menutypes, asi_wrap_msg);
                        	sprintf(msgstr, asi_wrap_msg[0].text,
                                	da_input.dname, da_input.dnameloc);
                        	free (asi_wrap_msg[0].text);
                        	asi_wrap_msg[0].text = msgstr;
                        	sprintf(msgstr1, asi_wrap_msg[1].text,
                                	da_input.dnameloc);
                        	free (asi_wrap_msg[1].text);
                        	asi_wrap_msg[1].text = msgstr1;

                        	rc = diag_display(FDDI_MENU_NUMBER_5, catd,
                                	NULL, DIAG_IO, ASL_DIAG_KEYS_ENTER_SC,
                                	&menutypes,asi_wrap_msg);
			}
                }

                check_asl_stat ();

                network_cable_main_card = TRUE;
                putdavar(da_input.dname, "network_cable_main_card",
                        DIAG_SHORT, &network_cable_main_card);
                fddi_display();
                /* running test 17      */
                rc = test_tu (17, FRU_117);
                if (rc != NO_ERROR) return (ERROR_FOUND);

        } /* else NOTLM or ENTERLM      */
        /* running data wrap test for extender card     */
        if ((rc = extender_card_exist ()) == TRUE )
        {
                rc = extender_card_wrap_test ();
                check_asl_stat ();
        }
        return (rc);
}
/*......................................................*/
/*            	Perform non interactive test group A 	*/
/*							*/
/*		Running port wrap test			*/
/*......................................................*/

int non_interactive_tests_A (start)
struct tu_fru_pair *start;
{
	int	rc= NO_ERROR;
	int	asl_rc;

	
	for (; start->tu != -1; start++)
	{
		rc = test_tu (start->tu, start->fru);
		if (rc != NO_ERROR)
			break;
		/* check if cancel or exit key are pressed	*/
 		check_asl_stat (); 

	}
	if (rc == NO_ERROR)
	{
		rc = extender_card_exist ();
		if (rc )
		{
			/* class A data path testing for extender card	*/
			tucb.header.tu = 30;
			alarm_timeout = FALSE;
			alarm (300);
			rc = exectu (filedes, &tucb);
			alarm (0);
 			check_asl_stat ();
			if ( rc != NO_ERROR)
			{
           			if ((verify_rc(30,rc)== TRUE ) ||
						alarm_timeout)
				{
					if ((rc = extender_card_in_database ()) == TRUE )
					{
					   strcpy (fddi_frus[FRU_130].frus[0].fname, extender_card_name);
                		     	   report_frub (&fddi_frus[FRU_130]);
					}
					else
                		     	   report_frub (&fddi_frus[FRU_130E]);
                			DA_SETRC_STATUS (DA_STATUS_BAD);
				}
				else
					DA_SETRC_ERROR (DA_ERROR_OTHER);
				return (ERROR_FOUND);
				
			}
			/* check if cancel or exit key are pressed	*/

 			check_asl_stat (); 

			/* VPD test for extender card			*/
			/* actually both test 32 and 35 will test 	*/
			/* VPD extender card however according to 	*/
			/* different version 				*/

			tucb.header.tu = 32;
			alarm_timeout = FALSE;
			alarm (300);
			rc = exectu (filedes, &tucb);
			alarm (0);
 			check_asl_stat ();
			if (rc == NO_ERROR) return (NO_ERROR);
			else
			{
				tucb.header.tu = 35;
				alarm_timeout = FALSE;
				alarm (300);
				rc = exectu (filedes, &tucb);
				alarm (0);

				if ( rc != NO_ERROR)
				{
           				if ((verify_rc(35,rc) == TRUE ) 
						|| alarm_timeout)
					{
						if ((rc = extender_card_in_database ()) == TRUE )
						{
							strcpy ( fddi_frus[FRU_135].frus[0].fname, extender_card_name);
                			     		report_frub (&fddi_frus[FRU_135]);
						}
						else
                			     		report_frub (&fddi_frus[FRU_135E]);
                			     DA_SETRC_STATUS (DA_STATUS_BAD);
					}
					else
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (ERROR_FOUND);
				}
			}

			/* check if cancel or exit key are pressed	*/
 			check_asl_stat (); 
		}
	}

 	check_asl_stat ();
	return (rc);
}
/*......................................................*/
/*            	Perform non interactive test group B 	*/
/*							*/
/*		Running port wrap test			*/
/*......................................................*/
int non_interactive_tests_B (start)
struct tu_fru_pair	*start;
{
	int	rc= NO_ERROR;
	int	asl_rc;
	
	for (; start->tu != -1; start++)
	{
		rc = test_tu (start->tu, start->fru);
		if (rc != NO_ERROR)
			break;
		/* check if cancel or exit key are pressed	*/
 		check_asl_stat (); 
	}
	return (rc);
}
/*......................................................*/
/*     Procedure description : display           	*/
/*......................................................*/

void fddi_display ()
{

	if ( da_input.loopmode == LOOPMODE_INLM) 

		display_menu ( TESTING_LOOPMODE);
	
	else if ( da_input.advanced == ADVANCED_TRUE)
		display_menu ( TESTING_ADVANCED_MODE);

	else
		display_menu (TESTING_REGULAR) ;

 	check_asl_stat ();
}

/*......................................................*/
/*    Procedure description :   opening the device	*/
/*				If error, switch 	*/
/*				on errors.		*/
/*......................................................*/

int open_device ()
{
        int     rc =NO_ERROR;
        int     open_error;
        char devname[32];


        sprintf(devname,"/dev/%s/D",da_input.dname);

        /* open device                  */
        filedes = open(devname, O_RDWR | O_NDELAY);
        open_error = errno;
	check_asl_stat ();

        if (filedes == ERROR_FOUND  )
        {
		switch (open_error)
		{
			case ENODEV:
			case EIO:
				/* report fru		*/
                		report_frub (&fddi_frus[FRU_151]);
                        	DA_SETRC_STATUS (DA_STATUS_BAD);
				break;
			case EPERM:
			case EBUSY:
                        	DA_SETRC_ERROR (DA_ERROR_OPEN);
				break;
			default:
				DA_SETRC_ERROR (DA_ERROR_OTHER);
				break;
		}
        	return (open_error);
        }
	if (da_input.dnameloc[3] == '1')	/* Second I/O Planar */
	{
		mdd_fdes = open( "/dev/bus1", O_RDWR);
	}
 	else
		mdd_fdes = open( "/dev/bus0", O_RDWR);
 	check_asl_stat ();
        if (mdd_fdes == ERROR_FOUND  )
        {
        	DA_SETRC_ERROR (DA_ERROR_OPEN);
		return (errno);
	}
	return (NO_ERROR);
} /* open_device */

/*......................................................*/
/*     Procedure description :  Cleanup before exit DA	*/ 
/*                              closing catalog file	*/
/*				quiting asl, restore  	*/
/*				the database to the     */
/*				initial state		*/
/*......................................................*/

clean_up ()
{
	int	rc = NO_ERROR;

	if (filedes != ERROR_FOUND)
		close (filedes);
	if (mdd_fdes != ERROR_FOUND)
		close (mdd_fdes);

        /*  Unload the diagnostics device driver and set the lpp device */
        /*  to original state                                             */
        configure_lpp_device();

	if (odm_flag != ERROR_FOUND) 
		term_dgodm();
	if ( da_input.console == CONSOLE_TRUE )
	{
		/* closing fddi catalog file , asl */
		if (catd != CATD_ERR)
			catclose (catd);
		diag_asl_quit (NULL);
	}

	DA_EXIT ();
}

/*......................................................*/
/*     Procedure description : 	Error logging analysis 	*/ 
/*......................................................*/

void error_log_analysis ()
{
	int ela_rc , ela_get_rc;
        char crit[128];
        struct errdata errdata_str;
	int	rc;

        /*..........................................................*/
        /* Look for an occurrence of any of the following errors in */
        /* the error log.  If one exists, exit_with_fru for ELA.    */
        /*..........................................................*/

        sprintf(crit,"%s -N %s -j %X,%X,%X,%X,%X,%X,%X,%X",
                  	da_input.date,
                  	da_input.dname,
			ERRID_CFDDI_DWNLD, 
			ERRID_CFDDI_MC_ERR,    
			ERRID_CFDDI_PIO,      
			ERRID_CFDDI_SELFT_ERR,
			ERRID_CFDDI_SELF_TEST,
			ERRID_CFDDI_BYPASS,
			ERRID_CFDDI_PORT,
			ERRID_CFDDI_DOWN);

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
	if (ela_get_rc >0)
	{
		DA_SETRC_STATUS (DA_STATUS_BAD);
		switch (errdata_str.err_id)
		{
			case ERRID_CFDDI_DWNLD:
                        case ERRID_CFDDI_MC_ERR:
                        case ERRID_CFDDI_PIO:
                        case ERRID_CFDDI_SELFT_ERR:
                        case ERRID_CFDDI_SELF_TEST:
                        case ERRID_CFDDI_BYPASS:
                        case ERRID_CFDDI_PORT:
                        case ERRID_CFDDI_DOWN:
			if ((rc = extender_card_in_database ()) == TRUE )
			{
				strcpy ( fddi_frus[FRU_134].frus[1].fname, 
					extender_card_name);
				strcpy ( fddi_frus[FRU_134].frus[1].floc, 
					extender_loc);
				/* report fru */
                		report_frub (&fddi_frus[FRU_134]);
			}
			else
                		report_frub (&fddi_frus[FRU_134E]);
			break;

			default:
				DA_SETRC_ERROR (DA_ERROR_OTHER);
				break;
		}
	}
        ela_rc = error_log_get(TERMI,crit,&errdata_str);
}



/*
 * NAME: intr_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void intr_handler(int sig)
{
        if ( da_input.console == CONSOLE_TRUE ) 
                diag_asl_clear_screen();
	/* restoring adapter to normal state	*/
	restore_normal_adapter();
        clean_up();
}
/*......................................................................*/
/*	Function description :	This function will display three 	*/
/*				different message. Advance, loopmode 	*/
/*				and regular testing menu		*/
/*......................................................................*/

void display_menu (menu_type)
int	menu_type;
{
	int	rc;

	switch (menu_type)
	{
		case TESTING_LOOPMODE:
                   	if ((rc = extender_card_exist ()) == TRUE )
			{
		 	    diag_msg_nw ( FDDI_MENU_NUMBER_3, catd, 
	   	     	      1, TESTING_LOOPMODE_WITH_EXTENDER,da_input.dname,
			      da_input.dnameloc, extender_card_name,
			      extender_loc, da_input.lcount, da_input.lerrors);
			}
			else
			{
		 		diag_msg_nw ( FDDI_MENU_NUMBER_3, catd, 
		   	     		1, TESTING_LOOPMODE,da_input.dname, 
			     		da_input.dnameloc, da_input.lcount,
			     		da_input.lerrors);
			}
			break;
		case TESTING_ADVANCED_MODE:
                   	if ((rc = extender_card_exist ()) == TRUE )
			{
				diag_msg_nw ( FDDI_MENU_NUMBER_2, catd, 
		   	     		1, TESTING_ADVANCED_MODE_WITH_EXTENDER,
					da_input.dname, da_input.dnameloc,
					extender_card_name, extender_loc); 
			}
			else
			{
				diag_msg_nw ( FDDI_MENU_NUMBER_2, catd, 
		   	     		1, TESTING_ADVANCED_MODE,da_input.dname,
			     		da_input.dnameloc); 
			}

			break;
		case TESTING_REGULAR:
                   	if ((rc = extender_card_exist ()) == TRUE )
			{
				diag_msg_nw ( FDDI_MENU_NUMBER_1, catd, 
		   		   1, TESTING_REGULAR_WITH_EXTENDER,
				   da_input.dname, da_input.dnameloc,
				   extender_card_name, extender_loc); 
			}
			else
			{
				diag_msg_nw ( FDDI_MENU_NUMBER_1, catd, 
		   		   1, TESTING_REGULAR,da_input.dname,
			           da_input.dnameloc);
			}

			break;
	}
 	check_asl_stat ();
}

/*......................................................................*/
/*	Function description :	This function will add the fru bucket 	*/
/*				into the data base			*/
/*......................................................................*/

int report_frub (frub_addr)
struct fru_bucket *frub_addr;
{
	int	rc;

	/* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);
	/* update FRU Bucket with parent device name 		*/


	if (card_type == FRONT_ROYAL)
	{
		rc = extender_card_exist ();
		if (rc == TRUE)
		{
			
			frub_addr =  (struct fru_bucket *) &frub180;
        		strcpy ( frub_addr->dname, da_input.dname);
        		strcpy ( frub_addr->frus[1].floc, extender_loc);
        		strcpy ( frub_addr->frus[1].fname, extender_card_name);
			rc = insert_frub (&da_input, &frub180);
		}
		else
		{
			frub_addr =  (struct fru_bucket *) &frub170;
        		strcpy ( frub_addr->dname, da_input.dname);
        		strcpy ( frub_addr->frus[1].floc, extender_loc);
			rc = insert_frub (&da_input, &frub170);
		}
	}
       	else 
		rc = insert_frub(&da_input, frub_addr);
	if (rc != NO_ERROR)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		return (ERROR_FOUND);
	}

	/* add a FRU bucket; 					*/
	if (card_type == FOXCROFT)
		frub_addr->sn = 0x997;
	else
		frub_addr->sn = 0x859;

	rc = addfrub(frub_addr);
	if (rc != NO_ERROR)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		return (ERROR_FOUND);
	}

}
/*......................................................................*/
/*	Function:	check  asl     return code			*/
/*......................................................................*/
check_asl_stat ()
{
	int rc;
				
	if (da_input.console != CONSOLE_TRUE)
	{
		return (0);
	}
	rc =  diag_asl_read ( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0);
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
		case ASL_OK:
		case ASL_ENTER:
		case ASL_COMMIT:
			break;
		
	}
	return (0);
} /* check_asl_stat 	*/

/*---------------------------------------------------------------------*/
/* NAME: fddi_davar_op
 *
 * FUNCTION: Initialize or read the necessary davars
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *       if op is FDDI_INIT_OP then initialize davars
 *       else if op is FDDI_GET_OP then read davars
 *
 * RETURNS:
 *       None
 *----------------------------------------------------------------------*/

void fddi_davar_op (op)
int	op;
{
	int	rc;

        switch(op) 
	{
        case FDDI_INIT_OP:
                wrap_plug = TRUE;
		network_cable_main_card = FALSE;
		network_cable_extender_card = FALSE;
		extender_card = TRUE;

                putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
			&wrap_plug);
                putdavar(da_input.dname, "network_cable_main_card",
			 DIAG_SHORT, &network_cable_main_card);
                putdavar(da_input.dname, "network_cable_extender_card",
                         DIAG_SHORT, &network_cable_extender_card);
                putdavar(da_input.dname, "extender_card",
                         DIAG_SHORT, &extender_card);
                break;

        case FDDI_GET_OP:
                getdavar(da_input.dname, "wrap_plug",
                         DIAG_SHORT, &wrap_plug);
                getdavar(da_input.dname, "network_cable_main_card",
                         DIAG_SHORT, &network_cable_main_card);
                getdavar(da_input.dname, "network_cable_extender_card",
                         DIAG_SHORT, &network_cable_extender_card);
                getdavar(da_input.dname, "extender_card",
                         DIAG_SHORT, &extender_card);
	}
 	check_asl_stat ();

}	/* fddi_davar_op	*/


/* NAME: restore_normal_adapter
 *
 * FUNCTION: Restoring the FDDI adapter to normal state
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *	     No report errors. Because when calling this function
 *	     there is already an error occured.
 *	     Do not need check return code from exectu
 *
 * RETURNS:
 *       None
 *----------------------------------------------------------------------*/

void restore_normal_adapter()
{
	int rc;

	/* ignore the signal until this test is done */
 	(void) signal (SIGINT, SIG_IGN);
	tucb.header.tu = 20;
	rc = exectu (filedes, &tucb);
 	check_asl_stat ();
 	/* restore the interrrupt       */
        (void) signal (SIGINT, intr_handler);

	tucb.header.tu = 18;
	rc = exectu (filedes, &tucb);
 	check_asl_stat ();

}	/* restore_normal_adapter 	*/


/*-------------------------------------------------------------------------
 *
 * FUNCTION: 	run test unit and report FRU specified by fru_index
 *           	for failed test; 
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Other local routines called --
 *         report_frub()
 *----------------------------------------------------------------------*/

int test_tu (test_unit, fru_index)
int	test_unit, fru_index;
{
	int	rc;

  	alarm_timeout = FALSE;
        alarm(300);

	/* this is recommended by engineering. If the signal is caught  */
	/* during test 20. It will cause the error in the next run	*/
	/* because the POS is not save as normal			*/
	if (test_unit == 20)	/* ingnore the interrupt signal	*/
		 (void) signal (SIGINT, SIG_IGN);
	tucb.header.tu = test_unit;
	rc = exectu (filedes, &tucb);
	if (test_unit == 20)	/* restore the interrupt signal */
		(void) signal (SIGINT, intr_handler);
	alarm (0);
	if (rc != NO_ERROR)
	{

           	if ((verify_rc(test_unit,rc) == TRUE ) || alarm_timeout)
		{
			if (( test_unit == 27) && ( rc == 0x1b0250bc))
			{
                		report_frub (&fddi_frus[FRU_127E]);
			}
			else
                		report_frub (&fddi_frus[fru_index]);
                	DA_SETRC_STATUS (DA_STATUS_BAD);
		}
		else
			DA_SETRC_ERROR (DA_ERROR_OTHER);
	}

 	check_asl_stat ();
        return(rc);
}

/*--------------------------------------------------------------
 * NAME: verify_rc
 *
 * FUNCTION: verify the return code from exectu() of the specified
 *           test unit is a valid return code
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:  Other local routines called --
 *
 * RETURNS:
 *       TRUE -- is in valid return code range
 *       FALSE - is not a valid return code
 *--------------------------------------------------------------*/
verify_rc(test_unit,rc)
int	test_unit, rc;
{
	int *rc_ptr = tu_rc_table[--test_unit];

        for (; *rc_ptr; rc_ptr++)
                if (*rc_ptr == rc )
                        return(TRUE);

        return(FALSE);
}

/*--------------------------------------------------------------
 * NAME: extender_card_exist 
 *
 * FUNCTION: Retrieve the information from the database if the extender
 *           card exist.
	     And if the extender is not missing.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *       TRUE -- if extender card exists 
 *       FALSE - else 
 *--------------------------------------------------------------*/

int extender_card_exist()
{
	char    odm_search_crit[40];
        struct  CuDv            *cudv;
        struct  listinfo        obj_info;
	int	rc;
	int	i;

	rc = 1;
	/* find out what extender card whose dname is its parent	*/
	for (i=0; i<=15;i++)
	{
		sprintf (odm_search_crit, "name = %s",extender_card_name);
       		cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit,
       					&obj_info, 1, 2 );
		rc = strcmp (cudv->parent, da_input.dname);	
		if (rc == 0)
		 	break;
		else
			extender_card_name[5] += 1;
	}

	if ((rc == 0 ) && ( cudv->chgstatus != MISSING))
		return (TRUE);
	else return (FALSE);
}
/*--------------------------------------------------------------
 * NAME: extender_card_wrap_test
 *
 * FUNCTION: Running the wrap test for extender card
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *       TRUE -- Running test passed  
 *       FALSE - else 
 *--------------------------------------------------------------*/

int extender_card_wrap_test()
{
	int	rc;
	
	tucb.header.tu = 33;
	alarm_timeout = FALSE;
	alarm (300);
	rc = exectu(filedes, &tucb);
	alarm (0);
 	check_asl_stat ();
	if (rc == NO_ERROR) 
		return (NO_ERROR);
	else 
	{
          	if ((verify_rc(33,rc) == TRUE )  || alarm_timeout)
		{
			if ((rc = extender_card_in_database ()) == TRUE )
			{
				strcpy ( fddi_frus[FRU_133].frus[0].fname, 
					extender_card_name);
               			report_frub (&fddi_frus[FRU_133]);
			}
			else
               			report_frub (&fddi_frus[FRU_133E]);
               		DA_SETRC_STATUS (DA_STATUS_BAD);
		}
		else
			DA_SETRC_ERROR (DA_ERROR_OTHER);
		return (ERROR_FOUND);
	}

	return (NO_ERROR);

}	/* extender_wrap_test 	*/


/*----------------------------------------------------------------
 * NAME: unplug_cable_main_card
 *
 * FUNCTION: Asking the user unplug network cables for main adapter 
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *--------------------------------------------------------------*/

void unplug_wrap_plug_main_card ()
{

	diag_display(FDDI_MENU_NUMBER_8, catd, unplug_msg,
       		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
               	&menutypes, asi_unplug_msg);

	sprintf(msgstr, asi_unplug_msg[0].text, 
		da_input.dname, da_input.dnameloc);
	free (asi_unplug_msg[0].text);
       	asi_unplug_msg[0].text = msgstr;
       	sprintf(msgstr1, asi_unplug_msg[1].text, da_input.dnameloc);
	free (asi_unplug_msg[1].text);
       	asi_unplug_msg[1].text = msgstr1;

  	diag_display(FDDI_MENU_NUMBER_8, catd, 
		NULL, DIAG_IO,
  	ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_unplug_msg);
 	check_asl_stat ();

}


/*----------------------------------------------------------------
 * NAME: unplug_cable_all
 *
 * FUNCTION: Asking the user unplug network cables for main adapter 
 *	     and extender adapter.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *--------------------------------------------------------------*/

void unplug_wrap_plug_all ()
{

	diag_display(FDDI_MENU_NUMBER_8, catd, all_unplug_msg,
       		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
               	&menutypes, asi_all_unplug_msg);

	sprintf(msgstr, asi_all_unplug_msg[0].text, 
		da_input.dname, da_input.dnameloc,
		extender_card_name, extender_loc);
	free (asi_all_unplug_msg[0].text);
       	asi_all_unplug_msg[0].text = msgstr;
       	sprintf(msgstr1, asi_all_unplug_msg[1].text, da_input.dnameloc,
		  extender_loc);
	free (asi_all_unplug_msg[1].text);
       	asi_all_unplug_msg[1].text = msgstr1;

  	diag_display(FDDI_MENU_NUMBER_8, catd, 
		NULL, DIAG_IO,
  	ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_all_unplug_msg);
 	check_asl_stat ();

}
void foxcroft_unplug_wrap_plug_all ()
{

	diag_display(FDDI_MENU_NUMBER_8, catd, foxcroft_all_unplug_msg,
       		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
               	&menutypes, asi_foxcroft_all_unplug_msg);

	sprintf(msgstr, asi_foxcroft_all_unplug_msg[0].text, 
		da_input.dname, da_input.dnameloc,
		extender_card_name, extender_loc);
	free (asi_foxcroft_all_unplug_msg[0].text);
       	asi_foxcroft_all_unplug_msg[0].text = msgstr;
       	sprintf(msgstr1, asi_foxcroft_all_unplug_msg[1].text, da_input.dnameloc,
		  extender_loc);
	free (asi_foxcroft_all_unplug_msg[1].text);
       	asi_foxcroft_all_unplug_msg[1].text = msgstr1;

  	diag_display(FDDI_MENU_NUMBER_8, catd, 
		NULL, DIAG_IO,
  	ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_foxcroft_all_unplug_msg);
 	check_asl_stat ();

}
/*--------------------------------------------------------------
 * NAME: extender_card_in_database 
 *
 * FUNCTION: Retrieve the information from the database if the extender
 *           card exist.
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *       TRUE -- if extender card  is in database 
 *       FALSE - else 
 *--------------------------------------------------------------*/

int extender_card_in_database()
{
	char    odm_search_crit[40];
        struct  CuDv            *cudv;
        struct  listinfo        obj_info;
	int	rc;
	int	i;

	rc = 1;
	/* find out what extender card whose dname is its parent	*/
	for (i=0; i<=15;i++)
	{
		sprintf (odm_search_crit, "name = %s",extender_card_name);
       		cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit,
       					&obj_info, 1, 2 );
		rc = strcmp (cudv->parent, da_input.dname);	
		if (rc == 0)
		 	break;
		else
			extender_card_name[5] += 1;
	}

	if (rc == 0 ) 
		return (TRUE);
	else return (FALSE);
}
/*--------------------------------------------------------------*/
/* 	NAME: card_determination				*/
/*	Description:	This function will search in CuVPD	*/
/*			database for FRU Part Number 		*/
/*			This FRU part number will tell which	*/
/*			type of FDDI card it is			*/
/*			FRU Part Number : 81F9003 : Front Royal */
/*			FRU Part Number : 93F0377 : Scarborough */
/*			FRU Part Number : 93F0379 : Foxcroft    */
/*--------------------------------------------------------------*/
void card_determination()
{

	struct CuVPD    *vpd_p;
        struct listinfo v_info;
        char    criteria[128];
	int	i;
	int	result;

	card_type = FOXCROFT;	/* default is Foxcroft	*/
	sprintf (criteria, "name = %s", da_input.dname);
	vpd_p = get_CuVPD_list(CuVPD_CLASS, criteria, &v_info, 1, 1);
	if ( vpd_p == (struct CuVPD *) -1 ) 
	{
		return;
        }
	for (i=0; i< 512; i++)
	{
		result = memcmp ( &vpd_p->vpd[i], "81F9003", 7);
		if (result == 0)
		{
			card_type = FRONT_ROYAL;
			break;
		}
		result = memcmp ( &vpd_p->vpd[i], "0377", 4);
		if (result == 0)
		{
			card_type = SCARBOROUGH;
			break;
		}
		result = memcmp ( &vpd_p->vpd[i], "0379", 4);
		if (result == 0)
		{
			card_type = FOXCROFT;
			break;
		}
	}
}
/*----------------------------------------------------------------
 * NAME: foxcroft_unplug_cable_main_card
 *
 * FUNCTION: Asking the user unplug network cables for main adapter 
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS:
 *--------------------------------------------------------------*/

void foxcroft_unplug_wrap_plug_main_card ()
{

	diag_display(FDDI_MENU_NUMBER_8, catd, foxcroft_unplug_msg,
       		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
               	&menutypes, asi_foxcroft_unplug_msg);

	sprintf(msgstr, asi_foxcroft_unplug_msg[0].text, 
		da_input.dname, da_input.dnameloc);
	free (asi_foxcroft_unplug_msg[0].text);
       	asi_foxcroft_unplug_msg[0].text = msgstr;
       	sprintf(msgstr1, asi_foxcroft_unplug_msg[1].text, da_input.dnameloc);
	free (asi_foxcroft_unplug_msg[1].text);
       	asi_foxcroft_unplug_msg[1].text = msgstr1;

  	diag_display(FDDI_MENU_NUMBER_8, catd, 
		NULL, DIAG_IO,
  	ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_foxcroft_unplug_msg);
 	check_asl_stat ();

}
/*--------------------------------------------------------------------------
 | NAME: timeout()
 |
 | FUNCTION: Desined to handle timeout interrupt handlers.
 |
 | EXECUTION ENVIRONMENT:
 |
 |      Environment must be a diagnostics environment wich is described
 |      in Diagnostics subsystem CAS.
 |
 | RETURNS: NONE
 ---------------------------------------------------------------------------*/

void timeout(int sig)
{
        alarm(0);
        alarm_timeout = TRUE;
}

/*---------------------------------------------------------------------------
 * NAME: configure_lpp_device () 
 *
 * FUNCTION: configure_lpp_device ()
 *	     Unload the diagnostic device driver 
 *	     setting the device to the DEFINE state (No matter what state
 *	     it is in before running the test. We do not want left it in
 *	     DIAGNOSE state after running diagnostics 
 *	     clean_up will restore the AVAILABLE state if it is so before
 *	     running the test
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/
int	configure_lpp_device()
{
        char    criteria[128];
	int	result;

	/* Unconfigure diagnostics device. Unload the device from system */
	if(diag_device_configured)
	{
		/* UCFG is defined as 1			*/
		sprintf (option," -l %s -f 1", da_input.dname);
		strcpy (criteria,"/usr/lib/methods/cfgddfddi");
		result = invoke_method(criteria,option, &new_out,&new_err);
		free(new_out);
		free(new_err);
		if (result !=  NO_ERROR)
		{
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
				DIAG_DEVICE_CANNOT_UNCONFIGURED), 
                                da_input.dname, da_input.dnameloc);
			if  ((da_input.console == CONSOLE_TRUE) &&
			    ((da_input.loopmode == LOOPMODE_ENTERLM)
			   	|| (da_input.loopmode == LOOPMODE_NOTLM)))
			{
				menugoal (msgstr);
			}
		}
	}

	/* Setting the device state to original state */
        if(diag_device_diagnose) {
                result = diagex_initial_state (da_input.dname);
                switch (result) 
                {
		case 4:
                        sprintf(msgstr, (char *)diag_cat_gets (
                                catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                da_input.dname, da_input.dnameloc);
			if  ((da_input.console == CONSOLE_TRUE) &&
			    ((da_input.loopmode == LOOPMODE_ENTERLM)
			   	|| (da_input.loopmode == LOOPMODE_NOTLM)))
			{
				menugoal (msgstr);
			}
			break;
		case 5:
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_CONFIGURED),
                                da_input.dname, da_input.dnameloc);
                        if  ((da_input.console == CONSOLE_TRUE) &&
                            ((da_input.loopmode == LOOPMODE_ENTERLM)
                                || (da_input.loopmode == LOOPMODE_NOTLM)))
                        {
                                menugoal (msgstr);
                        }
			break;
		}
	}

}
/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device 				*/
/*      Description:   						*/ 
/*	Return		: 0 Good 				*/
/*			  -1 BAD				*/
/*--------------------------------------------------------------*/
int	unconfigure_lpp_device()
{
        char    criteria[128];
	int	result, rc;

        result = diagex_cfg_state ( da_input.dname );

        switch (result) {
                case 2:
                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                        clean_up();
                        break;
                case 3: sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
                                da_input.dname, da_input.dnameloc);
			if  ((da_input.console == CONSOLE_TRUE) &&
		    		  ((da_input.loopmode == LOOPMODE_ENTERLM)
		   	           || (da_input.loopmode == LOOPMODE_NOTLM)))
			{
				menugoal (msgstr);
			}
                        clean_up();
                        break;
	          case -1:
                        DA_SETRC_STATUS(DA_ERROR_OTHER);
                        clean_up ();
			break;	
        }
        diag_device_diagnose=TRUE;

	/* The diagnostics device driver needs to be loaded into the system */

	/* CFG is defined as 0	*/
	sprintf (option," -l %s -f 0", da_input.dname);
	strcpy (criteria,"/usr/lib/methods/cfgddfddi");
	result = invoke_method(criteria,option, &new_out,&new_err);
	free(new_out);
	free(new_err);
	if (result == NO_ERROR)
	{
		diag_device_configured=TRUE;
	}
	else
	{
		if ((rc = extender_card_in_database ()) == TRUE )
		{
			strcpy ( fddi_frus[FRU_150].frus[1].fname, 
				extender_card_name);
			strcpy ( fddi_frus[FRU_150].frus[1].floc, 
				extender_loc);
			/* report fru */
               		report_frub (&fddi_frus[FRU_150]);
		}
		else
               		report_frub (&fddi_frus[FRU_150E]);
		DA_SETRC_STATUS (DA_STATUS_BAD);
		clean_up ();
	}

	return (0);
}

