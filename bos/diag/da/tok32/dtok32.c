static char sccsid[] = "@(#)20	1.3  src/bos/diag/da/tok32/dtok32.c, dtok32, bos411, 9438A411a 9/16/94 15:50:37";
/*
 *   COMPONENT_NAME: DALCTOK
 *
 *   FUNCTIONS: Having_cable_tested
 *		Which_type_of_cable
 *		cable_only_test
 *		check_asl_stat
 *		clean_malloc
 *		clean_up
 *		clear_variables
 *		da_display
 *		error_log_analysis
 *		exectu
 *		general_initialize
 *		interactive_tests
 *		intr_handler
 *		is_network_connected
 *		main
 *		test_network
 *		non_interactive_tests
 *		performing_test
 *		poll_kb_for_quit
 *		report_frub
 *		test_display
 *		test_tu
 *		timeout
 *		tu_close
 *		tu_open
 *		verify_rc
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
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
#include <locale.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_exit.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <diag/bit_def.h>
#include <diag/atu.h>
#include "dtok32_msg.h"
#include "dtok32.h"
#include "mpstu_type.h"


extern	nl_catd diag_catopen();
extern 	getdainput();		/* gets input from ODM 			*/
extern 	long getdamode();	/* transforms input into a mode 	*/
extern 	int errno;		/* reason device driver won't open 	*/
extern 	int exectu();
extern 	int diag_cfg_errno;


/* davar used */
short   cable_tested= FALSE;		
short   network_is_connected= FALSE;  	
int	alarm_timeout=FALSE;
struct  sigaction  invec;   	/* interrupt handler structure  */	

short	asl_flg = NOT_GOOD; 	/* return code from diag_asl_init() */

int	prior_state = NOT_GOOD;/* state prior to DA			*/
struct 	tm_input da_input;	/* initialized by getdainput 		*/
nl_catd catd;			/* pointer to message catalog  		*/
char	dname[NAMESIZE]; 	/* device name to be tested 		*/
TUTYPE 	tokenring_tucb;
int	cable_type=0;

char	*new_out, *new_err;
int     result;

ADAPTER_STRUCT	*adapter_info;
char	open_flag = FALSE;


int 	performing_test();
int 	interactive_tests();
int 	non_interactive_tests();
int	unplug_cable_procedure ();
int 	test_tu(int,int);
int 	verify_rc(int*,int);
int 	poll_kb_for_quit();
int 	Having_cable_tested();
int 	check_asl_stat(int);
void	intr_handler(int);	
void 	error_log_analysis ();
int	test_display();
void 	da_display();
void 	report_frub();
void 	clean_up();
void 	timeout(int);
void	clear_variables();
void	general_initialize ();
void	clean_malloc();
int	cable_only_test();
int	test_network();
int	tu_open ();
int	tu_close ();
void	report_fru_cable();

/*--------------------------------------------------------------*/
/*	Messages and menus					*/
/*--------------------------------------------------------------*/

static ASL_SCR_TYPE	menutypes=DM_TYPE_DEFAULTS;
char *msgstr;
char *msgstr1;


/*......................................................................*/
/* 	Function 	  	main					*/
/*......................................................................*/

main (int argc,char **argv)
{
	int	rc=0;			/* return code 			*/
	int	diskette_mode;
	int	diskette_based;

	general_initialize ();

	/* executing test if running problem determination	*/
	/* or repair mode					*/
	if (( da_input.dmode & (DMODE_PD | DMODE_REPAIR) ) &&
 		(!(da_input.loopmode & LOOPMODE_EXITLM)))

	{
		if  ( da_input.console == CONSOLE_TRUE) 
			da_display ();
	
		rc = performing_test ();
				
	}
	/* if running in LOOPMODE_EXITLM or NOTLM then 	*/
	/* check if the cable is hooked before running  */
	/* DA. If yes, then ask the user to put the   	*/
	/* cable back into the port.			*/

	getdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);

	if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
	    ( da_input.loopmode & LOOPMODE_NOTLM )) && (cable_tested) 
			&& ( da_input.console == CONSOLE_TRUE))
	{
		/* Ask the user to unplug the wrap plug	*/
		test_display(PUT_CABLE_BACK);
		da_display ();
		clear_variables();
	}	
	 
	/* if running problem determination, or error log analyis 	*/
	/* running error log analysis					*/

	if  (rc == GOOD && (( da_input.dmode ==  DMODE_PD) 
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
|		test_display()
|
| 	RETURNS:
|		None
----------------------------------------------------------------------------*/

void da_display ()
{
	if ( da_input.loopmode == LOOPMODE_INLM) 
		test_display ( TESTING_LOOPMODE);
	else if ( da_input.advanced == ADVANCED_TRUE)
		test_display ( TESTING_ADVANCED_MODE);
	else
		test_display (TESTING_REGULAR) ;
}



/*--------------------------------------------------------------------------
| 	NAME: Performing_test
|
| 	FUNCTION: Invokes appropriate test units
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
	int	rc =GOOD;

	/* calling tu open for testing		*/
	rc = tu_open ();
	if (rc != GOOD)
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}
	else
	{
		open_flag = TRUE;
	}
	rc =  non_interactive_tests ();

	/* system true means non-interactive test 	*/

	if (( rc == GOOD) && ( da_input.advanced == ADVANCED_TRUE) 
			&& ( da_input.console == CONSOLE_TRUE)
			&& ( da_input.system == SYSTEM_FALSE))
		rc = interactive_tests();
	return (rc);
}



/*--------------------------------------------------------------------------
| 	NAME: interactive_tests()
|
| 	FUNCTION: Run interactive tests
|
| 	EXECUTION ENVIRONMENT:
|
| 	NOTES:  Other local routines called --
|		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
|
| 	RETURNS:
|		GOOD - test passed
|		otherwise -test failed
----------------------------------------------------------------------------*/
int interactive_tests()
{
	int rc = GOOD;

	if (da_input.loopmode & LOOPMODE_INLM)
        {
		/* cable_tested check 	*/
		getdavar(da_input.dname, "cable_tested", 
				DIAG_SHORT, &cable_tested);

		if (cable_tested )
		{
			rc = cable_only_test ();
			if (rc != GOOD)
			{
				report_fru_cable (tokenring_tucb.header.tu, rc);
				return (BAD);
			}
		}
		getdavar(da_input.dname, "network_is_connected", 
				DIAG_SHORT, &network_is_connected);
		if (network_is_connected)
		{
			rc = test_network ();
			if (rc != GOOD)
			{
				/* report FRU */
				report_fru_cable (tokenring_tucb.header.tu, rc);
				return (BAD);
			}
		}

        }

	if ((da_input.loopmode & LOOPMODE_NOTLM) ||
		(da_input.loopmode & LOOPMODE_ENTERLM))
        {
		rc = cable_only_test ();
		if (rc != GOOD)
		{
			/* Ask the user which type of cable is tested */
			rc = Which_type_of_cable () ;
			cable_type = rc;
			if ( rc == -1)
				return (QUIT);
			/* cable testing is desired	*/
			if  (Having_cable_tested(rc) == YES)
			{
				/* ask the user to put the cable in 	*/
				switch (cable_type)
				{
					case TYPE_3:
				 	  rc = test_display (PLUG_CABLE_TYPE_3);
					  break;
					case NOT_TYPE_3:
				 	  rc = test_display
						 (PLUG_CABLE);
					default:
					  break;
				}
				if ((check_asl_stat (rc)) == QUIT) 
				{
					clear_variables();
					clean_up();
				}
				da_display();
				rc = cable_only_test();
				if (rc != GOOD)
				{
					/* ask the user to unplug Token-Ring */
					/* network cable from network 	     */
					rc = test_display (UNPLUG_CABLE);
					if ((check_asl_stat (rc)) == QUIT) 
					{
						clear_variables();
						clean_up();
					}
					da_display();
					/* run the test again */
					rc = cable_only_test();
					if (rc != GOOD)
					{
						/* report FRU 	*/
						report_fru_cable 
					      	    (tokenring_tucb.header.tu, rc);
						return (rc);
					}

				}
			}
			else	
				return (GOOD);
		}
		da_display();
		rc = test_network ();
		if (rc != GOOD)
		{
			/* ask the user if the network is */
			/* hooked up			  */
			if  (is_network_connected()== YES)
			{
				/* report FRU */
				report_fru_cable (tokenring_tucb.header.tu, rc);
				return (rc);
			}
			else
			{
				/* set database indicated  */
				/* no network 		   */
				return (0);
			}
		}
        }
	return (rc);
}


/*--------------------------------------------------------------------------
 | 	NAME: Having_cable_tested
 |
 | 	FUNCTION: Ensure user have the wrap plug and cable before testing 
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		YES --- Yes, user chooses to test cable 
 |		NO,QUIT --- otherwise
 ---------------------------------------------------------------------------*/

int Having_cable_tested(cable_type)
int	cable_type;
{
	int rc;
	char	part_number[20];


	switch (cable_type)
	{
		case 1:
		sprintf(part_number, (char *)diag_cat_gets 
			( catd, 1, PART_NUMBER_1));
		break;

		case 2:
		sprintf(part_number, (char *)diag_cat_gets 
			( catd, 1, PART_NUMBER_2));
		break;
		default:
		break;
	}

	/* asking the user if he wants to test cable    */

        rc = diag_display(0x85C005, catd,
                       testing_cable_yes_no, DIAG_MSGONLY,
                       ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                       asi_testing_cable_yes_no);
        sprintf(msgstr, asi_testing_cable_yes_no[0].text,
        		da_input.dname, da_input.dnameloc);
       	free (asi_testing_cable_yes_no[0].text);
       	asi_testing_cable_yes_no[0].text = msgstr;

        sprintf(msgstr1, asi_testing_cable_yes_no[3].text,part_number);
       	free (asi_testing_cable_yes_no[3].text);
       	asi_testing_cable_yes_no[3].text = msgstr1;


       	/* set the cursor points to NO */
       	menutypes.cur_index = NO;
       	rc = diag_display(0x85C005, catd, NULL, DIAG_IO,
       		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
       		asi_testing_cable_yes_no);

	if ((check_asl_stat (rc)) == QUIT) 
	{
		clear_variables();
		clean_up();
	}

   	rc = DIAG_ITEM_SELECTED (menutypes);
	if      /* if user does not want to test cable */ 
       		( rc != YES)
	{
		cable_tested= FALSE;
           	putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
   			&cable_tested);
           	return (NO);
	}

	/* setting cable_tested variable in data base to true */
	cable_tested= TRUE;
	putdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);

	return (YES);


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
int	rc, rc1;


	memset (&tokenring_tucb, sizeof (TUTYPE), 0);
	tokenring_tucb.header.mfg = 0;
	tokenring_tucb.header.loop = 1;
	strcpy (tokenring_tucb.header.pattern,"|none|");
	tokenring_tucb.header.pat_size= 32;
	

	start = tus_test;  

	for (; start->tu != -1; ++start) {
		rc = test_tu(start->tu,start->fru);
		if ( rc != GOOD)
			break;
		 if (da_input.console == TRUE )
		{
			rc1 =  poll_kb_for_quit();
			if (rc1 == QUIT)
				break;
		}
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

	tokenring_tucb.header.tu = test_unit;
	rc = exectu(da_input.dname, &tokenring_tucb, adapter_info); 

	if (alarm_timeout ) 
	{
		report_frub (fru_index);
		return (NOT_GOOD);
	}

	if (rc  != GOOD) 
	{
	   	if (verify_rc(tu_rctbl[fru_index],rc) == TRUE ) 
	   	{
			report_frub (fru_index);
           	}
	}
	return(rc);
}


/*--------------------------------------------------------------------------
| NAME: verify_rc
|
| FUNCTION: verify the return code from exectu() of the specified
|	    test unit is a valid return code
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	TRUE -- is in valid return code range
|	FALSE - is not a valid return code
----------------------------------------------------------------------------*/
verify_rc(int *rc_ptr,int rc)
{

	for (; *rc_ptr; rc_ptr++) 
	{
		if (*rc_ptr == rc )
			return(TRUE); 
	}

	DA_SETRC_ERROR (DA_ERROR_OTHER);
	return(FALSE);
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
int rc;

	if (open_flag == TRUE)
		tu_close ();

rc = diagex_initial_state (da_input.dname);
if (rc!= 0) 
{
	DA_SETRC_ERROR (DA_ERROR_OTHER);
}


        /* terminate odm */
        term_dgodm();

        if ( da_input.console == TRUE )
        {
                if (catd != CATD_ERR)
                        catclose (catd);
                if (asl_flg != NOT_GOOD)
                        diag_asl_quit (NULL);

        }
        clean_malloc();

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

        /*..........................................................*/
        /* Look for an occurrence of any of the following errors in */
        /* the error log.  If one exists, exit_with_fru for ELA.    */
        /*..........................................................*/
        sprintf(crit,"%s -N %s -j %X",
                  da_input.date,
                  da_input.dname, 0xFFFFFFF);

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
        if (ela_get_rc >0)
        {
		/* reporting FRU	*/
                DA_SETRC_ERROR (DA_STATUS_BAD);
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
    /*    if ( da_input.console == CONSOLE_TRUE ) 
                diag_asl_clear_screen(); */
        clean_up();
}


/*--------------------------------------------------------------------------
| NAME: test_display							   
|									   
| FUNCTION: This function will display three different message. Advanced,
|	    loopmode and regular testing menu.
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|	None
----------------------------------------------------------------------------*/

int test_display (menu_type)
int	menu_type;
{
	int	rc=0;
	switch (menu_type)
	{
		case TESTING_LOOPMODE:
			 	diag_msg_nw ( 0x85C003, catd, 
			   	1, TESTING_LOOPMODE,da_input.dname, 
				da_input.dnameloc, da_input.lcount,
				da_input.lerrors);
			break;
		case TESTING_ADVANCED_MODE:
				diag_msg_nw ( 0x85C002, catd, 
		   		1, TESTING_ADVANCED_MODE,da_input.dname,
				da_input.dnameloc); 

			break;
		case TESTING_REGULAR: 
				diag_msg_nw ( 0x85C001, catd, 
			   		1, TESTING_REGULAR,da_input.dname,
				da_input.dnameloc);
			break;

		case PLUG_CABLE_TYPE_3:
			/* ask the user to plug the cable in 	*/
                        rc = diag_display(0x85C007,
                                catd, plug_cable_type_3,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_cable_type_3);
                        sprintf(msgstr, asi_plug_cable_type_3[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_plug_cable_type_3[0].text);
                        asi_plug_cable_type_3[0].text = msgstr;
                        sprintf(msgstr1, asi_plug_cable_type_3[1].text,
                                da_input.dnameloc);
                        free (asi_plug_cable_type_3[1].text);
                        asi_plug_cable_type_3[1].text = msgstr1;

                        rc = diag_display(0x85C007, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_cable_type_3);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				clear_variables();
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        cable_tested= TRUE;
                        putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                                &cable_tested);
                        break;

		case PLUG_CABLE:
			/* ask the user to plug the cable in 	*/
                        rc = diag_display(0x85C007,
                                catd, plug_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_plug_cable);
                        sprintf(msgstr, asi_plug_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_plug_cable[0].text);
                        asi_plug_cable[0].text = msgstr;
                        sprintf(msgstr1, asi_plug_cable[1].text,
                                da_input.dnameloc);
                        free (asi_plug_cable[1].text);
                        asi_plug_cable[1].text = msgstr1;

                        rc = diag_display(0x85C007, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_plug_cable);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				clear_variables();
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        cable_tested= TRUE;
                        putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                                &cable_tested);
                        break;


		case PUT_CABLE_BACK:
			/* ask the user to unplug wrap from the cable 	*/
			/* and put the cable back into the network	*/
                        rc = diag_display(0x85C010,
                                catd, put_cable_back,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_cable_back);
                        sprintf(msgstr, asi_put_cable_back[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_cable_back[0].text);
                        asi_put_cable_back[0].text = msgstr;
                        sprintf(msgstr1, asi_put_cable_back[1].text,
                                 da_input.dnameloc);
                        free (asi_put_cable_back[1].text);
                        asi_put_cable_back[1].text = msgstr1;

                        rc = diag_display(0x85C010, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_cable_back);

                        /* setting cable_tested variable in database to true */
                        cable_tested= FALSE;
                        putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                                &cable_tested);
                        break;

		case UNPLUG_CABLE:
			/* ask the user to plug the cable  	*/
                        rc = diag_display(0x85C009,
                                catd, unplug_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_unplug_cable);
                        sprintf(msgstr, asi_unplug_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_unplug_cable[0].text);
                        asi_unplug_cable[0].text = msgstr;
                        sprintf(msgstr1, asi_unplug_cable[1].text,
                                da_input.dnameloc);
                        free (asi_unplug_cable[1].text);
                        asi_unplug_cable[1].text = msgstr1;

                        rc = diag_display(0x85C009, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_cable);

                        if ((check_asl_stat (rc)) == QUIT) 
			{
				clear_variables();
				clean_up();
			}

                        /* setting cable_tested variable in database to true */
                        cable_tested= TRUE;
                        putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                                &cable_tested);
                        break;

		default:
			break;

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
	struct fru_bucket *frub_addr = &tokenring_frus[frub_idx];


	strcpy (frub_addr->dname, da_input.dname);
       	if (insert_frub(&da_input, frub_addr) != GOOD ||
	    addfrub(frub_addr) != GOOD) {
		DA_SETRC_ERROR( DA_ERROR_OTHER );
		clean_up();
	}
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
 |	GOOD -otherwise
 --------------------------------------------------------------------------*/
int poll_kb_for_quit()
{
	int 	rc;
	int 	rc1;

	rc =  diag_asl_read (ASL_DIAG_LIST_CANCEL_EXIT_SC, 0, 0);

	rc1 = check_asl_stat(rc);
	if (rc1 == QUIT ) 
		return(QUIT);
	else 
		return(GOOD);
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


/*--------------------------------------------------------------------------
| NAME: clear_variables
|
| FUNCTION:  this procedure will set all the variables used by DA to FALSE
|	    
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS: NONE
----------------------------------------------------------------------------*/

void clear_variables()
{
	cable_tested = FALSE;
	putdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);
	network_is_connected= FALSE;
	putdavar(da_input.dname, "network_is_connected", DIAG_SHORT, 
		&network_is_connected);
}

/*--------------------------------------------------------------------------
* NAME: test_config
*
* FUNCTION: If device fails to configure, function determins if devices is
	    busy.  Then returns the appropriate information to the
	    diagnostic controller.
*
* EXECUTION ENVIRONMENT:
*       Called by the main program when the opening of the device driver
*       fails for the first time.
*
---------------------------------------------------------------------------*/

test_config()
{

	switch (diag_cfg_errno)
	{
		case EBUSY:             /* DD has already been in use */
        	    	DA_SETRC_TESTS(DA_TEST_NOTEST);
           	    	DA_SETRC_ERROR(DA_ERROR_OPEN);
            		clean_up();
            	break;
        	default:
            		DA_SETRC_TESTS(DA_TEST_NOTEST);
            		DA_SETRC_ERROR(DA_ERROR_OTHER);
            		clean_up();
            	break;
        } /* end switch */

} /* end test_config */

/*--------------------------------------------------------------------------
| NAME: general_initialize ()
|
| FUNCTION:  This message will general initialization of variables
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

	setlocale(LC_ALL,"");

	/* initializing interrupt handler */    

	invec.sa_handler = intr_handler;
	sigaction(SIGINT, &invec, (struct sigaction *)NULL);
/*	invec.sa_handler = timeout;
	sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
*/

	/*..............................*/
	/* General Initialization	*/
	/*..............................*/

	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);
	DA_SETRC_USER (DA_USER_NOKEY);

	/* initialize odm	*/
	/* do not need to check because this routine all the time return 0 */
	/* look at file diag_sdm.c					   */
	init_dgodm();

	/* get da input 	*/
	rc = getdainput(&da_input); 
	if (rc!= 0) 
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}

rc = diagex_cfg_state (da_input.dname);
if (rc!= 0) 
	test_config ();

	if 	/* running under console */
	   ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl 		*/
		asl_flg = diag_asl_init ( "NO_TYPE_AHEAD" );
		catd = diag_catopen ("dtok32.cat",0); 
		if (asl_flg == NOT_GOOD || catd == CATD_ERR)
			clean_up();
	}

        msgstr = (char *) malloc (1200 *sizeof (char));
        msgstr1 = (char *) malloc (1200 *sizeof (char));
	adapter_info = (ADAPTER_STRUCT *)malloc (sizeof (ADAPTER_STRUCT));

        if((msgstr == (char *) NULL) || (msgstr1 == (char *) NULL)
		||(adapter_info == (ADAPTER_STRUCT *)NULL)) 
        {
                clean_up();
        }
}
/*--------------------------------------------------------------------------
| NAME: clean_malloc ()
|
| FUNCTION:  This function will free all memory allocation by 
|	     general_initilization () 
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
	if (adapter_info != (ADAPTER_STRUCT *)NULL) 
		free (adapter_info);
}
/*-------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 | 	NAME: Having_cable_tested
 |
 | 	FUNCTION: Ensure user have the wrap plug and cable before testing 
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		YES --- Yes, user chooses to test cable 
 |		NO,QUIT --- otherwise
 ---------------------------------------------------------------------------*/

int Which_type_of_cable()
{
	int rc;
	/* asking the user if he wants to test cable    */

        rc = diag_display(0x85C004, catd,
                       which_type_of_cable, DIAG_MSGONLY,
                       ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                       asi_which_type_of_cable);
        sprintf(msgstr, asi_which_type_of_cable[0].text,
        		da_input.dname, da_input.dnameloc);
       	free (asi_which_type_of_cable[0].text);
       	asi_which_type_of_cable[0].text = msgstr;


       	/* set the cursor points to NO */
       	menutypes.cur_index = NO;
       	rc = diag_display(0x85C004, catd, NULL, DIAG_IO,
       		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
       		asi_which_type_of_cable);

	if ((check_asl_stat (rc)) == QUIT) 
	{
		return (-1);
	}

   	rc = DIAG_ITEM_SELECTED (menutypes);
	return (rc);


}
/*--------------------------------------------------------------------------
 | 	NAME: cable_only_test 
 |
 | 	FUNCTION: running all test units that the cable is required     
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		GOOD :	0
 |		BAD:    Other return code
 ---------------------------------------------------------------------------*/
int	cable_only_test ()
{
	struct	cable_tu_fru_pair *start;
	int	rc;

	start = cable_tus_test;  

	for (; start->tu != -1; ++start) 
	{
		tokenring_tucb.header.tu = start->tu;
		rc = exectu(da_input.dname,  
			&tokenring_tucb, adapter_info); 

		if (alarm_timeout)
			break;
		else if ( rc != GOOD)
			break;
		if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
		{
			rc = QUIT;
			break;
		}
	}

	return(rc);
}

/*--------------------------------------------------------------------------
 | 	NAME: report_fru_cable
 |
 | 	FUNCTION: running all test units that the cable is required     
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		GOOD :	0
 |		BAD:    Other return code
 ---------------------------------------------------------------------------*/
void report_fru_cable(test_unit, rc)
int	test_unit, rc;
{
	switch (test_unit)
	{
		case 4:
	   		if (verify_rc((int *)&tu004, rc) == TRUE ) 
				report_frub (FRU_104);
			break;
		case 5:
	   		if (verify_rc((int *)&tu005, rc) == TRUE ) 
				report_frub (FRU_105);
			break;
		case 6:
	   		if (verify_rc((int *)&tu006, rc) == TRUE ) 
				report_frub (FRU_106);
			break;
		case 7:
			report_frub (FRU_107);
			break;
		default:
			break;
	}
}
/*--------------------------------------------------------------------------
 | 	NAME: test_network 
 |
 | 	FUNCTION: running all test unit that require that cable is hooked
 |   	   	  to the network 
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		GOOD :	0
 |		BAD:    Other return code
 ---------------------------------------------------------------------------*/
int	test_network ()
{
	int	rc;

	tokenring_tucb.header.tu = 7;
	rc = exectu(da_input.dname, &tokenring_tucb, adapter_info); 

	return (rc);
}
/*--------------------------------------------------------------------------
 | 	NAME: is_network_connected 
 |
 | 	FUNCTION: ask the user if the network is functional     
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		YES --- Yes, user chooses to test cable 
 |		NO,QUIT --- otherwise
 ---------------------------------------------------------------------------*/

int is_network_connected()
{

	int	rc;

	/* ask the user if the Token-Ring network is funtional	*/

        rc = diag_display(0x85C006, catd,
                       network_connected, DIAG_MSGONLY,
                       ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                       asi_network_connected);
        sprintf(msgstr, asi_network_connected[0].text,
        		da_input.dname, da_input.dnameloc);
       	free (asi_network_connected[0].text);
       	asi_network_connected[0].text = msgstr;


       	/* set the cursor points to NO */
       	menutypes.cur_index = NO;
       	rc = diag_display(0x85C006, catd, NULL, DIAG_IO,
       		ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
       		asi_network_connected);

	if ((check_asl_stat (rc)) == QUIT) 
	{
		clear_variables();
		clean_up();
	}

   	rc = DIAG_ITEM_SELECTED (menutypes);
	if      /* if user does not want to test cable */ 
       		( rc != YES)
	{
		network_is_connected= FALSE;
           	putdavar(da_input.dname, "network_is_connected", DIAG_SHORT,
   			&network_is_connected);
           	return (NO);
	}

	/* setting cable_tested variable in data base to true */
	network_is_connected= TRUE;
	putdavar(da_input.dname, "network_is_connected", 
		DIAG_SHORT, &network_is_connected);

	return (YES);


}

/*--------------------------------------------------------------------------
 | 	NAME: tu_open         
 |
 | 	FUNCTION: calling test unit to open tu  
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		GOOD :	0
 |		BAD:    Other return code
 ---------------------------------------------------------------------------*/
int	tu_open()
{
	int rc;
	tokenring_tucb.header.tu = TU_OPEN;
	tokenring_tucb.header.mfg = 0;
	tokenring_tucb.header.loop = 1;

	rc = exectu (da_input.dname, &tokenring_tucb, adapter_info);  

	
	return (rc);
}

/*--------------------------------------------------------------------------
 | 	NAME: tu_close        
 |
 | 	FUNCTION: calling test unit to close
 |
 | 	EXECUTION ENVIRONMENT:
 |
 | 	NOTES:  Other local routines called --
 |		Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 | 	RETURNS:
 |		GOOD :	0
 |		BAD:    Other return code
 ---------------------------------------------------------------------------*/
int	tu_close()
{
	int rc;
	tokenring_tucb.header.tu = TU_CLOSE;
	tokenring_tucb.header.mfg = 0;
	tokenring_tucb.header.loop = 1;

	rc = exectu (da_input.dname, &tokenring_tucb, adapter_info); 
	
	return (rc);
}
