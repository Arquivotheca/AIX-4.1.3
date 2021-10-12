static char sccsid[] = "@(#)42	1.30  src/bos/diag/da/eth/denet.c, daeth, bos41J, 9523B_all 6/5/95 15:51:12";
/*
 *   COMPONENT_NAME: DAETH
 *
 *   FUNCTIONS: bnc_wrap_plug_testing
 *		check_asl_stat
 *		clean_malloc
 *		clean_up
 *		da_display
 *		da_odm_vars
 *		display
 *		dix_wrap_plug_testing
 *		error_log_analysis
 *		general_initialize
 *		interactive_tests
 *		intr_handler
 *		main
 *		non_interactive_tests
 *		open_device
 *		performing_test
 *		poll_kb_for_quit
 *		report_frub
 *		running_test_7
 *		set_timer
 *		test_10_base_2_transceiver
 *		test_tu
 *		test_twisted_pair_transceiver
 *		timeout
 *		unplug_transceiver
 *		unplug_xceiver_and_plug_dix_wrap
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include "ent_comio_errids.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/bit_def.h"
#include "denet_msg.h"
#include "eth.h"
#include "ethtst.h"
#include <locale.h>

extern  diag_exec_source();
extern 	invoke_method();
extern	nl_catd diag_catopen();
extern 	getdainput();		/* gets input from ODM 			*/
extern 	long getdamode();	/* transforms input into a mode 	*/
extern  int exectu (int,TUTYPE *); extern 	int errno;		/* reason device driver won't open 	*/

TUTYPE eth_tucb;
/* davar used */
short   wrap_plug = FALSE;	/* wrap plug is not on is assumed	*/
short   xceiver_10base2 = FALSE;	
short	xceiver_twisted = FALSE;
short	isolate_xceiver_10base2_process=FALSE;
short 	isolate_xceiver_twisted_process=FALSE;
short 	network_cable_only=FALSE;
volatile int	alarm_timeout=FALSE;
struct  sigaction  invec;   	/* interrupt handler structure  */	

int 	odm_flg = 0;		/* return code from init_dgodm() */
int	asl_flg = NOT_GOOD; 	/* return code from diag_asl_init() */

struct 	tm_input da_input;	/* initialized by getdainput 		*/
int 	filedes=NOT_GOOD;	/* file descriptor from device driver 	*/
int	diskette_mode;
int	diskette_based;
nl_catd catd;			/* pointer to message catalog  		*/
int	machine_type;
int	circuit_breaker_bad =FALSE;

char	option[256];
char	*new_out, *new_err;
int	prior_state=NOT_CONFIG;
int	unconfigure_lpp=FALSE;
int	diag_device_configured=FALSE;
int 	diag_device_diagnose=FALSE;
int	set_device_to_diagnose = FALSE;


int 	performing_test();
int 	interactive_tests();
int 	non_interactive_tests();
int 	test_tu(int,int);
int 	open_device();
int 	poll_kb_for_quit();
int 	check_asl_stat(int);
void	intr_handler(int);	
void 	error_log_analysis ();
int	display();
void 	da_display();
void 	report_frub();
void 	clean_up();
void 	timeout(int);
int 	unplug_cable_and_put_wrap ();
void 	general_initialize();
void	clean_malloc();
void 	da_odm_vars(int op);
int	unplug_xceiver_and_plug_dix_wrap();
int 	dix_wrap_plug_testing ();
int 	bnc_wrap_plug_testing ();
void 	set_timer();
int	test_10_base_2_transceiver();

/*--------------------------------------------------------------*/
/*	Messages and menus					*/
/*--------------------------------------------------------------*/

static ASL_SCR_TYPE	menutypes=DM_TYPE_DEFAULTS;
char *msgstr;
char *msgstr1;

/*......................................................................*/
/* RETURNS:			DA_STATUS_GOOD
 * 					No problems were found.
 *				DA_STATUS_BAD
 *					A FRU Bucket or a Menu Goal was
 *					reported.
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
 *				DA_TEST_FULL
 *					The full-tests were executed.
 *				DA_MORE_NOCONT
 *					The isolation process is complete.
 *				DA_MORE_CONT
 *					The path to the device should be
 *					tested.
 *
 ..........................................................................*/

main (argc, argv)
int argc;
char **argv;
{
	int	rc=0;			/* return code 			*/
	int	rc1=0;
	struct  stat stat_buf;          /* stat buffer defined in stat.h */
	int	diag_source=0;
	char	mountcdrfs[128];




	/*..............................*/
	/* General Initialization	*/
	/*..............................*/
	general_initialize ();

	/* executing test if running problem determination	*/
	/* or repair mode					*/
	if (( da_input.dmode & (DMODE_PD | DMODE_REPAIR)) &&
	    (!(da_input.loopmode & LOOPMODE_EXITLM)))
	{
		if  ( da_input.console == CONSOLE_TRUE) 
			da_display ();
	
		/* check if diskette package is running */
  		diskette_mode = ipl_mode(&diskette_based);
		diag_source = diag_exec_source(&mountcdrfs);

               	if ((diskette_based == DIAG_FALSE) &&
		    (diag_source == 0))
		{
			rc = stat ("/usr/lib/drivers/entdiag", &stat_buf);

                        /* if device driver file does not exist         */
                        if (( rc == ETH_ERROR) || ( stat_buf.st_size == 0 ))
                        {
                                /* just send menugoal one time only */

                                if  ((da_input.console == CONSOLE_TRUE) &&
                                     (da_input.loopmode & ( LOOPMODE_ENTERLM |
                                      LOOPMODE_NOTLM)))
                                {
                                        /* displaying the device driver */
                                        /* file error                   */
                                        sprintf (msgstr, (char *)diag_cat_gets (
                                                catd, 1,DD_FILE_BAD),
                                                NULL);
                                        menugoal (msgstr);
                                }
                                clean_up();
                        }

		}

 		prior_state=get_device_status(da_input.dname);
		rc = unconfigure_lpp_device();
	
		if (prior_state != -1)
		{
			/* opening the diagnostics device for testing	*/
			rc = open_device ();

			/* Perform testing and/or handle wrap plugs.
		 	*  rc = 0 means good path.
			*/
			if ( rc == NO_ERROR) 
			{

           			rc = performing_test();
			}
		}

				
	}

	da_odm_vars (READ_VARS);
	/* unplug something that is asked to plug in  during test	*/

	if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && ( 
		(isolate_xceiver_10base2_process)|| 
		(isolate_xceiver_twisted_process)) 
		&& ( da_input.console == CONSOLE_TRUE))
	{
		/* ask the user to unplug wrap DIX and put transceiver */ 
		/* menu 852-026 	*/
		if (isolate_xceiver_twisted_process)
			/* menu 852-026 	*/
			display(UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED);
		else
			/* menu 852-026 	*/
			display(UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2);
		da_display ();
	}	

	else if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && (wrap_plug) 
			&& ( da_input.console == CONSOLE_TRUE))
	{
		/* Ask the user to unplug the wrap plug	*/
		/* menu 852-019	*/
		display(UNPLUG_WRAP_GENERIC);
		da_display ();
	}	
	else if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && (xceiver_10base2) 
			&& ( da_input.console == CONSOLE_TRUE))
	{
		/* ask the user to unplug DIX wrap plug and put 10 base	*/
		/* 2 transceiver back into the network			*/
		/* menu 852-020	*/
		display(UNPLUG_DIX_AND_PUT_BASE2);
		da_display ();
	}	
	else if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && (xceiver_twisted) 
			&& ( da_input.console == CONSOLE_TRUE))
	{
		/* menu 852-022	*/
		display(PLUG_T_XCEIVER_BACK);
		da_display ();
	}	
	else if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && (network_cable_only) 
			&& ( da_input.console == CONSOLE_TRUE))
	{
		/* Menu 852030	*/
		/* Ask the user to plug the network cable only back	*/
		/* into the network. This has been asked to unplug	*/
		/* in Menu 852-015					*/
		display(PLUG_NETWORK_CABLE_ONLY);
		da_display ();
	}	

	 

	/* if running problem determination, or error log analyis 	*/
	/* running error log analysis					*/

	if  (rc == NO_ERROR && (( da_input.dmode ==  DMODE_PD) 
		|| (da_input.dmode == DMODE_ELA )) &&
		(da_input.loopmode & (LOOPMODE_NOTLM | LOOPMODE_ENTERLM)) )
	{
		  error_log_analysis ();
	}
	clean_up ();
	
} /* main */
/*--------------------------------------------------------------------------
| 	NAME: da_display
|
| 	FUNCTION: Update display for appropriate mode
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|		display()
|
| 	RETURNS:
|		None
----------------------------------------------------------------------------*/

void da_display ()
{
	if ( da_input.loopmode == LOOPMODE_INLM) 
		display (ETHERNET_LOOPMODE);
	else if ( da_input.advanced == ADVANCED_TRUE)
	{
		if ((da_input.loopmode & ( LOOPMODE_ENTERLM | LOOPMODE_NOTLM)))
			display(ADVANCED_NO_STANDBY_WITH_WAIT);
		else
			display (ETHERNET_ADVANCED);
	}
	else
		display (ETHERNET_CUSTOMER) ;
}


/*--------------------------------------------------------------------------
| 	NAME: Performing_test
|
| 	FUNCTION: Invokes interactive tests or non interactive tests
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|		non_interactive_tests()
|		interactive_tests()
|
| 	RETURNS:
|		same return code of exectu()
----------------------------------------------------------------------------*/
int performing_test ()
{
	int	rc =NO_ERROR;

	rc =  non_interactive_tests ();

	/* system true means non-interactive test 	*/

	if (( rc == NO_ERROR) && ( da_input.advanced == ADVANCED_TRUE) 
			&& (( da_input.console == CONSOLE_TRUE)
				|| ( da_input.exenv == EXENV_SYSX ))
			&& ( da_input.system == SYSTEM_FALSE))
		rc = interactive_tests();
	return (rc);
}

/*--------------------------------------------------------------------------
| 	NAME: interactive_tests()
|
| 	FUNCTION: Run interactive tests
|		  
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:   
|
| 	RETURNS:
|		NO_ERROR - test passed
|		otherwise -test failed
|		Same return code as tu
----------------------------------------------------------------------------*/
int interactive_tests()
{
	int 	rc = NO_ERROR;
	int	menu_rc;


	if (da_input.loopmode & LOOPMODE_INLM) 
	{
		da_odm_vars (READ_VARS);


		/* if no wrap plug is installed or transceiver  */
		/* is installed, then do not run any tests		    */

		if ((!wrap_plug) && (!xceiver_10base2)
                                 && (!xceiver_twisted))
			return (NO_ERROR);
		else if (wrap_plug == WRAP_PLUG_BNC)
		{
			/* testing BNC connector 	*/
			rc = bnc_wrap_plug_testing ();
		}
		else
		{
			/* testing DIX connector or transceiver 	*/
			set_timer ();
			rc = running_test_7();
			alarm (0);
 			if((rc != 0) || (alarm_timeout))
			{
			     if (wrap_plug)
		 	     {
					report_frub (FRU_307);
			     }
			     else 
			     {
				  if(xceiver_twisted)
				  {
					report_frub (FRU_403);
				  }
			          else if (xceiver_10base2)
				  {
					report_frub (FRU_402);
				  }
			      }
			}
		}
		return (rc);
	}

	if ((da_input.loopmode & LOOPMODE_NOTLM) ||
		(da_input.loopmode & LOOPMODE_ENTERLM))
        {
		menu_rc = display (WHICH_TYPE_CONNECTOR);
		if (menu_rc != YES)	/* BNC connector */
		{
			rc = bnc_wrap_plug_testing ();
			return (rc);
		}
		menu_rc = display (XCEIVER_EXIST);
		if (menu_rc != YES)
			rc =dix_wrap_plug_testing ();
		else
		{
			menu_rc = display (TEST_XCEIVER);
			if (menu_rc != YES)
			{
				rc =dix_wrap_plug_testing ();
			}
	
			else
			{
				menu_rc = display (WHICH_TYPE_XCEIVER);
				if (menu_rc == 1)	/* 10 Base T Xceiver */
				{
					rc = test_twisted_pair_transceiver ();
				}
				else if ( menu_rc == 2) /* 10 Base 2 Xceiver */
				{
					rc = test_10_base_2_transceiver();
				}
				else
					return (NO_ERROR);
			}
		}
        }
	return (rc);
}

/*--------------------------------------------------------------------------
| 	NAME: non_interactive_test
|
| 	FUNCTION: Run test unit 1 through 6
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|
| 	RETURNS:
|		same return code of exectu()
----------------------------------------------------------------------------*/

int non_interactive_tests ()
{
struct	tu_fru_pair *start;
int	rc;

	eth_tucb.header.mfg = 0;
	eth_tucb.header.loop = 1;

	start = tus_test;  

	/* Make sure that this variable is cleared when running the test */
	/* so that INLOOPMODE will keep tracks of all errors		 */
	PUTVAR(circuit_breaker_bad,DIAG_SHORT, FALSE);
        if (da_input.exenv == EXENV_SYSX
        	&& da_input.loopmode != LOOPMODE_INLM)
        {
		return(NO_ERROR);
        }
	for (; start->tu != -1; ++start) {
		rc = test_tu(start->tu,start->fru);
		if ( rc != NO_ERROR)
			break;
		 if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
			return(QUIT);
	}

	return(rc);
}


/*--------------------------------------------------------------------------
| 	NAME: test_tu
|
| 	FUNCTION: run test unit and report FRU specified by fru_index
|	    for failed test; and if fru_index equals NO_REPORT_FRU (-1) then
|	    no FRU report is needed.
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|	  report_frub()
|
| 	RETURNS:
|		same return code of exectu()
----------------------------------------------------------------------------*/
int test_tu (int test_unit,int fru_index)
{
int rc;

	eth_tucb.header.tu = test_unit;

	set_timer ();
	rc = exectu(filedes, &eth_tucb);
	alarm(0);

        if ((rc  != NO_ERROR) || (alarm_timeout ))
	{
		report_frub (fru_index);
		return (NOT_GOOD);
	}
	return(rc);
}
/*--------------------------------------------------------------------------
| NAME: open_device
|
| FUNCTION: opening the device 
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	errno after open()
----------------------------------------------------------------------------*/
int open_device ()
{
	int	open_error;
	char devname[32];
	

	sprintf(devname,"/dev/%s/D",da_input.dname);

	errno =0;
	/* open device			*/
	set_timer ();
	filedes = open(devname, O_RDWR | O_NDELAY);
	alarm(0);

	open_error = errno;
	
	if (alarm_timeout || filedes == NOT_GOOD)
	{
		DA_SETRC_ERROR (DA_ERROR_OPEN);
		return (open_error);
	}

	return (NO_ERROR);
} /* open_device */



/*......................................................*/
/*     Procedure description : 	Error logging analysis 	*/ 
/*......................................................*/

void error_log_analysis ()
{
        int ela_rc , ela_get_rc;
        char crit[128];
        struct errdata errdata_str;

        /*..........................................................*/
        /* Look for an occurrence of any of the following errors in */
        /* the error log.  If one exists, exit_with_fru for ELA.    */
        /*..........................................................*/
        sprintf(crit,"%s -N %s -j %X,%X",
                  da_input.date, da_input.dname,
                                  ERRID_ENT_ERR3, ERRID_ENT_ERR1);

	/* ERR1 : Indicates a permanent Ethernet adapter error */
        /* ERR3 : Is DMA channel check                  */
        /* ERR4 : Dcomplete does not finish. DMA error  */
        /* ERR5 : Transmit error due to hardware        */

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
        if (ela_get_rc >0)
        {
                switch (errdata_str.err_id)
                {
                        case ERRID_ENT_ERR1:
                        case ERRID_ENT_ERR3:
                                report_frub (FRU_124);
                                break;
                }
        }
        ela_rc = error_log_get(TERMI,crit,&errdata_str);

}


/*--------------------------------------------------------------------------
 | NAME: intr_handler
 |
 | FUNCTION: Perform general clean up on receipt of an interrupt
 |
 | EXECUTION ENVIRONMENT:
 |
 |      This should describe the execution environment for this
 |      procedure. For example, does it execute under a process,
 |      interrupt handler, or both. Can it page fault. How is
 |      it serialized.
 |
 | RETURNS: NONE
 --------------------------------------------------------------------------*/

void intr_handler(int sig)
{
        if ( da_input.console == CONSOLE_TRUE ) 
                diag_asl_clear_screen();
        clean_up();
}

/*--------------------------------------------------------------------------
| NAME: display							   
|									   
| FUNCTION: This function will display different messages
|           used in DA
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/

int display (menu_type)
int	menu_type;
{
	int	rc=0;
	char	*text_array[3];


	text_array[0] = (char *)diag_cat_gets (catd, 1,1);
	text_array[1] = da_input.dname;
	text_array[2] = da_input.dnameloc;

	switch (menu_type)
	{
		case ETHERNET_LOOPMODE:
			diag_display_menu (LOOPMODE_TESTING_MENU, 0x852003,
				text_array, da_input.lcount, da_input.lerrors);

			break;

		case ETHERNET_ADVANCED:
			diag_display_menu (ADVANCED_TESTING_MENU, 0x852002,
				text_array, da_input.lcount, da_input.lerrors);

			break;

		case ETHERNET_CUSTOMER: 
			diag_display_menu (CUSTOMER_TESTING_MENU, 0x852001,
				text_array, da_input.lcount, da_input.lerrors);
			break;

		case ADVANCED_NO_STANDBY_WITH_WAIT:
				diag_msg_nw ( 0x852002, catd, 
		   		1, ADVANCED_NO_STANDBY_WITH_WAIT,da_input.dname,
				da_input.dnameloc); 

			break;

		case UNPLUG_WRAP_GENERIC:
			/* ask the user to unplug wrap from the cable 	*/
			/* and put the cable back into the network	*/
                        rc = diag_display(0x852019,
                                catd, unplug_wrap_generic,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_unplug_wrap_generic);
                        sprintf(msgstr, asi_unplug_wrap_generic[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_unplug_wrap_generic[0].text);
                        asi_unplug_wrap_generic[0].text = msgstr;
                        sprintf(msgstr1, asi_unplug_wrap_generic[1].text,
                                 da_input.dnameloc);
                        free (asi_unplug_wrap_generic[1].text);
                        asi_unplug_wrap_generic[1].text = msgstr1;

                        rc = diag_display(0x852019, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_wrap_generic);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        wrap_plug= FALSE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;
		case WRAP_PLUG_BNC:
		{
			rc = diag_display(0x852008, catd,
                        	have_wrap_plug_bnc, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_have_wrap_plug_bnc);
        		sprintf(msgstr, asi_have_wrap_plug_bnc[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_have_wrap_plug_bnc[0].text);
        		asi_have_wrap_plug_bnc[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852008, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_bnc);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		/* setting wrap_plug variable in data base to true */
        		wrap_plug = TRUE;
        		putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
				&wrap_plug);

        		return (YES);
			break;
		}
		case WRAP_PLUG_DIX:
		{
			rc = diag_display(0x852009, catd,
                        	have_wrap_plug_dix, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_have_wrap_plug_dix);
        		sprintf(msgstr, asi_have_wrap_plug_dix[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_have_wrap_plug_dix[0].text);
        		asi_have_wrap_plug_dix[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852009, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_dix);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		/* setting wrap_plug variable in data base to true */
        		wrap_plug = TRUE;
        		putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
				&wrap_plug);

        		return (YES);
			break;
		}
		case WRAP_PLUG_TWISTED:
		{
			rc = diag_display(0x852010, catd,
                        	have_wrap_plug_twisted, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_have_wrap_plug_twisted);
        		sprintf(msgstr, asi_have_wrap_plug_twisted[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_have_wrap_plug_twisted[0].text);
        		asi_have_wrap_plug_twisted[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852010, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_twisted);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		/* setting wrap_plug variable in data base to true */
        		wrap_plug = TRUE;
        		putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
				&wrap_plug);

        		return (YES);
			break;
		}

		case PLUG_DIX:
		{
			/* ask the user to plug the wrap plug in	*/
                        rc = diag_display(0x852018,
                                catd, plug_dix,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_dix);
                        sprintf(msgstr, asi_plug_dix[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_plug_dix[0].text);
                        asi_plug_dix[0].text = msgstr;
                        sprintf(msgstr1, asi_plug_dix[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_plug_dix[1].text);
                        asi_plug_dix[1].text = msgstr1;

                        rc = diag_display(0x852018, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_dix);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        wrap_plug= WRAP_PLUG_DIX;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;
		}
		case STIL_ETH_PLUG_BNC:
		{
			/* ask the user to plug the wrap plug in	*/
                        rc = diag_display(0x852018,
                                catd, plug_bnc_connector,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_bnc_connector);
                        sprintf(msgstr, asi_plug_bnc_connector[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_plug_bnc_connector[0].text);
                        asi_plug_bnc_connector[0].text = msgstr;
                        sprintf(msgstr1, asi_plug_bnc_connector[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_plug_bnc_connector[1].text);
                        asi_plug_bnc_connector[1].text = msgstr1;

                        rc = diag_display(0x852018, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_bnc_connector);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        wrap_plug= WRAP_PLUG_BNC;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;
		}
		case XCEIVER_EXIST:
		{
			rc = diag_display(0x852004, catd,
                        	xceiver_exist, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_xceiver_exist);
        		sprintf(msgstr, asi_xceiver_exist[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_xceiver_exist[0].text);
        		asi_xceiver_exist[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852004, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_xceiver_exist);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		return (YES);
			break;
		}
		case TEST_XCEIVER:
		{
			rc = diag_display(0x852005, catd,
                        	test_xceiver, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_test_xceiver);
        		sprintf(msgstr, asi_test_xceiver[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_test_xceiver[0].text);
        		asi_test_xceiver[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852005, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_test_xceiver);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		return (YES);
			break;
		}
		case WHICH_TYPE_CONNECTOR:
		{
			rc = diag_display(0x852004, catd,
                        	which_type_connector, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_which_type_connector);
        		sprintf(msgstr, asi_which_type_connector[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_which_type_connector[0].text);
        		asi_which_type_connector[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852004, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_which_type_connector);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}
        		return (YES);
			break;
		}
		case WHICH_TYPE_XCEIVER:
		{
			rc = diag_display(0x852006, catd,
                        	what_type_xceiver, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_what_type_xceiver);
        		sprintf(msgstr, asi_what_type_xceiver[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_what_type_xceiver[0].text);
        		asi_what_type_xceiver[0].text = msgstr;


        		/* set the cursor points to type not known */
        		menutypes.cur_index = 3;
        		rc = diag_display(0x852006, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_what_type_xceiver);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
			if ( rc == 3)		
			{
				clean_up();
			}
			break;
		}
		case PLUG_XCEIVER_T:
		{
		      /* ask the user to put the twisted pair transceiver */
                      rc = diag_display(0x852011,
                                catd, plug_xceiver_twisted,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_xceiver_twisted);
                      sprintf(msgstr,asi_plug_xceiver_twisted[0].text,
                                da_input.dname, da_input.dnameloc);
                      free (asi_plug_xceiver_twisted[0].text);
                      asi_plug_xceiver_twisted[0].text = msgstr;
                      sprintf(msgstr1,asi_plug_xceiver_twisted[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                      free (asi_plug_xceiver_twisted[1].text);
                      asi_plug_xceiver_twisted[1].text = msgstr1;

                      rc = diag_display(0x852011, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_xceiver_twisted);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case ISOLATE_XCEIVER:
		{
			rc = diag_display(0x852012, catd,
                        	isolate_xceiver, DIAG_MSGONLY,
                        	ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        	asi_isolate_xceiver);
        		sprintf(msgstr, asi_isolate_xceiver[0].text,
                		da_input.dname, da_input.dnameloc);
        		free (asi_isolate_xceiver[0].text);
        		asi_isolate_xceiver[0].text = msgstr;


        		/* set the cursor points to NO */
        		menutypes.cur_index = NO;
        		rc = diag_display(0x852012, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_isolate_xceiver);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

        		rc = DIAG_ITEM_SELECTED (menutypes);
        		if      /* if user does not have the wrap NTF */
                		( rc != YES)
        		{
                		return (NO);
        		}

        		return (YES);
			break;
		}
		case XCEIVER_T_2_PHASE:
		{
		      /* ask the user to unplug wrap and turn OFF link test */
		      /* switch for second phase of testing		    */
                      rc = diag_display(0x852014,
                                catd, xceiver_t_2_phase,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_xceiver_t_2_phase);
                      sprintf(msgstr,asi_xceiver_t_2_phase[0].text,
                                da_input.dname, da_input.dnameloc);
                      free (asi_xceiver_t_2_phase[0].text);
                      asi_xceiver_t_2_phase[0].text = msgstr;
                      sprintf(msgstr1,asi_xceiver_t_2_phase[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                      free (asi_xceiver_t_2_phase[1].text);
                      asi_xceiver_t_2_phase[1].text = msgstr1;

                      rc = diag_display(0x852014, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_xceiver_t_2_phase);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case UNPLUG_WRAP_CABLE_FOR_ISOLATION:
		{
		    /* ask the user to unplug wrap plug or cable for 	*/
		    /* isolation for circuit breaker			*/
                    rc = diag_display(0x852015,
                                catd, unplug_wrap_cable_for_isolation,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes,asi_unplug_wrap_cable_for_isolation);
                    sprintf(msgstr,asi_unplug_wrap_cable_for_isolation[0].text,
                                da_input.dname, da_input.dnameloc);
                    free (asi_unplug_wrap_cable_for_isolation[0].text);
                    asi_unplug_wrap_cable_for_isolation[0].text = msgstr;
                    sprintf(msgstr1,asi_unplug_wrap_cable_for_isolation[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                    free (asi_unplug_wrap_cable_for_isolation[1].text);
                    asi_unplug_wrap_cable_for_isolation[1].text = msgstr1;

                    rc = diag_display(0x852015, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_wrap_cable_for_isolation);

                    if ((check_asl_stat (rc)) == QUIT) 
		    {
			da_odm_vars(INITIALIZE_VARS);
			clean_up();
		    }
                    break;
		}
		case PLUG_XCEIVER_10BASE_2:
		{
		      /* ask the user to plug 10 base 2 for testing */
                      rc = diag_display(0x852016,
                                catd, plug_xceiver_10base_2,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_xceiver_10base_2);
                      sprintf(msgstr,asi_plug_xceiver_10base_2[0].text,
                                da_input.dname, da_input.dnameloc);
                      free (asi_plug_xceiver_10base_2[0].text);
                      asi_plug_xceiver_10base_2[0].text = msgstr;
                      sprintf(msgstr1,asi_plug_xceiver_10base_2[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                      free (asi_plug_xceiver_10base_2[1].text);
                      asi_plug_xceiver_10base_2[1].text = msgstr1;

                      rc = diag_display(0x852016, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_xceiver_10base_2);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case UNPLUG_DIX_AND_PUT_BASE2:
		{
                      rc = diag_display(0x852020,
                                catd, unplug_dix_and_put_base2,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_unplug_dix_and_put_base2);
                      sprintf(msgstr,asi_unplug_dix_and_put_base2[0].text,
                                da_input.dname, da_input.dnameloc);
                      free (asi_unplug_dix_and_put_base2[0].text);
                      asi_unplug_dix_and_put_base2[0].text = msgstr;
                      sprintf(msgstr1,asi_unplug_dix_and_put_base2[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                      free (asi_unplug_dix_and_put_base2[1].text);
                      asi_unplug_dix_and_put_base2[1].text = msgstr1;

                      rc = diag_display(0x852020, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_dix_and_put_base2);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case PLUG_T_XCEIVER_BACK:
		{
                      rc = diag_display(0x852022,
                                catd, plug_t_xceiver_back,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, 
			        asi_plug_t_xceiver_back);
                      sprintf(msgstr,
		          asi_plug_t_xceiver_back[0].text,
                          da_input.dname, da_input.dnameloc);
                      free (asi_plug_t_xceiver_back[0].text);
                      asi_plug_t_xceiver_back[0].text = msgstr;
                      sprintf(msgstr1,
		              asi_plug_t_xceiver_back[1].text,
                              da_input.dnameloc,da_input.dnameloc);
                      free (asi_plug_t_xceiver_back[1].text);
                      asi_plug_t_xceiver_back[1].text= msgstr1;

                      rc = diag_display(0x852022, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_t_xceiver_back);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case UNPLUG_XCEIVER_AND_PLUG_DIX:
		{
                      rc = diag_display(0x852024,
                                catd, unplug_xceiver_and_plug_dix,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, 
			        asi_unplug_xceiver_and_plug_dix);
                      sprintf(msgstr,
		          asi_unplug_xceiver_and_plug_dix[0].text,
                          da_input.dname, da_input.dnameloc);
                      free (asi_unplug_xceiver_and_plug_dix[0].text);
                      asi_unplug_xceiver_and_plug_dix[0].text = msgstr;
                      sprintf(msgstr1,
		              asi_unplug_xceiver_and_plug_dix[1].text,
                              da_input.dnameloc,da_input.dnameloc);
                      free (asi_unplug_xceiver_and_plug_dix[1].text);
                      asi_unplug_xceiver_and_plug_dix[1].text= msgstr1;

                      rc = diag_display(0x852024, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_xceiver_and_plug_dix);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}

		case UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_BASE2:
		{
                      rc = diag_display(0x852026,
                                catd, unplug_dix_wrap_and_put_xceiver_base2,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, 
			        asi_unplug_dix_wrap_and_put_xceiver_base2);
                      sprintf(msgstr,
		          asi_unplug_dix_wrap_and_put_xceiver_base2[0].text,
                          da_input.dname, da_input.dnameloc);
                      free (asi_unplug_dix_wrap_and_put_xceiver_base2[0].text);
                      asi_unplug_dix_wrap_and_put_xceiver_base2[0].text =msgstr;
                      sprintf(msgstr1,
		              asi_unplug_dix_wrap_and_put_xceiver_base2[1].text,
                              da_input.dnameloc,da_input.dnameloc);
                      free (asi_unplug_dix_wrap_and_put_xceiver_base2[1].text);
                      asi_unplug_dix_wrap_and_put_xceiver_base2[1].text= msgstr1;

                      rc = diag_display(0x852026, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_dix_wrap_and_put_xceiver_base2);

                      if ((check_asl_stat (rc)) == QUIT) 
		      {
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
		      }
                      break;
		}
		case UNPLUG_DIX_WRAP_AND_PUT_XCEIVER_TWISTED:
		{
                    rc = diag_display(0x852027,
                                catd, unplug_dix_wrap_and_put_xceiver_twisted,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, 
			        asi_unplug_dix_wrap_and_put_xceiver_twisted);
                    sprintf(msgstr,
		          asi_unplug_dix_wrap_and_put_xceiver_twisted[0].text,
                          da_input.dname, da_input.dnameloc);
                    free(asi_unplug_dix_wrap_and_put_xceiver_twisted[0].text);
                    asi_unplug_dix_wrap_and_put_xceiver_twisted[0].text =msgstr;
                    sprintf(msgstr1,
		            asi_unplug_dix_wrap_and_put_xceiver_twisted[1].text,
                              da_input.dnameloc,da_input.dnameloc);
                    free (asi_unplug_dix_wrap_and_put_xceiver_twisted[1].text);
                    asi_unplug_dix_wrap_and_put_xceiver_twisted[1].text=msgstr1;

                    rc = diag_display(0x852027, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_dix_wrap_and_put_xceiver_twisted);

                    if ((check_asl_stat (rc)) == QUIT) 
		    {
			da_odm_vars(INITIALIZE_VARS);
			clean_up();
		    }
                      break;
		}
		case PLUG_NETWORK_CABLE_ONLY:
		{
			/* ask the user the plug the network cable only back */
                        rc = diag_display(0x852030,
                                catd, plug_network_cable_only,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_network_cable_only);
                        sprintf(msgstr, asi_plug_network_cable_only[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_plug_network_cable_only[0].text);
                        asi_plug_network_cable_only[0].text = msgstr;
                        sprintf(msgstr1, asi_plug_network_cable_only[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_plug_network_cable_only[1].text);
                        asi_plug_network_cable_only[1].text = msgstr1;

                        rc = diag_display(0x852030, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_network_cable_only);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				da_odm_vars(INITIALIZE_VARS);
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        wrap_plug= TRUE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;
		}
	}
	return (rc);
}

/*--------------------------------------------------------------------------
| NAME: report_frub
|
| FUNCTION: The function will add fru bucket into database
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/
void report_frub (frub_idx)
int frub_idx;
{
struct fru_bucket *frub_addr = &eth_frus[frub_idx];


        /* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);

        if (insert_frub(&da_input, frub_addr) != NO_ERROR ||
            addfrub(frub_addr) != NO_ERROR) 
        {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
	else
		DA_SETRC_STATUS (DA_STATUS_BAD);
}

/*--------------------------------------------------------------------------
 | NAME: check_asl_stat
 |
 | FUNCTION: Check asl return code
 |
 | EXECUTION ENVIRONMENT:
 |
 | NOTES:  Other local routines called --
 |
 | RETURNS:
 ---------------------------------------------------------------------------*/
int check_asl_stat (int rc)
{
				
	int	return_code =0;
	switch ( rc)
	{
		case ASL_CANCEL:
			DA_SETRC_USER (DA_USER_QUIT);
			return_code = QUIT;
			break;

		case ASL_EXIT:
			DA_SETRC_USER (DA_USER_EXIT);
			return_code = QUIT;
			break;
		default:
			break;
		
	}
	return (return_code);
} /* check_asl_stat 	*/

/*--------------------------------------------------------------------------
 | NAME: poll_kb_for_quit
 |
 | FUNCTION: Poll the keyboard input to see whether user want to
 |	    quit
 |
 | EXECUTION ENVIRONMENT:
 |
 | NOTES:  Other local routines called --
 |	check_asl_stat()
 |
 | RETURNS:
 |	QUIT ---- user entered CANCEL,EXIT
 |	NO_ERROR -otherwise
 --------------------------------------------------------------------------*/
int poll_kb_for_quit()
{
	int rc;


	if (check_asl_stat( diag_asl_read
		(ASL_DIAG_LIST_CANCEL_EXIT_SC, FALSE, NULL))==QUIT) 
		return(QUIT);
	else 
		return(NO_ERROR);
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

/* --------------------------------------------------------------------------
 * NAME: set_timer ()
 *
 * FUNCTION: Designed to set up alarm timer
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 ----------------------------------------------------------------------- */

void set_timer()
{
        invec.sa_handler = timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(60);
}


/*--------------------------------------------------------------------------
| NAME: clean_up
|
| FUNCTION: Cleanup before exit DA ; closing catalog file ;
|	    quiting asl; restore the database to the initial 
|	    state
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/

void clean_up ()
{
        int     rc = NO_ERROR;


        if (filedes != NOT_GOOD)
                close (filedes);

	/*  Unload the diagnostics device driver and set the lpp device */
	/*  to original  state						*/
	configure_lpp_device();



        /* terminate odm */
        term_dgodm();

        if ( da_input.console == TRUE )
        {
		if (da_input.exenv == EXENV_SYSX) clrdainput();
                if (catd != CATD_ERR)
                        catclose (catd);
                if (asl_flg != NOT_GOOD)
                        diag_asl_quit (NULL);

        }
        clean_malloc();

        DA_EXIT ();

}
/*--------------------------------------------------------------------------
| NAME: general_initialize ()
|
| FUNCTION:  This routine will general initialization of  variables
|	     used in DA 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None.
|       If there is an error, exit without testing
----------------------------------------------------------------------------*/
void general_initialize()
{
	int	rc;


	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);
	DA_SETRC_USER (DA_USER_NOKEY);

	setlocale(LC_ALL,"");

	/* initializing interrupt handler */    
	invec.sa_handler = intr_handler;
	sigaction(SIGINT, &invec, (struct sigaction *)NULL);
	invec.sa_handler = timeout;
	sigaction( SIGALRM, &invec, (struct sigaction *) NULL );

        msgstr = (char *) malloc (1200 *sizeof (char));
        msgstr1 = (char *) malloc (1200 *sizeof (char));
	if((msgstr == (char *) NULL) || (msgstr1 == (char *) NULL)) 
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}
	/* initialize odm	*/
	odm_flg = init_dgodm();

	/* get TMInput object from database	*/
	rc = getdainput (&da_input);
	if (odm_flg == -1 || rc != 0) 
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}

	if 	/* running under console */
	   ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl  & open catalog	*/
		if (da_input.loopmode == LOOPMODE_INLM)
			asl_flg = diag_asl_init (ASL_INIT_DEFAULT);
		else
			asl_flg = diag_asl_init ("NO_TYPE_AHEAD");
		catd = diag_catopen (MF_DENET,0); 
		if ( (asl_flg == NOT_GOOD) || (catd == CATD_ERR ))
			clean_up();
	}
	if ( da_input.loopmode & LOOPMODE_EXITLM )
		da_odm_vars (READ_VARS);
	else if ((da_input.loopmode & LOOPMODE_NOTLM) ||
		(da_input.loopmode & LOOPMODE_ENTERLM))
		da_odm_vars (INITIALIZE_VARS);

}
/*---------------------------------------------------------------------------
| NAME: clean_malloc ()
|
| FUNCTION:  This message will free all the memory allocation by
|            general_intialization
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None.
|       If there is an error, exit without testing
----------------------------------------------------------------------------*/
void clean_malloc ()
{
        if(msgstr != (char *) NULL)
                free (msgstr);
        if(msgstr1 != (char *) NULL)
                free (msgstr1);
}
/****************************************************************
* NAME: da_odm_vars 
*
* FUNCTION: Initialize or read the necessary davars
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*       if op is SETH_INIT_OP then initialize davars
*       else if op is SETH_GET_OP then read davars
*
* RETURNS:
*       None
****************************************************************/
void da_odm_vars(int op)
{
        switch(op) {
        case INITIALIZE_VARS:

                PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
                PUTVAR(xceiver_10base2,DIAG_SHORT,FALSE);
                PUTVAR(xceiver_twisted,DIAG_SHORT,FALSE);
                PUTVAR(isolate_xceiver_10base2_process,DIAG_SHORT,FALSE);
                PUTVAR(isolate_xceiver_twisted_process,DIAG_SHORT,FALSE);
                PUTVAR(circuit_breaker_bad,DIAG_SHORT,FALSE);
                PUTVAR(network_cable_only,DIAG_SHORT,FALSE);

                break;
        case READ_VARS:
                getdavar(da_input.dname, "wrap_plug", DIAG_SHORT, &wrap_plug);
                getdavar(da_input.dname, "xceiver_10base2", 
			DIAG_SHORT, &xceiver_10base2);
                getdavar(da_input.dname, "xceiver_twisted", 
			DIAG_SHORT, &xceiver_twisted);
                getdavar(da_input.dname, "isolate_xceiver_10base2_process", 
			DIAG_SHORT, &isolate_xceiver_10base2_process);
                getdavar(da_input.dname, "isolate_xceiver_twisted_process", 
			DIAG_SHORT, &isolate_xceiver_twisted_process);
                getdavar(da_input.dname, "circuit_breaker_bad", 
			DIAG_SHORT, &circuit_breaker_bad);
                getdavar(da_input.dname, "network_cable_only", 
			DIAG_SHORT, &network_cable_only);
		break;
	}
}

/*---------------------------------------------------------------------------
| NAME:  unplug_xceiver_and_plug_dix_wrap()
|
| FUNCTION: This procedure is called when the wrap test for xceiver fails
|           The user is asked if he does has the wrap plug for further 
|           testing. If he has, DIX wrap plug is tested. Else a FRU
|	    is reported 
|            
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None.
|       If there is an error, exit without testing
----------------------------------------------------------------------------*/
int unplug_xceiver_and_plug_dix_wrap()
{
	int	menu_rc, rc;
	/* ask if the user has DIX wrap plug	*/
	/* menu 852-012 */

	rc = display (ISOLATE_XCEIVER);
	if (rc != YES)
	{
		/* Report 80 % Transceiver , 20 % Ethernet adapter 	*/
		if (xceiver_twisted)
			report_frub (FRU_403);
		else
			report_frub (FRU_402);
		return (ETH_ERROR);
	}
	
	/* ask the user to unplug Converter and put DIX wrap plug in	*/
	/* menu 852-024 	*/

	display (UNPLUG_XCEIVER_AND_PLUG_DIX);
	
	da_display();
	set_timer ();
	rc = running_test_7();
	alarm(0);
 	if((rc != 0) || (alarm_timeout))
	{
		report_frub (FRU_307);
	}
	else	/* the transceiver is bad	*/ 
	{
		if (xceiver_twisted)
			report_frub (FRU_121);
		else
			report_frub (FRU_122);
	}

	/* updating ODM for status of wrap , xceiver   		*/

	if (xceiver_twisted)
	{
		PUTVAR(isolate_xceiver_twisted_process,DIAG_SHORT,TRUE);
		PUTVAR(isolate_xceiver_10base2_process,DIAG_SHORT,FALSE);
	}
	else if (xceiver_10base2)
	{
		PUTVAR(isolate_xceiver_10base2_process,DIAG_SHORT,TRUE);
		PUTVAR(isolate_xceiver_twisted_process,DIAG_SHORT,FALSE);
	}
	PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
	PUTVAR(xceiver_twisted,DIAG_SHORT,FALSE);
	PUTVAR(xceiver_10base2,DIAG_SHORT,FALSE);

	return (ETH_ERROR);
}

/*---------------------------------------------------------------------------
| NAME: dix_wrap_plug_testing()
|
| FUNCTION: Ask the user to put DIX the wrap plug in and testing 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 		bad : ETH_ERROR
|               good: NO_ERROR
----------------------------------------------------------------------------*/
int dix_wrap_plug_testing ()
{
	int rc, menu_rc;

	menu_rc = display (WRAP_PLUG_DIX);
	if (menu_rc != YES)
	{
		wrap_plug = FALSE;
                PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
		return (NO_ERROR);
	}
	display (PLUG_DIX);
	PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
	da_display();

	set_timer ();
	rc = running_test_7();
	alarm(0);
 	if((rc != 0) || (alarm_timeout))
	{
		report_frub (FRU_307);
		return (ETH_ERROR);
	}
	return (rc);
}

/*---------------------------------------------------------------------------
| NAME: bnc_wrap_plug_testing()
|
| FUNCTION: Ask the user to put BNC  wrap plug in and testing 
|
|	This test is run according to the TU Specs recommendation
|	
|	Test Unit		Frame size
|	8			1499
|	9			1489
|	10			1479
|	11			1469
|	11			1459
|	11			1449
|	11			1439
|	11			1429
|	11			1419
|	11			1409
|	11			1399
|	11			1389
|	11			1379
|	11			1369
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 		bad : ETH_ERROR
|               good: NO_ERROR
----------------------------------------------------------------------------*/
int bnc_wrap_plug_testing ()
{
	int rc, menu_rc, i, j;
 
	if ( da_input.loopmode != LOOPMODE_INLM) 
	{
		menu_rc = display (WRAP_PLUG_BNC);
		if (menu_rc != YES)
		{
			wrap_plug = FALSE;
                	PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
			return (NO_ERROR);
		}
		display (STIL_ETH_PLUG_BNC);
		da_display();
	}

	set_timer ();
	eth_tucb.header.loop = 1;
	eth_tucb.header.tu = 8;
	eth_tucb.header.r1 = 1499;
	rc = exectu(filedes, &eth_tucb);
	alarm(0);
	if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
		return(QUIT);
 	if((rc != 0) || (alarm_timeout))
	{
		report_frub (FRU_307);
		return (ETH_ERROR);
	}

	set_timer ();
	eth_tucb.header.tu = 9;
	eth_tucb.header.r1 = 1489;
	rc = exectu(filedes, &eth_tucb);
	alarm(0);
	if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
		return(QUIT);
 	if((rc != 0) || (alarm_timeout))
	{
		report_frub (FRU_307);
		return (ETH_ERROR);
	}

	set_timer ();
	eth_tucb.header.tu = 10;
	eth_tucb.header.r1 = 1479;
	rc = exectu(filedes, &eth_tucb);
	alarm(0);
	if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
		return(QUIT);
 	if((rc != 0) || (alarm_timeout))
	{
		report_frub (FRU_307);
		return (ETH_ERROR);
	}

	j = 1469;
	for ( i=1; i<=11; i++)
	{
		set_timer ();
		eth_tucb.header.tu = 11;
		eth_tucb.header.r1 = j;
		rc = exectu(filedes, &eth_tucb);
		alarm(0);
		if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
			break;
 		if((rc != 0)|| (alarm_timeout))
		{
			report_frub (FRU_307);
			break;
		}
		j -= 10;

	}


	return (rc);
}
/*---------------------------------------------------------------------------
| NAME: test_twisted_pair_transceiver () 
|
| FUNCTION: 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 		bad : ETH_ERROR
|               good: NO_ERROR
----------------------------------------------------------------------------*/
int test_twisted_pair_transceiver ()
{
	int 	rc,menu_rc;


	display (PLUG_XCEIVER_T);
	da_display ();
	PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
	PUTVAR(xceiver_10base2,DIAG_SHORT,FALSE);
	PUTVAR(xceiver_twisted,DIAG_SHORT,TRUE);

	set_timer ();
	rc = running_test_7();
	alarm(0);
 	if (alarm_timeout || (rc != 0 ) )
	{
		unplug_transceiver ();
	}
	return (rc);
} 

/*---------------------------------------------------------------------------
| NAME: running_test_7 () 
|
| FUNCTION: This procedure will call exectu to run test unit 7 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 		bad : ETH_ERROR
|               good: NO_ERROR
----------------------------------------------------------------------------*/
int running_test_7 ()
{
	int	rc;

        if (da_input.exenv == EXENV_SYSX
        	&& da_input.loopmode != LOOPMODE_INLM)
        {
		return(NO_ERROR);
        }
	eth_tucb.header.tu = 7;
	rc = exectu(filedes, &eth_tucb);
	return (rc);
}
/*---------------------------------------------------------------------------
| NAME: unplug_transceiver () 
|
| FUNCTION: This procedure will ask the user to unplug transceiver 
|	    and put DIX wrap plug for problem isolation 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 	 ETH_ERROR	
|               
----------------------------------------------------------------------------*/
int unplug_transceiver ()
{
	da_odm_vars (READ_VARS);
	if ((xceiver_10base2) || (xceiver_twisted))
	{
		 unplug_xceiver_and_plug_dix_wrap();
		
	}
	return (ETH_ERROR);
}
/*---------------------------------------------------------------------------
| NAME: test_10_base_2_transceiver() 
|
| FUNCTION: 
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 		bad : ETH_ERROR
|               good: NO_ERROR
----------------------------------------------------------------------------*/
int test_10_base_2_transceiver()
{
	int 	rc,menu_rc;

	/* menu 852-008	*/
	rc = display (WRAP_PLUG_BNC);
	if (rc != YES)
		return (NO_ERROR);
	/* menu 852-016 */
	display (PLUG_XCEIVER_10BASE_2);
	da_display ();
	PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
	PUTVAR(xceiver_10base2,DIAG_SHORT,TRUE);
	PUTVAR(xceiver_twisted,DIAG_SHORT,FALSE);

	set_timer ();
	rc = running_test_7();
	alarm(0);
 	if ((alarm_timeout) || (rc != 0 ))
	{
		unplug_transceiver ();
	}
	else
		return (NO_ERROR);
	return (rc);
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
 *      Environment must be a diagnostics environment wich is described
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
		strcpy (criteria,"/usr/lib/methods/cfgddent");
		result = invoke_method(criteria,option, &new_out,&new_err);
	        free(new_out);
                free(new_err);

		if (result !=  NO_ERROR)
		{
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
				DIAG_DEVICE_CANNOT_UNCONFIGURED), 
                                da_input.dname, da_input.dnameloc);
			menugoal (msgstr);
		}
	}

	/* Setting the device state to original state */
        if(diag_device_diagnose) {
                result = diagex_initial_state (da_input.dname);
                if (result != NO_ERROR)
                        {
                        sprintf(msgstr, (char *)diag_cat_gets (
                                catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                da_input.dname, da_input.dnameloc);
                        menugoal (msgstr);
                        }
        }

}
/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device 				*/
/*      Description:    					*/
/*	Return		: 0 Good 				*/
/*			  -1 BAD				*/
/*--------------------------------------------------------------*/
int	unconfigure_lpp_device()
{
        char    criteria[128];
	int	result;

	result = diagex_cfg_state ( da_input.dname );

        switch (result) {
                case 2:
                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                        clean_up();
                        break;
                case 3: sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
                                da_input.dname, da_input.dnameloc);
                        menugoal (msgstr);
                        clean_up();
                        break;
        }
        diag_device_diagnose=TRUE;

	/* The diagnostics device driver needs to be loaded into the system */

	/* CFG is defined as 0	*/
	sprintf (option," -l %s -f 0", da_input.dname);
	strcpy (criteria,"/usr/lib/methods/cfgddent");
	result = invoke_method(criteria,option, &new_out,&new_err);
  	free(new_out);
        free(new_err);
	if (result == NO_ERROR)
	{
		diag_device_configured=TRUE;
	}
	else
	{
		/* report frub */
		report_frub (FRU_117);
		DA_SETRC_STATUS (DA_STATUS_BAD);
		clean_up();
	}

	return (0);
}

