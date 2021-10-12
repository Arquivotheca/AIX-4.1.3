static char sccsid[] = "@(#)46	1.24  src/bos/diag/da/ethstw/dstileth.c, daethstw, bos412, 9443A412b 10/25/94 14:49:27";
/*
 * COMPONENT_NAME: (DAETHSTW)  	Stilwel Ethnet Diagnostic Application
 *
 * FUNCTIONS: 	main, performing_test, interactive_tests,non_interactive_tests
 *		seth_da_display, open_device, clean_up, error_log_analysis
 *		test_tu, intr_handler, poll_kb_for_quit
 *		test_display, disp_menu, 
 *		report_frub, check_asl_stat
 *		 
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include "ient_comio_errids.h" 
#include "diag/tm_input.h"
#include <asl.h>
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/bit_def.h"
#include "dstileth_msg.h"
#include "tu_type.h"
#include "stileth.h"
#include <locale.h>

extern	diag_exec_source();
extern	invoke_method();
extern	nl_catd diag_catopen();
extern 	getdainput();		/* gets input from ODM 			*/
extern 	long getdamode();	/* transforms input into a mode 	*/
extern  int exectu (int,TUTYPE *);
extern 	int errno;		/* reason device driver won't open 	*/

/* davar used */
short 	transceiver_test = FALSE; /* no transceiver test */
short	transceiver_type;
short   wrap_plug = FALSE;	/* wrap plug is not on is assumed	*/
short   query_wrap_plug = FALSE;/* et to TRUE when user is asked to plug*/
int	riser_card;		/* riser card type			*/
volatile int	alarm_timeout=FALSE;
struct  sigaction  invec;          /* interrupt handler structure  */	

short	odm_flg = 0;	/* return code from init_dgodm() */
short	asl_flg = ERROR; /* return code from diag_asl_init() */

int	prior_state = NOT_CONFIG;/* state prior to DA			*/
struct 	tm_input da_input;	/* initialized by getdainput 		*/
int 	filedes=ERROR;		/* file descriptor from device driver 	*/
nl_catd catd;			/* pointer to message catalog  		*/
char	dname[NAMESIZE]; 	/* device name to be tested 		*/
char	*devname;		/* name of device to be tested 		*/
TUTYPE stileth_tucb;

char	option[256];
char	new_out[256], new_err[256];
char	*lpp_configure_method;
char	*lpp_unconfigure_method;
int	unconfigure_lpp=FALSE;
int	diag_device_configured=FALSE;
int	set_device_to_diagnose = FALSE;
struct 	PdDv	*pddv_p;
struct 	CuDv 	*cudv_p; 

#define PUTVAR(A,T,V)	A = V; \
			putdavar(da_input.dname,"A",T,&A)

struct tu_fru_pair {
	short	tu; /* test unit */
	short	fru; /* index into seth_frus[] */
} twisted_pair_NWP_tests[] = {
	{ 0x01, FRU_101 },
	{ 0x02, FRU_102 },
	{ 0x03, FRU_103 },
	{ 0x04, FRU_104 },
	{ 0x06, FRU_123 },
	{ 0x07, FRU_112 },
	{ 0x0D, FRU_113 },
	{ 0x0E, FRU_114 },
	{ 0x0F, FRU_115 },
	{ -1, -1 }
},  twisted_pair_WP_tests[] = {
	{ 0x05, FRU_105 },
	{ -1, -1 }
 }, thick_and_thin_NWP_tests[] = {
	{ 0x01, FRU_101 },
	{ 0x02, FRU_102 },
	{ 0x03, FRU_103 },
	{ 0x04, FRU_104 },
	{ 0x06, FRU_106 },
	{ -1, -1 }
 }, thick_and_thin_WP_tests[] = {
	{ 0x0A, FRU_109 },
	{ 0x0B, FRU_110 },
	{ 0x0C, FRU_111 },
	{ 0x05, FRU_105 },
	{ -1, -1 }
 };

void seth_davar_op(int);
int performing_test();
int interactive_tests();
int non_interactive_tests(int);
int test_tu(int,int);
int verify_rc(int,int);
int open_device();
int poll_kb_for_quit();
int  riser_card_determination();
int advanced_twisted_pair_tests();
int advanced_thick_tests();
int advanced_thin_tests();
int Having_transceiver();
int Having_wrap_plug(int);
int check_asl_stat(int);
int disp_menu(int);
void intr_handler(int);	/* da interrupt handler 		*/
void error_log_analysis ();
void test_display();
void seth_da_display();
void report_frub();
void clean_up();
void timeout(int);
void general_initialization ();
int test_twisted_pair_transceiver ();
int unplug_transceiver_and_plug_dix ();
void unplug_if_removed();
void clean_malloc();

/*--------------------------------------------------------------*/
/*	Messages and menus					*/
/*--------------------------------------------------------------*/

static ASL_SCR_TYPE	menutypes=DM_TYPE_DEFAULTS;
char msgstr[512];

/*......................................................................*/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the  Stilwel Ethernet Adapter Diagnostics
 *		Application.
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

main (argc,argv)
int argc;
char **argv;
{
	int	rc=0;			/* return code 			*/
	int	diskette_mode;
	int	diskette_based;
	int	diag_source;
	char	mountcdrfs[128];
	struct	stat stat_buf;		/* stat buffer defined in stat.h */

	general_initialization ();


	/* if running in LOOPMODE_EXITLM then 		*/
	/* check if the cable is hooked before running  */
	/* DA. If yes, then ask the user to put the   	*/
	/* cable back into the port.			*/

	if ( da_input.loopmode & LOOPMODE_EXITLM )
	{
		seth_davar_op (SETH_GET_OP);
		unplug_if_removed();
		clean_up ();
	}	/* LOOPMODE_EXITLM	*/
	else if ( da_input.dmode & (DMODE_PD | DMODE_REPAIR) )
	        /* executing test if running problem determination	*/
	        /* or repair mode					*/
	{
		if  ( da_input.console == CONSOLE_TRUE) 
			seth_da_display ();
	
		/* check if diskette package is running */
		diag_source = diag_exec_source(&mountcdrfs);
  		diskette_mode = ipl_mode(&diskette_based);
               	if ( (diskette_based == DIAG_FALSE) &&
		     (diag_source == 0) )
		{
			rc = stat ("/usr/lib/drivers/ethdiag", &stat_buf);
			if /* file does not exist */
			   (( rc == ERROR) 
				|| ( stat_buf.st_size == 0 ))
			{

				/* just send menugoal one time only */ 

				if  ((da_input.console == CONSOLE_TRUE) &&
				     (da_input.loopmode & ( LOOPMODE_ENTERLM |
				      LOOPMODE_NOTLM)))
				{
					/* displaying the device driver */
					/* file error 			*/
					sprintf (msgstr, (char *)diag_cat_gets (
						catd, 1,DD_FILE_BAD),
						NULL);
					menugoal (msgstr);
				}
				DA_SETRC_STATUS (DA_STATUS_BAD);
				clean_up();
			}
			rc = stat ("/usr/lib/methods/cfgddeth", &stat_buf);
			if /* file does not exist */
			   (( rc == ERROR) 
				|| ( stat_buf.st_size == 0 ))
			{

				/* just send menugoal one time only */ 

				if  ((da_input.console == CONSOLE_TRUE) &&
				     (da_input.loopmode & ( LOOPMODE_ENTERLM |
				      LOOPMODE_NOTLM)))
				{
					/* displaying configuration method */
					/* Missing or not accessbible	   */
					sprintf (msgstr, (char *)diag_cat_gets (
						catd, 1,CONFIG_FILE_BAD),
						NULL);
					menugoal (msgstr);
				}
				DA_SETRC_STATUS (DA_STATUS_BAD);
				clean_up();
			}
		}
                prior_state=get_device_status(da_input.dname);
		if (prior_state == NOT_CONFIG)
			clean_up();
                rc = unconfigure_lpp_device();

		/* opening the diagnostics device for testing	*/
		rc = open_device ();

		if ( rc == NO_ERROR) 
		{
			rc = performing_test ();
		}

	}

	/* menugoal which tell the user which kind of jumper is set	*/

	if  ((rc == NO_ERROR) &&(da_input.console == CONSOLE_TRUE) &&
       		((da_input.loopmode == LOOPMODE_ENTERLM)
               	|| (da_input.loopmode == LOOPMODE_NOTLM)))
	{
		if (riser_card_determination() == ERROR)
			return(ERROR); /* determine type of riser card */
		switch( riser_card) 
		{
			case THICK_RISER_CARD: 
			{
  				sprintf( msgstr, (char *) diag_cat_gets(catd, 1,
                                        JUMPER_SETTING_DIX));
				menugoal(msgstr);
			}
			break;
			case THIN_RISER_CARD:
			{
  				sprintf( msgstr, (char *) diag_cat_gets(catd, 1,
                                        JUMPER_SETTING_BNC));
				menugoal(msgstr);
			}
			break;
			default:
			break;
		}

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

/****************************************************************
* NAME: seth_da_display
*
* FUNCTION: Update display for appropriate mode
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	test_display()
*
* RETURNS:
*	None
****************************************************************/

void seth_da_display ()
{
	if ( da_input.loopmode == LOOPMODE_INLM)
                test_display (ETHERNET_LOOPMODE);
        else if ( da_input.advanced == ADVANCED_TRUE)
                test_display (ETHERNET_ADVANCED);
        else
                test_display (ETHERNET_CUSTOMER) ;
}

/****************************************************************
* NAME: riser_card_determination
*
* FUNCTION: Run riser card type test
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  set the global variable riser_card
*
* RETURNS:
*	None
****************************************************************/

int  riser_card_determination()
{
int rc = NO_ERROR;

	/* determine riser card type */
	stileth_tucb.header.mfg = 0;
	stileth_tucb.header.loop = 1;
	stileth_tucb.header.tu = 0x11;

	riser_card = exectu(filedes, &stileth_tucb) ;

	if (verify_rc( 0x11,riser_card) == FALSE) 
		return( ERROR);

	switch( riser_card) {
		case TWISTED_PAIR_RISER_CARD:
		case THICK_RISER_CARD: 
		case THIN_RISER_CARD:
			break;
		case BADFUSE:/* bad fuse of thick */
			report_frub (FRU_125);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			rc = ERROR;
			break;
		case IO6_RD_ERR:
			report_frub (FRU_118);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			rc = ERROR;
			break;
		case RISERCARD_ERR: 
		default:
			report_frub (FRU_120);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			rc = ERROR;
			break;
	}

	if (rc == NO_ERROR)
		putdavar(da_input.dname, "riser_card", DIAG_INT, &riser_card);
	return(rc);
}

/****************************************************************
* NAME: Performing_test
*
* FUNCTION: Invokes appropriate test units
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	seth_davar_op()
*	riser_card_determination()
*	non_interactive_tests()
*	interactive_tests()
*	advanced_twisted_pair_tests()
*	advanced_thick_tests()
*	advanced_thin_tests()
*	disp_menu()
*
* RETURNS:
*	same return code of exectu()
****************************************************************/
int performing_test ()
{
	int	rc =NO_ERROR;

	if (da_input.loopmode & ( LOOPMODE_ENTERLM | LOOPMODE_NOTLM ) ) 
	{
		seth_davar_op (SETH_INIT_OP); /* initialize davars */
		if (riser_card_determination() == ERROR)
			return(ERROR); /* determine type of riser card */
	}
	else	/* read davars */
		seth_davar_op (SETH_GET_OP);

	rc =  non_interactive_tests (riser_card);

	/* system true means non-interactive test 	*/
	/* for Ethernet adapter		  		*/
	/* the interactive test is not needed		*/

	if (( rc == NO_ERROR) && ( da_input.advanced == ADVANCED_TRUE) 
			&& ( da_input.console == CONSOLE_TRUE)
			&& ( da_input.system == SYSTEM_FALSE))
		rc = interactive_tests();
	return (rc);
}


/****************************************************************
* NAME: interactive_tests()
*
* FUNCTION: Run interactive tests
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
*	disp_menu()
*
* RETURNS:
*	NO_ERROR - test passed
*	otherwise -test failed
****************************************************************/
int interactive_tests()
{
	int rc = NO_ERROR;

	if ((da_input.loopmode & LOOPMODE_INLM) && wrap_plug==FALSE)
		return(NO_ERROR);

	/* the following tests requires wrap plugs */
	if (riser_card == TWISTED_PAIR_RISER_CARD)
		rc = advanced_twisted_pair_tests();
	else if (riser_card == THICK_RISER_CARD)
		rc = advanced_thick_tests();
	else /* thin */
		rc = advanced_thin_tests();

	/* ask user to unplug wrap plug or cable if they were not
	 * there before the test  */
	if (da_input.loopmode & LOOPMODE_NOTLM) 
	{
		unplug_if_removed();
	}
	return (rc);
}

/****************************************************************
* NAME: Having_transceiver
*
* FUNCTION: Query user if he has and want to proceed with transceiver test
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	disp_menu()
*
* RETURNS:
*	YES --- Yes, user has transceiver plug
*	NO --- No, do not want transceiver test
*	QUIT--  Quit
****************************************************************/
Having_transceiver()
{
int rc1,rc2;

	/* menu 0x887004*/
 	rc1 =  disp_menu(MENU_EXIST_TRAN);
	if (rc1 == YES ) {
		/* menu 0x887005*/
		rc2 = disp_menu(MENU_TEST_TRAN);
		if (rc2 == YES)
			return(YES);
	}

	if (rc1 == NO || rc2 == NO)
		return(NO);

	return(QUIT);
}

/****************************************************************
* NAME: Having_wrap_plug
*
* FUNCTION: Ensure user have the wrap plug before proceeding wrap plug
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
*	disp_menu()
*
* RETURNS:
*	YES --- Yes, user has wrap plug
*	NO,QUIT --- otherwise
****************************************************************/

Having_wrap_plug(int wrap_plug_menu)
{
int menu_rc;

	/* menu 0x887008 */
	menu_rc = disp_menu(wrap_plug_menu);

	/* Having the wrap plug for transceiver test ? */
	if (menu_rc == YES) {
		PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
		PUTVAR(query_wrap_plug,DIAG_SHORT,TRUE);
	}
	else {
		PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
		PUTVAR(query_wrap_plug,DIAG_SHORT,FALSE);
	}

    return(menu_rc);
}

/****************************************************************
* NAME: run_wrap_plug_test_groups
*
* FUNCTION: Run test units passed in argument start
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	same return code of exectu()
****************************************************************/
int run_wrap_plug_test_groups(struct tu_fru_pair *start)
{
int rc = NO_ERROR;

	for (; start->tu != -1; ++start) {
		rc = test_tu(start->tu,start->fru);
		if (rc != NO_ERROR)
			break;
	}

	return(rc);
}

/****************************************************************
* NAME: twisted_pair_tests
*
* FUNCTION: Run test units 7,D,E,F,10 for twisted pair riser card
*	    adapter
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM or LOOPMODE_INLM
*
* RETURNS:
*	same return code of exectu()
****************************************************************/
int advanced_twisted_pair_tests()
{
int rc=ERROR;


	if (da_input.loopmode & LOOPMODE_INLM) 
	{
		/* run test 0x10 */
		stileth_tucb.header.tu = 0x10;
		rc = exectu(filedes, &stileth_tucb);
	}

	if (rc != NO_ERROR) 
	{ 
		if (da_input.loopmode & LOOPMODE_INLM) 
		{
			if (verify_rc( 0x10,rc) == FALSE) 
				return(ERROR);

			report_frub (FRU_116);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			return(ERROR);
		}
		else {
			      	/* ask the user if he has the wrap plug */ 
			     	/* ask the user to plug in the wrap plug */
			     	/* menu 0x887009 */
			if (Having_wrap_plug(MENU_WP_TWISTED) != YES 
			    || disp_menu(MENU_PLUG_TWISTED) == QUIT) 
				return(QUIT);

			seth_da_display();
			/* close the device and reopen device */
			if (stileth_tucb.mdd_fd != ERROR)
			{
				rc = close( stileth_tucb.mdd_fd);
				if (rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
sleep(3);
			if (filedes != ERROR)
			{
				rc = close (filedes);
				if ( rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
sleep(3);
			rc = open_device ();
			if ( rc != NO_ERROR) 
			{
				return (rc);
			}

			/* wrap plug test */
			rc = test_tu(0x10,FRU_116);
		}
	}
	else { /* wrap plug already there */
		PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
	}

	if ( rc == NO_ERROR  && (query_wrap_plug == TRUE ||
	     wrap_plug == TRUE) ) /* pass test 8  */
		rc = run_wrap_plug_test_groups(twisted_pair_WP_tests);

	return(rc);
}

/****************************************************************
* NAME: advanced_thick_tests
*
* FUNCTION: Run thick(DIX)  riser card adapter tests
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
*	test_tu()
*
* RETURNS:
*	same return code of exectu()
****************************************************************/

int advanced_thick_tests()
{
	int rc=ERROR;

	alarm_timeout = FALSE;
	if (da_input.loopmode & LOOPMODE_INLM) 
	{
		/* run test 8 */
    		alarm(10);

		stileth_tucb.header.tu = 8;
		rc = exectu(filedes, &stileth_tucb);
		alarm(0);
		/* this means test 8 hung	*/
		if (alarm_timeout)
			rc = -1;
	}


	if (rc != NO_ERROR)  /* fail; no wrap plug plugged yet */
	{
		if (da_input.loopmode & LOOPMODE_INLM) 
		{
			if (alarm_timeout || verify_rc(8,rc) == TRUE) 
			{
				report_frub (FRU_107);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				return( ERROR );
			}
			else
				return( ERROR );
		}
		else /* ask the user if he wants to test transceiver */ 
		{
			int menu_rc;

			if ( (menu_rc = Having_transceiver()) == YES) 
			{
				/* ask which type of transceiver */
				transceiver_type = disp_menu(MENU_WHICH_TRAN);

				switch(transceiver_type) {
				case BASE_T: /* twisted pair */
				    if (disp_menu(MENU_UNPLUG_T) == QUIT)
                                                return(QUIT);
				    seth_da_display();
				   break;
				case BASE_2: /* thick and thin */
					/* ask if the user has the wrap plug,
					   ask the user to unplug transceiver 
					   from network and plugs wrap plug 
					   into transceiver */

				     if ( Having_wrap_plug(MENU_WP_BNC)
					      != YES ||
				              disp_menu(MENU_UNPLUG_2) == QUIT) 
					 return(QUIT);

				     putdavar(da_input.dname,"transceiver_type",
						DIAG_SHORT,&transceiver_type);
				     seth_da_display();
				     break;

				case ETHERNET_UNKNOWN:
				default:
				     PUTVAR(transceiver_test,DIAG_SHORT, FALSE);
				     putdavar(da_input.dname,"transceiver_type",
						DIAG_SHORT,&transceiver_type);
				     return(QUIT);
				}

				PUTVAR(transceiver_test,DIAG_SHORT,TRUE);

				/* close the device and reopen device */
				if (stileth_tucb.mdd_fd != ERROR)
				{
					rc = close( stileth_tucb.mdd_fd);
					if ( rc == ERROR)
					{
						DA_SETRC_ERROR (DA_ERROR_OTHER);
						return (rc);
					}
				}
sleep(3);
				if (filedes != ERROR)
				{
					rc = close (filedes);
					if ( rc == ERROR)
					{
						DA_SETRC_ERROR (DA_ERROR_OTHER);
						return (rc);
					}
				}
sleep(3);
			rc = open_device ();
				if ( rc != NO_ERROR) 
				{
					return (rc);
				}

				/* perform transceiver test */
				alarm_timeout = FALSE;
			    	alarm(10);

				stileth_tucb.header.tu = 8;
				rc = exectu(filedes, &stileth_tucb);
				alarm(0);

				if ( rc == NO_ERROR  )
				{
					/* pass test 8  */
					rc = run_wrap_plug_test_groups(
						thick_and_thin_WP_tests);
					return (rc);
				}
			}
			else if (menu_rc == QUIT) 
				return(QUIT);

			if (!transceiver_test)
			{

				/* ask if the user has the wrap plug */
				/* menu 0x887007 */
				menu_rc = disp_menu(MENU_WP_DIX);
			}
			else	/* tell the user about isolation of xceiver */
			{
				menu_rc = disp_menu(MENU_ISOLATE_XCEIVER);
			}

			/* ask the user to put wrap plug in */
			switch(menu_rc) {
			case YES:
				if (transceiver_test == TRUE) {
				/* ask the user to unplug transceiver
				 * and put wrap plug in Ethernet
			        *  adapter*/
				/* menu 0x887011 */
				    int menu_rc = 
			            disp_menu(MENU_UNPLUG_XCEIVER_AND_PLUG_DIX);
					if (menu_rc == QUIT) {
					PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
					PUTVAR(query_wrap_plug,DIAG_SHORT,
						FALSE);
						return(QUIT);
					}
				}
				else 
					/* menu 0x887009 */
				if (disp_menu(MENU_PLUG_DIX)==QUIT)
						return( QUIT );
				seth_da_display();
				PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
				PUTVAR(query_wrap_plug,DIAG_SHORT,TRUE);
				break;
			case NO: /* no wrap plug; report error 
				if (verify_rc( 8,rc ) == FALSE) {
				return( ERROR );
				}
				report_frub (FRU_107);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				return( ERROR );*/
			default: /* F3, F10 */
				return(QUIT);
			}

			if (stileth_tucb.mdd_fd != ERROR)
			{
				rc = close( stileth_tucb.mdd_fd);
				if ( rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
sleep(3);
			if (filedes != ERROR)
			{
				rc = close (filedes);
				if ( rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
			/* give it more time for device to complete	*/
sleep(3);
			rc = open_device ();

			if ( rc != NO_ERROR) 
			{
				return (rc);
			}

			/* transceiver test or wrap plug test */
			alarm_timeout = FALSE;
			alarm(10);

			stileth_tucb.header.tu = 8;
			rc = exectu(filedes, &stileth_tucb);
			alarm(0);

			if ( alarm_timeout == FALSE && rc == NO_ERROR) {
		            if (transceiver_test == TRUE) {
				if (transceiver_type == BASE_T)
					report_frub (FRU_121);
				else
				if (transceiver_type == BASE_2)
					report_frub (FRU_122);
				DA_SETRC_STATUS (DA_STATUS_BAD);
			   }
			}
			else {
				if (alarm_timeout || verify_rc(8,rc)==TRUE) {
					report_frub (FRU_107);
					DA_SETRC_STATUS (DA_STATUS_BAD);
				}
				else 
					return(ERROR);
			}
		}
	}
	else { /* wrap plug already there */
		PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
	}
	if ( rc == NO_ERROR  && (query_wrap_plug == TRUE ||
	     wrap_plug == TRUE) ) /* pass test 8  */
		rc = run_wrap_plug_test_groups(thick_and_thin_WP_tests);
	return(rc);
}
		
int advanced_thin_tests()
{
int rc=ERROR;

	alarm_timeout = FALSE;
	if (da_input.loopmode & LOOPMODE_INLM) 
	{
		/* run test 9 */
    		alarm(10);

		stileth_tucb.header.tu = 9;
		rc = exectu(filedes, &stileth_tucb);
		alarm(0);

		/* this means test 9 hangs	*/
		if (alarm_timeout)
			rc = -1;
	}


	if (rc != NO_ERROR) { /* fail */
		if (da_input.loopmode & LOOPMODE_INLM) 
		{
			if (alarm_timeout || verify_rc(9, rc ) == TRUE ) 
			{
				report_frub (FRU_108);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				return ( ERROR );
			}
			else
				return ( ERROR );
		}
		else 
		{ 
			/* ask the user if he has the wrap plug */ 
			if ( Having_wrap_plug(MENU_WP_BNC) != YES ||
			/* ask the user to plug in the wrap plug */
			/* menu 0x887009 */
			    disp_menu(MENU_PLUG_BNC) == QUIT) 
				return(QUIT);
			if (stileth_tucb.mdd_fd != ERROR)
			{
				rc = close( stileth_tucb.mdd_fd);
				if (rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
sleep(3);
			if (filedes != ERROR)
			{
				rc = close (filedes);
				if (rc == ERROR)
				{
					DA_SETRC_ERROR (DA_ERROR_OTHER);
					return (rc);
				}
			}
			seth_da_display ();
			rc = open_device ();

			if ( rc != NO_ERROR) 
			{
				return (rc);
			}

			/* wrap plug test */
			rc = test_tu(9,FRU_108);
		}
	}
	else  	/* wrap plug already there */
	{
		PUTVAR(wrap_plug,DIAG_SHORT,TRUE);
	}

	if (rc == NO_ERROR && (wrap_plug == TRUE || query_wrap_plug
	    == TRUE) )
		rc = run_wrap_plug_test_groups(thick_and_thin_WP_tests);
}

/****************************************************************
* NAME: non_interactive_test
*
* FUNCTION: Run test unit 1 through 6
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	same return code of exectu()
****************************************************************/

int non_interactive_tests (int riser_card)
{
struct	tu_fru_pair *start;
int	rc;

	stileth_tucb.header.mfg = 0;
	stileth_tucb.header.loop = 1;

	if (riser_card == TWISTED_PAIR_RISER_CARD)
		start = twisted_pair_NWP_tests;
	else
		start = thick_and_thin_NWP_tests;

	for (; start->tu != -1; ++start) {
		rc = test_tu(start->tu,start->fru);
		if ( rc != NO_ERROR)
			break;
		 if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
			return(QUIT);
	}

	return(rc);
}

/* run test unit and report FRU specified by fru_index */
/* if fru_index equals -1 then no FRU report is needed */
/****************************************************************
* NAME: test_tu
*
* FUNCTION: run test unit and report FRU specified by fru_index
*	    for failed test; and if fru_index equals NO_REPORT_FRU (-1) then
*	    no FRU report is needed.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	  report_frub()
*
* RETURNS:
*	same return code of exectu()
****************************************************************/
int test_tu (int test_unit,int fru_index)
{
int rc;

	stileth_tucb.header.tu = test_unit;

	alarm_timeout = FALSE;
	alarm(10);

	rc = exectu(filedes, &stileth_tucb);
	alarm(0);

	if (alarm_timeout ) {
			report_frub (fru_index);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			return (ERROR);
	}

	if (rc  != NO_ERROR) {
	   if (verify_rc(test_unit,rc) == TRUE ) {
		report_frub (fru_index);
		DA_SETRC_STATUS (DA_STATUS_BAD);
           }
	}
	return(rc);
}

/****************************************************************
* NAME: verify_rc
*
* FUNCTION: verify the return code from exectu() of the specified
*	    test unit is a valid return code
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	TRUE -- is in valid return code range
*	FALSE - is not a valid return code
****************************************************************/
verify_rc(int test_unit,int rc)
{
int *rc_ptr = tu_rctbl[--test_unit];

	for (; *rc_ptr; rc_ptr++) {
		if (*rc_ptr == rc )
			return(TRUE); 
	}

	DA_SETRC_ERROR (DA_ERROR_OTHER);
	return(FALSE);
}

/****************************************************************
* NAME: open_device
*
* FUNCTION: opening the device 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	errno after open()
****************************************************************/
int open_device ()
{
	int	rc =NO_ERROR;	
	int	open_error;
	char devname[32];
	

	strcat (devname, da_input.dname);
	sprintf(devname,"/dev/%s/D",da_input.dname);

	/* open device			*/
	alarm_timeout = FALSE;
	alarm(10);
	/* Note Do not use open with O_NDELAY because the test unit is not */
	/* designed with this flag. DA has been experimenting with this    */
	/* flag and there is a core dump in test unit.			   */
	/* So do not use with this flags on				   */

	filedes = open(devname, O_RDWR);
	alarm(0);

	open_error = errno;
	
	if (alarm_timeout || filedes == ERROR  )
	{
		DA_SETRC_ERROR (DA_ERROR_OPEN);
		return (open_error);
	}

	alarm(10);
	stileth_tucb.mdd_fd = open("/dev/bus0",O_RDWR);
	alarm(0);

	if (alarm_timeout || stileth_tucb.mdd_fd == ERROR) {
		DA_SETRC_ERROR (DA_ERROR_OPEN);
		return (errno);
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
                  da_input.date,
                  da_input.dname, ERRID_IENT_ERR1,ERRID_IENT_ERR3);

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
	if (ela_get_rc >0)
	{
		DA_SETRC_ERROR (DA_STATUS_BAD);
		switch (errdata_str.err_id)
		{
			case ERRID_IENT_ERR1:
			case ERRID_IENT_ERR3:
				report_frub (FRU_124);
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
        clean_up();
}

/****************************************************************
* NAME: test_display
*
* FUNCTION: This function will display three different message. Advanced,
*	    loopmode and regular testing menu.
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	None
****************************************************************/

void test_display (menu_type)
int	menu_type;
{
        int     rc=0;
        char    *text_array[3];


        text_array[0] = diag_cat_gets (catd, 1,1);
        text_array[1] = da_input.dname;
        text_array[2] = da_input.dnameloc;

        switch (menu_type)
        {
                case ETHERNET_LOOPMODE:
                        diag_display_menu (LOOPMODE_TESTING_MENU, 0x887003,
                                text_array, da_input.lcount, da_input.lerrors);

                        break;

                case ETHERNET_ADVANCED:
                        diag_display_menu (ADVANCED_TESTING_MENU, 0x887002,
                                text_array, da_input.lcount, da_input.lerrors);

                        break;

                case ETHERNET_CUSTOMER:
                        diag_display_menu (CUSTOMER_TESTING_MENU, 0x887001,
                                text_array, da_input.lcount, da_input.lerrors);
                        break;
		default:
			break;
	}

}

/****************************************************************
* NAME: report_frub
*
* FUNCTION: The function will add fru bucket into database
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
*	None
****************************************************************/

void report_frub (frub_idx)
int frub_idx;
{
struct fru_bucket *frub_addr = &seth_frus[frub_idx];


	/* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);

       	if (insert_frub(&da_input, frub_addr) != NO_ERROR ||
	    addfrub(frub_addr) != NO_ERROR) {
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}
}

/****************************************************************
* NAME: check_asl_stat
*
* FUNCTION: Check asl return code
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*
* RETURNS:
****************************************************************/
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
		case ASL_OK:
		case ASL_ENTER:
		case ASL_COMMIT:
			break;
		default:
			DA_SETRC_ERROR (DA_ERROR_OTHER);
			return_code = QUIT;
			break;
		
	}
	return (return_code);
} /* check_asl_stat 	*/

/****************************************************************
* NAME: poll_kb_for_quit
*
* FUNCTION: Poll the keyboard input to see whether user want to
*	    quit
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	check_asl_stat()
*
* RETURNS:
*	QUIT ---- user entered CANCEL,EXIT
*	NO_ERROR -otherwise
****************************************************************/
int poll_kb_for_quit()
{
int rc;

	if (check_asl_stat( diag_asl_read(ASL_DIAG_LIST_CANCEL_EXIT_SC, FALSE, NULL))==QUIT) {
		return(QUIT);
	}
	else {
		return(NO_ERROR);
	}
}

/****************************************************************
* NAME: disp_menu
*
* FUNCTION: Construct and display the menu passed by the menu_index
*
* EXECUTION ENVIRONMENT:
*
* NOTES:  Other local routines called --
*	Only the first display line (title) need to be updated
* RETURNS:
*	the index of menu item selected
****************************************************************/
int disp_menu(int menu_index)
{
SETH_MENU *menu_ptr;
int rc;

	menu_ptr = &menutbl[menu_index];
	rc = diag_display(menu_ptr->menu_no, catd,menu_ptr->msg_list,
               			DIAG_MSGONLY, menu_ptr->scrtype,
               			&menutypes, menu_ptr->menu_list);

       	sprintf(msgstr, menu_ptr->menu_list->text, 
				da_input.dname, da_input.dnameloc);
	free (menu_ptr->menu_list->text);
       	menu_ptr->menu_list->text = msgstr;
	
	switch(menu_ptr->menu_no) {
	case 0x887004: /* index by MENU_EXIST_TRAN */
		menutypes.cur_index = NO;
		break;
	case 0x887005: /* MENU_TEST_TRAN */
		menutypes.cur_index = YES;
		break;
	case 0x887006: /* MENU_WHICH_TRAN */
		menutypes.cur_index = BASE_T;
		break;
	case 0x887007: 
	/* MENU_WP_BNC, MENU_WP_DIX, MENU_WP_TWISTED*/
		menutypes.cur_index = NO;
		break;
	default: /* do nothing */
		break;
	}

	rc = diag_display(menu_ptr->menu_no, catd,NULL,
               			DIAG_IO, menu_ptr->scrtype,
               			&menutypes, menu_ptr->menu_list);
	if (check_asl_stat(rc) == QUIT) {
		if (menu_ptr->menu_no != MENU_PLUG_T &&
			menu_ptr->menu_no != MENU_PLUG_2 &&
			menu_ptr->menu_no != MENU_UNPLUG_WP) {
			PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
			PUTVAR(query_wrap_plug,DIAG_SHORT,FALSE);
			return(QUIT);
		}
	}

	if (rc == DIAG_ASL_COMMIT) /* user make item selection */
		rc = DIAG_ITEM_SELECTED(menutypes);
	else /* just ENTER to continue */
		rc = NO_ERROR;

	return(rc);
}


/****************************************************************
* NAME: seth_davar_op
*
* FUNCTION: Initialize or read the necessary davars
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
* 	if op is SETH_INIT_OP then initialize davars
*       else if op is SETH_GET_OP then read davars
*
* RETURNS:
*	None
****************************************************************/
void seth_davar_op(int op)
{
	switch(op) {
	case SETH_INIT_OP:
		wrap_plug = FALSE;
		query_wrap_plug = FALSE;
		riser_card = TWISTED_PAIR_RISER_CARD;
		transceiver_test = FALSE;
		transceiver_type = ETHERNET_UNKNOWN;

		PUTVAR(wrap_plug,DIAG_SHORT,FALSE);
		PUTVAR(query_wrap_plug,DIAG_SHORT,FALSE);
		PUTVAR(riser_card,DIAG_INT,TWISTED_PAIR_RISER_CARD);
		PUTVAR(transceiver_test,DIAG_SHORT,FALSE);
		PUTVAR(transceiver_type,DIAG_SHORT,ETHERNET_UNKNOWN);
		break;
	case SETH_GET_OP:
		getdavar(da_input.dname, "wrap_plug",
			 DIAG_SHORT, &wrap_plug);
		getdavar(da_input.dname, "query_wrap_plug",
			 DIAG_SHORT, &query_wrap_plug);
		getdavar(da_input.dname, "riser_card",
			 DIAG_INT, &riser_card);
		getdavar(da_input.dname, "transceiver_test",
			 DIAG_SHORT, &transceiver_test);
		getdavar(da_input.dname, "transceiver_type",
			 DIAG_SHORT, &transceiver_type);
	}
}
/*
 * NAME: timeout()
 *
 * FUNCTION: Desined to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void timeout(int sig)
{
        alarm(0);
	alarm_timeout = TRUE;
}

/*--------------------------------------------------------------------------
* NAME: general_initialization () 
*
* FUNCTION: Initialize or read the necessary davars
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:
*	   NO_ERROR : Good
*-------------------------------------------------------------------------- */
void general_initialization ()
{

	setlocale(LC_ALL,"");

	/* initializing interrupt handler */    
	invec.sa_handler = intr_handler;
	sigaction(SIGINT, &invec, (struct sigaction *)NULL);
	invec.sa_handler = timeout;
	sigaction( SIGALRM, &invec, (struct sigaction *) NULL );

	/*..............................*/
	/* General Initialization	*/
	/*..............................*/

	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);
	DA_SETRC_USER (DA_USER_NOKEY);

	/* initialize odm	*/
	odm_flg = init_dgodm();
	stileth_tucb.mdd_fd = ERROR;

	/* get da input 	*/
	if (odm_flg == -1 || getdainput(&da_input) != 0) 
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}

	if 	/* running under console */
	   ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl 		*/
		asl_flg = diag_asl_init ( ASL_INIT_DEFAULT);
		if (asl_flg == ERROR )
			clean_up();

		catd = diag_catopen (MF_DSTILETH,0); 
	}
        lpp_configure_method= (char *) malloc (1200 *sizeof (char));
        lpp_unconfigure_method= (char *) malloc (1200 *sizeof (char));
	cudv_p = (struct CuDv *) malloc (1200 *sizeof (char));
	pddv_p = (struct PdDv *) malloc (1200 *sizeof (char));
	if((lpp_unconfigure_method == (char *) NULL) || 
	      (lpp_configure_method == (char *) NULL)) 
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}
}
/*--------------------------------------------------------------------------
* NAME: plug_back_if_removed()
*
* FUNCTION: Restore network cable(s) to the normal state if removed 
*
* EXECUTION ENVIRONMENT:
*
* NOTES:
*
* RETURNS:
*	   NO_ERROR : Good
*-------------------------------------------------------------------------- */
void unplug_if_removed()
{
	int	rc;

	if ((transceiver_test == TRUE) && (query_wrap_plug == TRUE))
	{
		/* means some problem with transceiver */
		if (transceiver_type == BASE_2)
		{
			rc = disp_menu(MENU_UNPLUG_DIX_AND_PUT_BASE2);

		}
		else	/* 10 BASE- T */
		{
			rc = disp_menu(MENU_UNPLUG_DIX_AND_PUT_TWISTED);
		}
	}
	else if (transceiver_test == TRUE) 
	{
		if (transceiver_type == BASE_2)
		/* ask the user to put back transceiver
		 * ,unplug wrap plug from transceiver
		 * and put network cable back */
		/* menu 0x887010 */
			rc = disp_menu(MENU_PLUG_2);
		else /* 10BASE-T */
			/* ask the user to put back transceiver
			 * , turn link switch to on
			 * and put the network cable back */
			/* menu 0x887010 */
			rc = disp_menu(MENU_PLUG_T);
	}
	else if (query_wrap_plug == TRUE) 
	{
		/* ask the user to unplug wrap plug from Ethernet 
		 * adapter and put the network cable back */
		/* menu 0x887010*/
			rc = disp_menu(MENU_UNPLUG_WP);
	}
	seth_davar_op (SETH_INIT_OP); /* initialize davars */

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

	if (stileth_tucb.mdd_fd != ERROR)
		close( stileth_tucb.mdd_fd);
sleep(2);

        if (filedes != ERROR)
                close (filedes);
        /*  Unload the diagnostics device driver and set the lpp device */
        /*  to DEFINE state                                             */
        configure_lpp_device();


        if (prior_state == AVAILABLE)
        {
                sprintf (option," -l %s", da_input.dname);
                rc= invoke_method(lpp_configure_method,option,
                                   &new_out,&new_err);
                if (rc != NO_ERROR)
                {
                        sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_CONFIGURED),
                                da_input.dname, da_input.dnameloc);
                        menugoal (msgstr);
                }
                DA_EXIT();
        }

        if ( da_input.console == TRUE )
        {
                if (catd != CATD_ERR)
                        catclose (catd);
                if (asl_flg != ERROR)
                        diag_asl_quit (NULL);

        }

        /* terminate odm */
        term_dgodm();
        clean_malloc();

        DA_EXIT ();

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
        if(lpp_configure_method != (char *) NULL)
                free (lpp_configure_method);
        if(lpp_unconfigure_method != (char *) NULL)
                free (lpp_unconfigure_method);
	if (cudv_p != (struct CuDv *) NULL)
		free (cudv_p); 
	if (pddv_p != (struct PdDv *) NULL)
		free (pddv_p); 
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
	struct listinfo v_info;
        char    criteria[128];
	char	*result_p;
	int	result;

	/* Unconfigure diagnostics device. Unload the device from system */
	if(diag_device_configured)
	{
		/* UCFG is defined as 1			*/
		sprintf (option," -l %s -f 1", da_input.dname);
		strcpy (criteria,"/usr/lib/methods/cfgddeth");
		result = invoke_method(criteria,option, &new_out,&new_err);
		if (result !=  NO_ERROR)
		{
			sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
				DIAG_DEVICE_CANNOT_UNCONFIGURED), 
				cudv_p->name, cudv_p->location);
			menugoal (msgstr);
		}
	}

	/* Setting the device state to DEFINE state */
	/* clean_up will restore the state to original	*/
	/* The DIAGNOSE state of the device is set to DEFINE state */
	/* Even though before running diagnostics it is in DIAGNOSE state */
	if (set_device_to_diagnose)
	{
		sprintf (option," -l %s", da_input.dname);
		strcpy (criteria,"/usr/lib/methods/ucfgdiag");
		result = invoke_method(criteria,option,
			 &new_out,&new_err);
		if (result == NO_ERROR)
		{
			set_device_to_diagnose= FALSE;
		}
		else 
		{
			sprintf(msgstr, (char *)diag_cat_gets (
			  catd, 1,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
				 da_input.dname, da_input.dnameloc);
			menugoal (msgstr);
		}
	}

}
/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device */
/*      Description:    
/*	Return		: 0 Good 				*/
/*			  -1 BAD				*/
/*--------------------------------------------------------------*/
int	unconfigure_lpp_device()
{
	struct listinfo v_info;
        char    criteria[128];
	char	*result_p;
	int	result;

        sprintf (criteria, "location like %s* AND name=%s AND chgstatus != 3", 
		da_input.dnameloc, da_input.dname);
        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
        if ( cudv_p == (struct CuDv *) NULL )
        {
		sprintf(msgstr, (char *)diag_cat_gets (
			catd, 1,DEVICE_DOES_NOT_EXIST),
				 cudv_p->name, cudv_p->location);
		menugoal (msgstr);
		clean_up();
        }
	else
	{
		/* Make the device into DEFINE state		*/
		if (cudv_p->status == AVAILABLE)
		{

			sprintf(criteria,"uniquetype like adapter/sio/ient_1*");
			pddv_p = get_PdDv_list (PdDv_CLASS, criteria, 
				&v_info, 1, 1);
			strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
			strcpy (lpp_configure_method, pddv_p->Configure);


			sprintf (option," -l %s", da_input.dname);
			result = invoke_method(lpp_unconfigure_method,option,
				&new_out,&new_err);
			if (result == NO_ERROR)
			{
				unconfigure_lpp = TRUE;
			}
			else
			{
				sprintf(msgstr, (char *)diag_cat_gets (
					catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
				 	cudv_p->name, cudv_p->location);
				menugoal (msgstr);
				clean_up();
			}
		}
	}

	/* The device is either in DIAGNOSE or DEFINE state or STOPPED */ 
	
	sprintf (option," -l %s", da_input.dname);
	strcpy (criteria,"/usr/lib/methods/cfgdiag");
	result = invoke_method(criteria,option, &new_out,&new_err);
	if (result == NO_ERROR)
	{
		set_device_to_diagnose= TRUE;
	}
	else
	{
		sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
			LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE), 
			cudv_p->name, cudv_p->location);
		menugoal (msgstr);
		clean_up();
	}

	/* The diagnostics device driver needs to be loaded into the system */

	/* CFG is defined as 0	*/
	sprintf (option," -l %s -f 0", da_input.dname);
	strcpy (criteria,"/usr/lib/methods/cfgddeth");
	result = invoke_method(criteria,option, &new_out,&new_err);
	if (result == NO_ERROR)
	{
		diag_device_configured=TRUE;
	}
	else
	{
		/* report frubucket and exit DA		*/
		report_frub (FRU_117);
		DA_SETRC_STATUS (DA_STATUS_BAD);
		clean_up();
	}

	return (0);
}
