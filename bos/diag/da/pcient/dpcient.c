static char sccsid[] = "@(#)47  1.4  src/bos/diag/da/pcient/dpcient.c, dapcient, bos41J, 9523B_all 6/6/95 17:13:41";
/*
 *   COMPONENT_NAME: dapcient
 *
 *   FUNCTIONS: bnc_wrap_plug_test
 *		check_asl_stat
 *		clean_up
 *		close_device
 *		da_display
 *		da_odm_vars
 *		determine_card_type
 *		display
 *		display_common_menu
 *		display_menugoal
 *		dix_connection_test
 *		dix_wrap_plug_skip_xceiver_test
 *		dix_wrap_plug_test
 *		error_log_analysis
 *		general_initialize
 *		interactive_tests
 *		intr_handler
 *		isolate_xceiver_test
 *		main
 *		non_interactive_tests
 *		open_device
 *		perform_test
 *		poll_kb_for_quit
 *		report_frub
 *		set_timer
 *		test_10_base_2_transceiver
 *		test_tu
 *		test_twisted_pair_transceiver
 *		timeout
 *		tp_wrap_plug_test
 *		unplug_cable_and_put_wrap
 *
 *   ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <errno.h>
#include <sys/signal.h>
#include <locale.h>
#include <diag/diag_trace.h>

#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/bit_def.h"

#include "dpcient_msg.h"
#include "dpcient.h"
#include "kent_defs.h"
#include "kent_tu_type.h"
#include "kent_errids.h"

extern	nl_catd diag_catopen();
extern	char *diag_cat_gets();
extern 	getdainput();			/* gets input from ODM */
extern 	long getdamode();		/* transforms input into a mode */
extern  int exectu ();

TU_TYPE	eth_tucb;

/* davar used */
short   wrap_plug = FALSE;		/* wrap plug is not on is assumed */
short   xceiver_used = XCEIVER_NONE;	
short	xceiver_removed_flag = FALSE;
short   adapter_type = GENERIC_ADAPTER;	/* wrap plug is not on is assumed */
struct  sigaction  invec;   		/* interrupt handler structure */
volatile int	alarm_timeout=FALSE;

int 	odm_flg = 0;			/* return code from init_dgodm() */
int	asl_flg = NOT_GOOD; 		/* return code from diag_asl_init() */

struct 	tm_input da_input;		/* initialized by getdainput */
nl_catd catd = (nl_catd) CAT_CLOSED;	/* pointer to message catalog */
short	device_open_flag = FALSE;

/* alphabetical order of functions used */
int 	bnc_wrap_plug_test ();
int 	check_asl_stat(int);
void 	clean_up();
void 	da_display();
void 	da_odm_vars(int op);
void 	determine_card_type();
int	display();
void	display_common_menu();
void	display_menugoal();
int 	dix_connection_test ();
int 	dix_wrap_plug_skip_xceiver_test ();
int 	dix_wrap_plug_test ();
void 	error_log_analysis ();
void 	general_initialize();
int 	interactive_tests();
void	intr_handler(int);	
int	isolate_xceiver_test();
int 	non_interactive_tests();
int 	open_device();
int 	perform_test();
int 	poll_kb_for_quit();
void 	report_frub();
void 	set_timer();
int	test_10_base_2_transceiver();
int 	test_twisted_pair_transceiver();
int 	test_tu(int,int,int);
int 	tp_wrap_plug_test ();
void 	timeout(int);
int 	unplug_cable_and_put_wrap ();

/*--------------------------------------------------------------*
 *	Messages and menus					*
 *--------------------------------------------------------------*/
static	ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
static	char	msgstr0[2048];
static	char	msgstr1[2048];
static	char	msgstr2[2048];
static	char	msgstr3[2048];


/*--------------------------------------------------------------------------
| 	NAME: main
|
| 	FUNCTION: main function for PCI Ethernet DA
|
| 	EXECUTION ENVIRONMENT:
|
|       RETURNS: does not return, the clean_up() routine returns to the
|		 Diag Controller
*--------------------------------------------------------------------------*/
main (argc, argv)
int argc;
char **argv;
{
	int	rc;

	(void) general_initialize ();
	(void) determine_card_type();

	/* executing test if running problem determination *
	 * or repair mode */
	if (( da_input.dmode & (DMODE_PD | DMODE_REPAIR)) &&
	    (!(da_input.loopmode & LOOPMODE_EXITLM)))
	{
		(void) da_display ();
	
		open_device();

           	rc = perform_test();
	}

	/* ask user to put back the network */
	if ((da_input.loopmode & (LOOPMODE_EXITLM | LOOPMODE_NOTLM))
	   && (da_input.console == CONSOLE_TRUE)){
		if (xceiver_removed_flag)
			display((xceiver_used == XCEIVER_TP) ?
				 UNPLUG_DIX_AND_PUT_TP_XCEIVER_BACK :
				 UNPLUG_DIX_AND_PUT_GENERIC_XCEIVER_BACK);
		else if (xceiver_used == XCEIVER_TP)
			display(PLUG_T_XCEIVER_BACK);
		else if (xceiver_used == XCEIVER_T2)
			display(PLUG_GENERIC_XCEIVER_BACK);
	    	else if (wrap_plug)
			display(UNPLUG_WRAP_GENERIC);
	}	

	/* if running problem determination, or error log analyis
	 * execute error log analysis */
	if  (rc == NO_ERROR && (da_input.dmode & (DMODE_PD | DMODE_ELA)) &&
		(da_input.loopmode & (LOOPMODE_NOTLM | LOOPMODE_ENTERLM)))
	{
		  (void) error_log_analysis ();
	}
	(void) clean_up ();
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
	if (da_input.console != CONSOLE_TRUE)
		return;
	if ( da_input.loopmode == LOOPMODE_INLM) 
		(void) display_common_menu(LOOPMODE_TESTING_MENU);
	else if ( da_input.advanced == ADVANCED_TRUE)
			(void) display_common_menu(ADVANCED_TESTING_MENU);
	else
		(void) display_common_menu(CUSTOMER_TESTING_MENU) ;
	sleep(3);
}

/*--------------------------------------------------------------------------
| 	NAME: determine_card_type
|
| 	FUNCTION: determine the type of adapter to be tested
|
| 	EXECUTION ENVIRONMENT:
|
| 	RETURNS: none
|	
|	NOTES: side effect: change value of global variable adapter_type
----------------------------------------------------------------------------*/
void
determine_card_type()
{
	if ((da_input.console != CONSOLE_TRUE) ||
	    (da_input.system == SYSTEM_TRUE) ||
	    (da_input.loopmode & (LOOPMODE_INLM | LOOPMODE_EXITLM)))
		return;		/* use the default generic type */

	/* adjust the menu's selection position to 
	 * the index of adapter type */
	adapter_type = display(WHAT_TYPE_ADAPTER) - 1;
	putdavar(da_input.dname, "adapter_type", DIAG_SHORT, adapter_type);
}


/*--------------------------------------------------------------------------
| 	NAME: perform_test
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
|		good: NO_ERROR
|		bad : otherwise
----------------------------------------------------------------------------*/
int perform_test ()
{
	int	rc = NO_ERROR;

	rc = non_interactive_tests();

	/* system true means non-interactive test 	*/

	if ((rc == NO_ERROR) &&
	    (da_input.advanced == ADVANCED_TRUE) &&
	    ((da_input.console == CONSOLE_TRUE) ||
	     (da_input.exenv == EXENV_SYSX)) &&
	    (da_input.system == SYSTEM_FALSE))
		rc = interactive_tests();
	return (rc);
}


/*--------------------------------------------------------------------------
| 	NAME: non_interactive_test
|
| 	FUNCTION: Run test units 1 through 5
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|
| 	RETURNS:
|		good: NO_ERROR
|		bad : otherwise
----------------------------------------------------------------------------*/
int non_interactive_tests ()
{
	struct	tu_fru_group_pair	*tu_ptr;
	int	rc;

        if (da_input.exenv == EXENV_SYSX
        	&& da_input.loopmode != LOOPMODE_INLM)
        {
		return(NO_ERROR);
        }

	for (tu_ptr = internal_tus; tu_ptr->tu_id != -1; ++tu_ptr) {
		rc = test_tu(tu_ptr->tu_id, TU_ACT_REPORT_FRU,
				tu_frus[tu_ptr->fru_group][adapter_type]);
		if ( rc != NO_ERROR)
			break;
		if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
			return(QUIT);
	}

	return(rc);
}


/*--------------------------------------------------------------------------
| 	NAME: interactive_tests()
|
| 	FUNCTION: Run interactive tests
|		  
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:   
|
| 	RETURNS:
|		pass: NO_ERROR
|		fail: otherwise 
----------------------------------------------------------------------------*/
int interactive_tests()
{
	int 	rc = NO_ERROR;
	int	menu_rc;

	if (da_input.loopmode & LOOPMODE_INLM) 
	{
		if (xceiver_used) 
			rc = test_tu (EXTERNAL_WRAP_AUI, TU_ACT_REPORT_FRU,
				xceiver_frus[XCEIVER_FRUS_80][xceiver_used]);
		else {
			switch (wrap_plug) {
			case WP_BNC:
				rc = bnc_wrap_plug_test ();
				break;
			case WP_TP:
				rc = tp_wrap_plug_test ();
				break;
			case WP_DIX:
				rc = dix_wrap_plug_test ();
				break;
			default:
				break;	/* should never happens */
			}
		}
		return (rc);
	}

	if (da_input.loopmode & (LOOPMODE_NOTLM | LOOPMODE_ENTERLM))
        {
		switch (adapter_type) {
		case T2_ADAPTER:
			menu_rc = display (T2_WHAT_TYPE_CONNECTOR);
			rc = ((menu_rc == MENU_SELECT_1)?
				tp_wrap_plug_test() : bnc_wrap_plug_test());
			break;

		case TP_ADAPTER:
			menu_rc = display (TP_WHAT_TYPE_CONNECTOR);
			rc = ((menu_rc == MENU_SELECT_1)?
				tp_wrap_plug_test() : dix_connection_test());
			break;
		default:	/* no external test */
			break;
		}
		return (rc);
	}
} /* end interactive_tests */


/*--------------------------------------------------------------------------
| 	NAME: dix_connection_test()
|
| 	FUNCTION: driver to run test on dix connection
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
int
dix_connection_test()
{
	if (display (XCEIVER_EXIST) != YES)
		return(dix_wrap_plug_test ());

	if (display (TEST_XCEIVER) != YES)
		return(dix_wrap_plug_skip_xceiver_test ());

	switch (display (WHAT_TYPE_XCEIVER)) {
	case 1:		/* 10 Base T Xceiver */
		return(test_twisted_pair_transceiver());
	case 2:		/* 10 Base 2 Xceiver */
		return(test_10_base_2_transceiver());
	default:
		return (NO_ERROR);
	}
}


/*---------------------------------------------------------------------------
| NAME: bnc_wrap_plug_test()
|
| FUNCTION: Ask the user to put BNC  wrap plug in.
|	    Run TU tests specified in the TU Specs.
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int bnc_wrap_plug_test ()
{
	int rc, menu_rc;
 
	if (da_input.loopmode != LOOPMODE_INLM) {
		menu_rc = display (HAVE_WRAP_PLUG_BNC);
		if (menu_rc != YES)
			return (NO_ERROR);
		
		display (PLUG_BNC_CONNECTOR);
		(void) da_display();

		PUTVAR(wrap_plug, DIAG_SHORT, WP_BNC);
	}

	rc = test_tu(EXTERNAL_WRAP_10Base2, TU_ACT_REPORT_FRU,
		     tu_frus[ADAPTER_FRUS_100][adapter_type]);
	return (rc);
}

/*---------------------------------------------------------------------------
| NAME: dix_wrap_plug_test()
|
| FUNCTION: Ask the user to put DIX the wrap plug in and testing 
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int dix_wrap_plug_test ()
{
	int rc, menu_rc;

	if (da_input.loopmode != LOOPMODE_INLM) {
		menu_rc = display (HAVE_WRAP_PLUG_DIX);
		if (menu_rc != YES)
			return (NO_ERROR);
	
		display (PLUG_DIX_CONNECTOR);
		(void) da_display();

		PUTVAR(wrap_plug, DIAG_SHORT, WP_DIX);
	}

	rc = test_tu(EXTERNAL_WRAP_AUI, TU_ACT_REPORT_FRU,
		     tu_frus[ADAPTER_FRUS_100][adapter_type]);
	return (rc);
}


/*---------------------------------------------------------------------------
| NAME: tp_wrap_plug_test()
|
| FUNCTION: Ask the user to put RJ-45 wrap plug in.
|	    Run TU tests specified in the TU Specs.
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int tp_wrap_plug_test ()
{
	int rc, menu_rc;
 
	if (da_input.loopmode != LOOPMODE_INLM) {
		menu_rc = display (HAVE_WRAP_PLUG_TWISTED);
		if (menu_rc != YES)
			return (NO_ERROR);
		
		display (PLUG_TP_CONNECTOR);
		(void) da_display();

		PUTVAR(wrap_plug, DIAG_SHORT, WP_TP);
	}

	rc = test_tu(EXTERNAL_WRAP_10BaseT, TU_ACT_REPORT_FRU,
		     tu_frus[ADAPTER_FRUS_100][adapter_type]);
	return (rc);
}

/*---------------------------------------------------------------------------
| NAME: dix_wrap_plug_skip_xceiver_test()
|
| FUNCTION: Ask the user to put DIX the wrap plug in and testing 
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int dix_wrap_plug_skip_xceiver_test ()
{
	int rc, menu_rc;

	if (da_input.loopmode != LOOPMODE_INLM) {
		menu_rc = display (HAVE_WRAP_PLUG_DIX);
		if (menu_rc != YES)
			return (NO_ERROR);
	
		display (UNPLUG_XCEIVER_AND_PLUG_DIX);
		(void) da_display();

		PUTVAR(xceiver_removed_flag, DIAG_SHORT, TRUE);
	}

	rc = test_tu(EXTERNAL_WRAP_AUI, TU_ACT_REPORT_FRU,
		     tu_frus[ADAPTER_FRUS_100][adapter_type]);
	return (rc);
}


/*---------------------------------------------------------------------------
| NAME: test_twisted_pair_transceiver () 
|
| FUNCTION: test twisted pair transceiver
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int test_twisted_pair_transceiver ()
{
	if (display(HAVE_WRAP_PLUG_TWISTED) != YES)
		return(NO_ERROR);
	display (PLUG_XCEIVER_T);
	(void) da_display ();

	PUTVAR(xceiver_used, DIAG_SHORT, XCEIVER_TP);

	if (test_tu(EXTERNAL_WRAP_AUI, TU_ACT_NOP, NULL) == NO_ERROR)
		return(NO_ERROR);

	return (isolate_xceiver_test());
}

/*---------------------------------------------------------------------------
| NAME: isolate_xceiver_test () 
|
| FUNCTION: This procedure will ask the user to unplug transceiver 
|	    and put DIX wrap plug for problem isolation 
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
| 	ETH_ERROR: for all cases
|               
----------------------------------------------------------------------------*/
int isolate_xceiver_test ()
{
	int	rc;

	rc = display (ISOLATE_XCEIVER);
	if (rc != YES)
	{
		/* Report 80 % Transceiver , 20 % Ethernet adapter      */
		(void) report_frub (xceiver_frus[XCEIVER_FRUS_80][xceiver_used]);
		return (ETH_ERROR);
	}

	display (UNPLUG_XCEIVER_AND_PLUG_DIX);
	(void) da_display();

	PUTVAR(xceiver_removed_flag, DIAG_SHORT, TRUE);

	/* if test_tu fails, report 100% adapter problem */
	rc = test_tu(EXTERNAL_WRAP_AUI, TU_ACT_REPORT_FRU,
		     tu_frus[ADAPTER_FRUS_100][adapter_type]);

	if (rc == NO_ERROR)
		/* The test was good therefore it's 100% transceiver problem */
		(void) report_frub(xceiver_frus[XCEIVER_FRUS_100][xceiver_used]);

	return (ETH_ERROR);
}


/*---------------------------------------------------------------------------
| NAME: test_10_base_2_transceiver() 
|
| FUNCTION: 
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|               good: NO_ERROR
| 		bad : otherwise
----------------------------------------------------------------------------*/
int test_10_base_2_transceiver()
{
	if (display(HAVE_WRAP_PLUG_BNC) != YES)
		return(NO_ERROR);

	display (PLUG_XCEIVER_10BASE_2);
	(void) da_display ();

	PUTVAR(xceiver_used, DIAG_SHORT, XCEIVER_T2);

	if (test_tu(EXTERNAL_WRAP_10Base2, TU_ACT_NOP, NULL) == NO_ERROR)
		return(NO_ERROR);

	return (isolate_xceiver_test());
}


/*--------------------------------------------------------------------------
| 	NAME:	test_tu
|
| 	FUNCTION: run test unit and
|		  base on the 'action' indicated do:
|		  - report FRU specified by tu_action_param
|		  - set DA value by tu_action_param
|		  - nothing
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|		display_menugoal()
|		report_frub()
|
| 	RETURNS:
|		NO_ERROR or
|		same return code of exectu()
----------------------------------------------------------------------------*/
int test_tu (int test_unit, int tu_action, int tu_action_param)
{
	int rc;

	eth_tucb.kent.mfg = 0;
	eth_tucb.kent.loop = 1;
	eth_tucb.kent.tu = test_unit;

	set_timer ();
	rc = exectu(da_input.dname, &eth_tucb);
	alarm(0);

        if ((rc  != NO_ERROR) || (alarm_timeout ))
	{
		dt("pcient", DT_MDEC, 4, "test_unit", test_unit, "tu_action",
			tu_action, "tu_action_param", tu_action_param, "rc", rc);
		switch (tu_action) {
		case TU_ACT_REPORT_FRU:
			(void) report_frub(tu_action_param);
			break;
		case TU_ACT_MENUGOAL:
			(void) display_menugoal(tu_action_param);
			break;
		case TU_ACT_DA_SETRC_ERROR:
			DA_SETRC_ERROR(tu_action_param);
			break;
		case TU_ACT_NOP:
		default:		/* do nothing */
			break;
		}
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
|		test_tu
| SIDE EFFECT:
|	change value of device_open_flag to TRUE upon successful open
|
| RETURNS:
|	NO_ERROR:	success
|	call clean_up() and exit DA if fails
----------------------------------------------------------------------------*/
int open_device ()
{
	int	rc;

	if ((rc = test_tu(TU_OPEN, TU_ACT_DA_SETRC_ERROR, DA_ERROR_OPEN)) != NO_ERROR)
		(void) clean_up();

	device_open_flag = TRUE;

	return (rc);
} /* open_device */

/*--------------------------------------------------------------------------
| NAME: close_device
|
| FUNCTION: opening the device 
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|		test_tu
| SIDE EFFECT:
|	change value of device_open_flag to FALSE upon successful close
|
| RETURNS:
|	NO_ERROR:	success
|	call clean_up() and exit DA if fails
----------------------------------------------------------------------------*/
int close_device ()
{
	device_open_flag = FALSE;
	return(test_tu(TU_CLOSE, TU_ACT_MENUGOAL, TU_CLOSE_FAILED));
} /* open_device */


/*--------------------------------------------------------------------------
| NAME: error_log_analysis
|
| FUNCTION: analyze the error log
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:	void
----------------------------------------------------------------------------*/
void error_log_analysis () {
        int ela_rc;
        char crit[128];
        struct errdata errdata_str;

	/* look for irrecoverable errors first */
        sprintf(crit,"%s -N %s -j %X",
                da_input.date, da_input.dname, ERRID_KENT_DOWN);

        ela_rc = error_log_get(INIT,crit,&errdata_str);
	if (ela_rc > 0){
		(void) report_frub(tu_frus[ELA_FRUS][adapter_type]);
		if (error_log_get(TERMI,crit,&errdata_str) != 0) {
			dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			(void) clean_up();
		}
		return;
	}
	else if (ela_rc < 0){
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		(void) clean_up();
	}

	if (error_log_get(TERMI,crit,&errdata_str) != 0) {
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		(void) clean_up();
	}

	/* ela_rc is 0 meaning no irrecovable errors found.
	 * Go on to search for other errors. */
        sprintf(crit,"%s -N %s -j %X,%X,%X",
                da_input.date, da_input.dname,
		ERRID_KENT_ADAP_ERR,
		ERRID_KENT_ERROR_RCVRY,
		ERRID_KENT_PIO);

        ela_rc = error_log_get(INIT,crit,&errdata_str);
	if (ela_rc > 0){
		(void) display_menugoal(ELA_WARNING);
	}
	else if (ela_rc < 0){
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		(void) clean_up();
	}
        error_log_get(TERMI,crit,&errdata_str);
}


/*--------------------------------------------------------------------------
| NAME: display_menugoal
|									   
| FUNCTION: This function will display menugoals used in DA
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/
void display_menugoal(int menugoal_msg)
{
	int	menu_number;
	char	text_buffer[1024];

	switch (menugoal_msg) {
	case ELA_WARNING:
		menu_number = 0x742901;
		break;
	case TU_CLOSE_FAILED:
		menu_number = 0x742902;
		break;
	default:
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
		DA_SETRC_ERROR(DA_ERROR_OTHER);
		return;
	}

	if (catd == (nl_catd) CAT_CLOSED)
		if ((catd = diag_catopen (MF_DPCIENT, 0)) == CATD_ERR )
			(void) clean_up();
		
        sprintf(text_buffer, diag_cat_gets(catd, MSG_MENUGOAL_SET, menugoal_msg),
		menu_number, da_input.dname, da_input.dnameloc);

        catclose(catd);

	menugoal(text_buffer);
}


/*--------------------------------------------------------------------------
| NAME: menu_replace_str
|
| FUNCTION: substitute text for menu
|
| EXECUTION ENVIRONMENT:
|
| RETURNS: None
----------------------------------------------------------------------------*/
void
menu_replace_str(char *msgstr, ASL_SCR_INFO *menuinfo_item, char *str1, char *str2)
{
	if (str2 == NULL)
		sprintf(msgstr, menuinfo_item->text, str1);
	else
		sprintf(msgstr, menuinfo_item->text, str1, str2);
	free(menuinfo_item->text);
	menuinfo_item->text = msgstr;
}


/*--------------------------------------------------------------------------
| NAME: menu_set_head_line
|
| FUNCTION: set up the head line of the DA's menus
|
| EXECUTION ENVIRONMENT:
|
| RETURNS: None
----------------------------------------------------------------------------*/
void
menu_set_head_line(long mnum, struct msglist *msglist, ASL_SCR_INFO *menuinfo)
{
	diag_display(mnum, catd, msglist, DIAG_MSGONLY,
		     ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, menuinfo);

	menu_replace_str(msgstr0, &menuinfo[0], da_input.dname, da_input.dnameloc);
}


/*--------------------------------------------------------------------------
| NAME: display_common_menu
|									   
| FUNCTION: This function will display the default menus
|           which are supported by diagnostics controller
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/
void display_common_menu(int menu_type)
{
	static	char	*text_array[3]={NULL, NULL, NULL};
	int	mnum;

	if (text_array[0] == (char *) NULL){
		text_array[0] = diag_cat_gets(catd, MSG_MAIN_SET, ETHERNET_DEVICE);
		text_array[1] = da_input.dname;
		text_array[2] = da_input.dnameloc;
	}

	switch (menu_type) {
		case CUSTOMER_TESTING_MENU:
			mnum = 0x742001;
			break;
		case ADVANCED_TESTING_MENU:
			mnum = 0x742002;
			break;
		case LOOPMODE_TESTING_MENU:
			mnum = 0x742003;
			break;
		case DEVICE_INITIAL_STATE_FAILURE:
			mnum = 0x742902;
			break;
		default:	/* should never happen */
			dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
                	DA_SETRC_ERROR( DA_ERROR_OTHER );
                	(void) clean_up();
			break;
	}
	diag_display_menu(menu_type, mnum, text_array, da_input.lcount,
			  da_input.lerrors);
} /* display_common_menu */


/*--------------------------------------------------------------------------
| NAME: display							   
|									   
| FUNCTION: This function will display different messages
|           used in DA
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/
int display(int menu_type)
{
	int	rc;

	switch (menu_type) {
		case WHAT_TYPE_ADAPTER:
			menu_set_head_line(0x742004, what_type_adapter,
					   asi_what_type_adapter);
        		menutypes.cur_index = MENU_SELECT_1;
        		rc = diag_display(0x742004, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_what_type_adapter);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		return (DIAG_ITEM_SELECTED (menutypes));
			break;
		case T2_WHAT_TYPE_CONNECTOR:
			menu_set_head_line(0x742005, t2_what_type_connector,
					   asi_t2_what_type_connector);
        		menutypes.cur_index = MENU_SELECT_1;
        		rc = diag_display(0x742005, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_t2_what_type_connector);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		return (DIAG_ITEM_SELECTED (menutypes));
			break;
		case TP_WHAT_TYPE_CONNECTOR:
			menu_set_head_line(0x742006, tp_what_type_connector,
					   asi_tp_what_type_connector);
        		menutypes.cur_index = MENU_SELECT_1;
        		rc = diag_display(0x742006, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_tp_what_type_connector);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		return (DIAG_ITEM_SELECTED (menutypes));
			break;
		/* ------------- BNC WP Menus ------------------- */
		case HAVE_WRAP_PLUG_BNC:
			menu_set_head_line(0x742010, have_wrap_plug_bnc,
					   asi_have_wrap_plug_bnc);
			menu_replace_str(msgstr3, &asi_have_wrap_plug_bnc[3],
				     PN_BNC_WP, NULL);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742010, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_bnc);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case PLUG_BNC_CONNECTOR:
			menu_set_head_line(0x742011, plug_bnc_connector,
					   asi_plug_bnc_connector);
			menu_replace_str(msgstr1, &asi_plug_bnc_connector[1],
				     PN_BNC_WP, NULL);
                        rc = diag_display(0x742011, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_bnc_connector);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
                        break;
		case UNPLUG_WRAP_GENERIC:
			menu_set_head_line(0x742019, unplug_wrap_generic,
					   asi_unplug_wrap_generic);
                        rc = diag_display(0x742019, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_wrap_generic);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
                        break;
		/* ------------- DIX WP Menus ------------------- */
		case HAVE_WRAP_PLUG_DIX:
			menu_set_head_line(0x742020, have_wrap_plug_dix,
					   asi_have_wrap_plug_dix);
			menu_replace_str(msgstr3, &asi_have_wrap_plug_dix[3],
				     PN_DIX_WP, NULL);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742020, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_dix);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case PLUG_DIX_CONNECTOR:
			menu_set_head_line(0x742021, plug_dix_connector,
					   asi_plug_dix_connector);
			menu_replace_str(msgstr1, &asi_plug_dix_connector[1],
				     PN_DIX_WP, NULL);
                        rc = diag_display(0x742021, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_dix_connector);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
                        break;
		/* ------------- Twisted Pair WP Menus ------------------- */
		case HAVE_WRAP_PLUG_TWISTED:
		{
			menu_set_head_line(0x742030, have_wrap_plug_twisted,
					   asi_have_wrap_plug_twisted);
			menu_replace_str(msgstr3, &asi_have_wrap_plug_twisted[3],
				     PN_TP_WP, NULL);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742030, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_have_wrap_plug_twisted);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case PLUG_TP_CONNECTOR:
			menu_set_head_line(0x742031, plug_tp_connector,
					   asi_plug_tp_connector);
			menu_replace_str(msgstr1, &asi_plug_tp_connector[1],
				     PN_TP_WP, NULL);
                        rc = diag_display(0x742031, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_tp_connector);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
			break;
		/* ------------- Transceiver Menus ------------------- */
		case XCEIVER_EXIST:
			menu_set_head_line(0x742040, xceiver_exist,
					   asi_xceiver_exist);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742040, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_xceiver_exist);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case TEST_XCEIVER:
			menu_set_head_line(0x742041, test_xceiver,
					   asi_test_xceiver);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742041, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_test_xceiver);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case WHAT_TYPE_XCEIVER:
			menu_set_head_line(0x742042, what_type_xceiver,
					   asi_what_type_xceiver);
			menu_replace_str(msgstr1, &asi_what_type_xceiver[1],
				     PN_10BASET_XCEIVER, NULL);
			menu_replace_str(msgstr2, &asi_what_type_xceiver[2],
				     PN_10BASE2_XCEIVER, NULL);
        		menutypes.cur_index = MENU_SELECT_3;
        		rc = diag_display(0x742042, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_what_type_xceiver);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case PLUG_XCEIVER_T:
			menu_set_head_line(0x742050, plug_xceiver_twisted,
					   asi_plug_xceiver_twisted);
			menu_replace_str(msgstr1, &asi_plug_xceiver_twisted[1],
				     PN_TP_WP, NULL);
                        rc = diag_display(0x742050, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_xceiver_twisted);
			if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case PLUG_T_XCEIVER_BACK:
			menu_set_head_line(0x742051, plug_t_xceiver_back,
					   asi_plug_t_xceiver_back);
                        rc = diag_display(0x742051, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_t_xceiver_back);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case ISOLATE_XCEIVER:
			menu_set_head_line(0x742052, isolate_xceiver,
					   asi_isolate_xceiver);
			menu_replace_str(msgstr3, &asi_isolate_xceiver[3],
				     PN_DIX_WP, NULL);
        		menutypes.cur_index = NO;
        		rc = diag_display(0x742052, catd, NULL, DIAG_IO,
                		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                		asi_isolate_xceiver);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
			}
        		rc = DIAG_ITEM_SELECTED (menutypes);
			break;
		case UNPLUG_XCEIVER_AND_PLUG_DIX:
			menu_set_head_line(0x742053, unplug_xceiver_and_plug_dix,
					   asi_unplug_xceiver_and_plug_dix);
			menu_replace_str(msgstr1, &asi_unplug_xceiver_and_plug_dix[1],
				     PN_DIX_WP, NULL);
                        rc = diag_display(0x742053, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_xceiver_and_plug_dix);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case UNPLUG_DIX_AND_PUT_TP_XCEIVER_BACK:
			menu_set_head_line(0x742054, unplug_dix_and_put_tp_xceiver_back,
					   asi_unplug_dix_and_put_tp_xceiver_back);
                        rc = diag_display(0x742054, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_dix_and_put_tp_xceiver_back);

                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case PLUG_XCEIVER_10BASE_2:
			menu_set_head_line(0x742060, plug_xceiver_10base_2,
					   asi_plug_xceiver_10base_2);
			menu_replace_str(msgstr1, &asi_plug_xceiver_10base_2[1],
				     PN_BNC_WP, NULL);
                        rc = diag_display(0x742060, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_xceiver_10base_2);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case PLUG_GENERIC_XCEIVER_BACK:
			menu_set_head_line(0x742061, plug_generic_xceiver_back,
					   asi_plug_generic_xceiver_back);
                        rc = diag_display(0x742061, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_generic_xceiver_back);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		case UNPLUG_DIX_AND_PUT_GENERIC_XCEIVER_BACK:
			menu_set_head_line(0x742064, unplug_dix_and_put_generic_xceiver_back,
					   asi_unplug_dix_and_put_generic_xceiver_back);
                        rc = diag_display(0x742064, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_dix_and_put_generic_xceiver_back);
                        if (check_asl_stat(rc) == QUIT) {
				(void) clean_up();
		        }
                        break;
		default:	/* should never happen */
			dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
                	DA_SETRC_ERROR( DA_ERROR_OTHER );
                	(void) clean_up();
			break;
		}
	}
	return(rc);
}/* display */


/*--------------------------------------------------------------------------
| NAME: report_frub
|
| FUNCTION: The function will add fru bucket into database
|
| EXECUTION ENVIRONMENT:
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/
void report_frub (short frub_idx)
{
	struct fru_bucket *frub_addr = &eth_frus[frub_idx];

        /* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);

        if (insert_frub(&da_input, frub_addr) != NO_ERROR ||
            addfrub(frub_addr) != NO_ERROR) 
        {
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                (void) clean_up();
        }
	else
		DA_SETRC_STATUS (DA_STATUS_BAD);
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
        (void) clean_up();
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
 | FUNCTION: Poll the keyboard input to see whether user want to quit
 |
 | EXECUTION ENVIRONMENT:
 |
 | NOTES:  Other local routines called --
 |	   check_asl_stat()
 |
 | RETURNS:
 |		QUIT ---- user entered CANCEL,EXIT
 |		NO_ERROR -otherwise
 --------------------------------------------------------------------------*/
int poll_kb_for_quit()
{
	if (check_asl_stat( diag_asl_read
		(ASL_DIAG_LIST_CANCEL_EXIT_SC, FALSE, NULL))==QUIT) 
		return(QUIT);
	else 
		return(NO_ERROR);
}


/*--------------------------------------------------------------------------
 | NAME: timeout()
 |
 | FUNCTION: Designed to handle timeout interrupt handlers.
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
        if (device_open_flag == TRUE)
                close_device();

        /* terminate odm */
        term_dgodm();

        if ( da_input.console == TRUE )
        {
		if (da_input.exenv == EXENV_SYSX)
			clrdainput();
                if (catd != CATD_ERR)
                        catclose (catd);
                if (asl_flg != NOT_GOOD)
                        diag_asl_quit (NULL);
        }

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

	/* initialize odm	*/
	odm_flg = init_dgodm();

	/* get TMInput object from database	*/
	rc = getdainput (&da_input);
	if (odm_flg == -1 || rc != 0) 
	{
		dt("pcient", DT_DEC, "DA_ERROR_OTHER, line:", __LINE__);
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		(void) clean_up();
	}

	if ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl  & open catalog	*/
		if (da_input.loopmode == LOOPMODE_INLM)
			asl_flg = diag_asl_init (ASL_INIT_DEFAULT);
		else
			asl_flg = diag_asl_init ("NO_TYPE_AHEAD");
		catd = diag_catopen (MF_DPCIENT, 0); 
		if ( (asl_flg == NOT_GOOD) || (catd == CATD_ERR ))
			(void) clean_up();
	}

	if (da_input.loopmode & (LOOPMODE_NOTLM || LOOPMODE_ENTERLM))
		da_odm_vars (INITIALIZE_VARS);
	else 
		da_odm_vars (READ_VARS);
}

/*----------------------------------------------------------------------------
| NAME: da_odm_vars 
|
| FUNCTION: Initialize or read the necessary davars
|
| EXECUTION ENVIRONMENT:
|
| NOTES:
|       if op is SETH_INIT_OP then initialize davars
|       else if op is SETH_GET_OP then read davars
|
| RETURNS:
|       None
----------------------------------------------------------------------------*/
void da_odm_vars(int op)
{
        switch(op) {
        case INITIALIZE_VARS:
                PUTVAR(adapter_type, DIAG_SHORT, GENERIC_ADAPTER);
                PUTVAR(wrap_plug, DIAG_SHORT, WP_NONE);
                PUTVAR(xceiver_used, DIAG_SHORT, XCEIVER_NONE);
                PUTVAR(xceiver_removed_flag, DIAG_SHORT, FALSE);
                break;

        case READ_VARS:
                getdavar(da_input.dname, "adapter_type",
			DIAG_SHORT, &adapter_type);
                getdavar(da_input.dname, "wrap_plug",
			DIAG_SHORT, &wrap_plug);
                getdavar(da_input.dname, "xceiver_used", 
			DIAG_SHORT, &xceiver_used);
                getdavar(da_input.dname, "xceiver_removed_flag", 
			DIAG_SHORT, &xceiver_removed_flag);
		break;
	}
}
