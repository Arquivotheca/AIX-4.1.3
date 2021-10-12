static char sccsid[] = "@(#)89	1.22  src/bos/diag/da/sol/doptics.c, dasol, bos411, 9430C411a 7/18/94 15:29:23";
/*
 *   COMPONENT_NAME: DASOL
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *		check_asl_stat
 *		check_rc_1_2
 *		check_rc_3
 *		check_rc_4
 *		clean_up
 *		configure_own_device1648
 *		display_menu
 *		error_log_analysis1191
 *		interactive_test
 *		intr_handler
 *		main
 *		non_interactive_test
 *		open_device
 *		optics_da_display
 *		performing_test
 *		report_frub
 *		run_tu1
 *		run_tu2
 *		run_tu3
 *		run_tu4
 *		slc_exist
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1994
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
#include <cf.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/cfgodm.h>
#include "diag/tm_input.h"
#include <asl.h>
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/bit_def.h"
#include "optics.h"
#include "optics_msg.h"
#include "soltst.h"
#include "sol_errids.h"
#include <locale.h>


extern	diag_exec_source();
extern 	nl_catd diag_catopen();
extern 	getdainput();		/* gets input from ODM 			*/
extern 	long getdamode();	/* transforms input into a mode 	*/
extern 	disp_menu();		/* displays menus 			*/
extern 	user_quit();		/* cleanup routine 			*/
extern 	int exectu();		/* test unit interface 			*/
extern 	int errno;		/* reason device driver won't open 	*/
void 	intr_handler(int);	/* da interrupt handler 		*/
int	child_state= NOT_CONFIG; /* device state 			*/
int	parent_state= NOT_CONFIG; /* parent of device state: op 	*/
int	grand_parent_state= NOT_CONFIG; /* grand parent of device : slc */	
char	child[NAMESIZE];	/* device name : op0, op1, op2 or op3   */
char	parent[NAMESIZE];	/* parent	: otp0 or otp1		*/
char	grand_parent[NAMESIZE]; /* grand parent : slc0 or slc1		*/
char	*test_port[8]= {"op0","op1", "op2", "op3", "op4", "op5", "op6", "op7"};
char	base_port[]= "op0";	
char	second_port[]= "op0";	
int	define_child=FALSE;
int	second_port_exist=FALSE;
int	define_parent=FALSE;
int	define_second_port=FALSE;
char	*test_parent[4]={"otp0", "otp1", "otp2", "otp3"};
char	base_parent[]="otp0";
struct 	tm_input da_input;	/* initialized by getdainput 		*/
long 	da_mode;		/* diagnostic modes 			*/
int 	filedes=ERROR;		/* file descriptor from device driver 	*/
nl_catd catd;			/* pointer to message catalog  		*/
char	dname[NAMESIZE]; 	/* device name to be tested 		*/
short	network_cable= FALSE;	/* network cable is assumed not hooked up */
short	nowrap = FALSE;		/* if the user choose test with wrap plug */	
short	wrap_plug = FALSE;	/* if the user choose test with wrap plug */	
char	*devname;		/* name of device to be tested 		*/
struct tucb_t	tucb;		/* test unit control block 		*/
int	slc_device= FALSE;	/* the variable for slc 		*/
int	display_rc;
int	data_file_exist = FALSE; /* data file for debugging purpose 	*/
int	diskette_mode;
int	diskette_based;
int	otp_card_exist=FALSE;	/* the presence flag of OTP card	*/
int 	found_child=FALSE;	
int 	found_parent=FALSE;	
struct	CuDv	child_cudv, parent_cudv, second_child_cudv;
char	define_method[256];
char	undefine_method[256];
struct	PdDv	pddv;

void error_log_analysis ();
int display_menu();
void optics_da_display();


/* fru bucket structures for the optics 2 port  adapter */

/* this FRU is for running op				*/
struct fru_bucket frub101 =
  	{"", FRUB1, 0x861, 0x101, NEW_OPTICS_MSG_101,
		{
			{60, "", "", 0, 
				DA_NAME, NONEXEMPT},
			{40, "", "", 0, 
				PARENT_NAME, NONEXEMPT},

		},
	};	

/* this FRU is for running slc with OTP card presence	*/
struct fru_bucket frub102 =
  	{"", FRUB1, 0x861, 0x101, NEW_OPTICS_MSG_101,
		{
			{60, "", "", 0, 
				CHILD_NAME, NONEXEMPT},
			{40, "", "", 0, 
				DA_NAME, NONEXEMPT},

		},
	};	

/* this FRU is for running slc without OTP card presence	*/
struct fru_bucket frub103 =
  	{"", FRUB1, 0x861, 0x103, NEW_OPTICS_MSG_101,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},

		},
	};	


/* This FRU for running op		*/

struct fru_bucket frub201 =
	{"", FRUB1, 0x861, 0x201, NEW_OPTICS_MSG_201,
		{
			{60, "", "",  0, 
				DA_NAME, NONEXEMPT},
			{40, "", "",  0, 
				PARENT_NAME, NONEXEMPT},
		},
	};

/* This FRU for running slc		*/
struct fru_bucket frub202 =
	{"", FRUB1, 0x861, 0x201, NEW_OPTICS_MSG_201,
		{
			{60, "", "",  0, 
				CHILD_NAME, NONEXEMPT},
			{40, "", "",  0, 
				DA_NAME, NONEXEMPT},
		},
	};

/* This FRU for running op		*/
struct fru_bucket frub301 =
  	{"", FRUB1, 0x861, 0x301, NEW_OPTICS_MSG_301,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", 0, PARENT_NAME, NONEXEMPT}, 
		},
	};

/* This FRU for running slc			*/
struct fru_bucket frub302 =
  	{"", FRUB1, 0x861, 0x301, NEW_OPTICS_MSG_301,
		{
			{80, "", "", 0, CHILD_NAME, NONEXEMPT},
			{20, "", "", 0, DA_NAME, NONEXEMPT}, 
		},
	};

struct fru_bucket frub401 =
  	{"", FRUB1, 0x861, 0x401, NEW_OPTICS_MSG_401,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	};

/* configuration fails for op	*/
struct fru_bucket frub501 =
  	{"", FRUB1, 0x861, 0x501, OPTICS_MSG_501,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{10, " ", "", OPTICS_MSG_502 , 
					NOT_IN_DB, EXEMPT},
			{10, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	};

/* configuration fails for slc 	*/
struct fru_bucket frub601 =
	/* using the same OPTICS_MSG_501 configuration fails */
  	{"", FRUB1, 0x861, 0x601, OPTICS_MSG_501,
		{
			{90, "", "", 0, CHILD_NAME, NONEXEMPT},
			{10, " ", "", OPTICS_MSG_502, NOT_IN_DB, EXEMPT},
		},
	};

/* open device fails for op 	*/
struct fru_bucket frub701 =
  	{"", FRUB1, 0x861, 0x701, OPTICS_MSG_701,
		{
			{80, "", "", 0, DA_NAME, NONEXEMPT},
			{20, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	};

/* open device fails for slc	*/
struct fru_bucket frub801 =
  	{"", FRUB1, 0x861, 0x801, OPTICS_MSG_701,
		{
			{80, "", "", 0, CHILD_NAME, NONEXEMPT},
			{20, " ", "", OPTICS_MSG_502, NOT_IN_DB, EXEMPT},
		},
	};

/* error log for ERRID_SLA_EXCEPT_ERR */
struct fru_bucket frub901 =
  	{"", FRUB1, 0x861, 0x901, OPTICS_MSG_801,
		{
			{100, "", "", 0, PARENT_NAME, NONEXEMPT},
		},
	};

struct fru_bucket frub911 =
  	{"", FRUB1, 0x861, 0x901, OPTICS_MSG_801,
		{
			{100, "", "", 0, DA_NAME, NONEXEMPT},
		},
	};

/* error log for ERRID_SLA_PARITY_ERR */
struct fru_bucket frub902 =
  	{"", FRUB1, 0x861, 0x902, OPTICS_MSG_801,
		{
			{100, " ", "", OPTICS_MSG_901, NOT_IN_DB, EXEMPT},
		},
	};

/* error log for ERRID_SLA_FRAME_ERR */
struct fru_bucket frub903 =
  	{"", FRUB1, 0x861, 0x903, OPTICS_MSG_801,
		{
			{60, " ", "", OPTICS_MSG_902, NOT_IN_DB, EXEMPT},
			{40, " ", "", NEW_OPTICS_MSG_903, NOT_IN_DB, EXEMPT},
		},
	};

/* error log for ERRID_SLA_CRC_ERR */
struct fru_bucket frub904 =
  	{"", FRUB1, 0x861, 0x904, OPTICS_MSG_801,
		{
			{60, " ", "", OPTICS_MSG_902, NOT_IN_DB, EXEMPT},
			{40, " ", "", NEW_OPTICS_MSG_903, NOT_IN_DB, EXEMPT},
		},
	};

/* error log for ERRID_SLA_SIG_ERR */
struct fru_bucket frub905 =
  	{"", FRUB1, 0x861, 0x905, OPTICS_MSG_801,
		{
			{60, " ", "", OPTICS_MSG_902, NOT_IN_DB, EXEMPT},
			{40, " ", "", NEW_OPTICS_MSG_903, NOT_IN_DB, EXEMPT},
		},
	};

/* error log for ERRID_SLA_DRIVER_ERR */
struct fru_bucket frub906 =
  	{"", FRUB1, 0x861, 0x906, OPTICS_MSG_801,
		{
			{100, " ", "", OPTICS_MSG_902, NOT_IN_DB, EXEMPT},
		},
	};

/* Fiber optics cable tested fails	*/
struct fru_bucket frub907 =
  	{"", FRUB1, 0x861, 0x907, NEW_OPTICS_MSG_903,
		{
			{100, " ", "", NEW_OPTICS_MSG_903, NOT_IN_DB, EXEMPT},
		},
	};

/*--------------------------------------------------------------*/
/*	Messages and menus					*/
/*--------------------------------------------------------------*/

ASL_SCR_TYPE	menutypes=DM_TYPE_DEFAULTS;
char msgstr[512];
char msgstr1[512];


struct msglist wrap_yes_no[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_YES_OPTION		},
		{ 1, OPTICS_NO_OPTION		},
		{ 1, OPTICS_WRAP_PLUG_TITLE     },
		0    
	};

ASL_SCR_INFO asi_wrap_yes_no[DIAG_NUM_ENTRIES(wrap_yes_no)];

struct msglist wrap_msg[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_PLUG_WRAP           },
		{ 1, OPTICS_FINISHED    	},
		0   
	};

ASL_SCR_INFO asi_wrap_msg[DIAG_NUM_ENTRIES(wrap_msg)];

/* struct for asking the user to unplug the wrap and plug the cables in */

struct msglist unplug_msg[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_UNPLUG_WRAP         },
		{ 1, OPTICS_FINISHED    	},
		0    
	};

ASL_SCR_INFO asi_unplug_msg[DIAG_NUM_ENTRIES(unplug_msg)];

/* struct warning the user about the data lost  	*/

struct msglist warning_yes_no[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_YES_OPTION		},
		{ 1, OPTICS_NO_OPTION		},
		{ 1, OPTICS_WARNING_MSG         },
		0    
	};

ASL_SCR_INFO asi_warning_yes_no[DIAG_NUM_ENTRIES(warning_yes_no)];

/* struct asking the user's confirmation		*/

struct msglist confirm_yes_no[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_YES_OPTION		},
		{ 1, OPTICS_NO_OPTION		},
		{ 1, OPTICS_CONFIRM_MSG         },
		0    
	};

ASL_SCR_INFO asi_confirm_yes_no[DIAG_NUM_ENTRIES(confirm_yes_no)];

/* this message will ask the user if he wants to test cable	*/

struct msglist testing_cable_yes_no[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, OPTICS_YES_OPTION		},
		{ 1, OPTICS_NO_OPTION		},
		{ 1, TESTING_CABLE              },
		0    
	};
ASL_SCR_INFO asi_testing_cable_yes_no[DIAG_NUM_ENTRIES(testing_cable_yes_no)];

/* this message will ask the user to unplug wrap plug and put	*/
/* the cable wrap tool						*/

struct msglist cable_wrap[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, PLUG_CABLE},
		{ 1, OPTICS_FINISHED    	},
		0   
	};

ASL_SCR_INFO asi_cable_wrap[DIAG_NUM_ENTRIES(cable_wrap)];

/* this message will ask the user to unplug the tool and put the network */
/* cable back								*/
struct msglist unplug_tool[]=
	{
		{ 1, NEW_ADVANCED_NO_STANDBY	},
		{ 1, UNPLUG_TOOL		},
		{ 1, OPTICS_FINISHED    	},
		0    
	};

ASL_SCR_INFO asi_unplug_tool[DIAG_NUM_ENTRIES(unplug_tool)];

/*......................................................................*/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the  Optics 2 port Adapter 
 *           and Serial Link Chip Diagnostic Application. 
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
 	struct  sigvec  invec;          /* interrupt handler structure  */	
	int	rc=0;			/* return code 			*/
	struct	stat stat_buf;		/* stat buffer defined in stat.h */
	int	diag_source;
	char	mountcdrfs[128];


	/* initializing interrupt handler */    
	(void) signal (SIGINT, intr_handler);
	setlocale (LC_ALL, "");

	/*..............................*/
	/* General Initialization	*/
	/*..............................*/

	DA_SETRC_STATUS (DA_STATUS_GOOD);
	DA_SETRC_ERROR (DA_ERROR_NONE);
	DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);

	/* initialize odm	*/
	init_dgodm();

	/* get da input 	*/
	getdainput (&da_input);


	if 	/* running under console */
	   ( da_input.console == CONSOLE_TRUE)
	{
		/* initialize asl 		*/
		diag_asl_init ( ASL_INIT_DEFAULT); 
		catd = diag_catopen (CATALOG_NAME,0); 
	}


	/* check if running slc (serial link chip ) diagnostics */
	slc_device = slc_exist ();

	/* getting the define and undefine methods from database	*/

	rc = odm_get_obj (PdDv_CLASS, "type=slc" , &pddv, TRUE);
	if (rc != -1)
	{
		strcpy (define_method,pddv.Define);
		strcpy (undefine_method,pddv.Undefine);
	}
	else
	{
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		clean_up();
	}

	/* if running in LOOPMODE_EXITLM then 		*/
	/* check if the cable is hooked before running  */
	/* DA. If yes, then ask the user to put the   	*/
	/* cable back into the port.			*/

	if ( da_input.loopmode == LOOPMODE_EXITLM)
	{

		/* do not need to check network_cable return code 	*/
		/* because if the wrap plug is already plugged 		*/
		/* before running the test then we do not need to 	*/
		/* ask the user to put it back				*/
		/* in that case the getdavar will returns error 	*/

		wrap_plug= FALSE;
	   	getdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
					&wrap_plug);

		/* if the cable is hooked up before running the DA */
		if ((wrap_plug ) && ( da_input.console == CONSOLE_TRUE))
		{
			rc = diag_display(OPTICS_MENU_NUMBER_8, catd, 
				unplug_msg, DIAG_MSGONLY, 
				ASL_DIAG_KEYS_ENTER_SC,
				 &menutypes, asi_unplug_msg);

               		sprintf(msgstr, asi_unplug_msg[0].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_msg[0].text);
               		asi_unplug_msg[0].text = msgstr;
               		sprintf(msgstr1, asi_unplug_msg[1].text, 
					da_input.dname, da_input.dnameloc);
			free (asi_unplug_msg[1].text);
               		asi_unplug_msg[1].text = msgstr1;

               		rc = diag_display(OPTICS_MENU_NUMBER_8, catd, 
				NULL, DIAG_IO,
                       	ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_unplug_msg);
			network_cable = FALSE;
			putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
			&network_cable );

		}	

		network_cable = FALSE;
	   	getdavar(da_input.dname, "network_cable", DIAG_SHORT, 
					&network_cable);

		/* if the cable is hooked up before running the DA */
		if ((network_cable) && ( da_input.console == CONSOLE_TRUE))
		{
			/* asking the user to unplug tool    g 	*/
			/* and put the nework cable back	*/

			rc = diag_display(OPTICS_MENU_NUMBER_13, catd, 
				unplug_tool, DIAG_MSGONLY, 
				ASL_DIAG_KEYS_ENTER_SC,
               			&menutypes, asi_unplug_tool);
               		sprintf(msgstr, asi_unplug_tool[0].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_tool[0].text);
               		asi_unplug_tool[0].text = msgstr;
               		sprintf(msgstr1, asi_unplug_tool[1].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_tool[1].text);
               		asi_unplug_tool[1].text = msgstr1;

               		rc = diag_display(OPTICS_MENU_NUMBER_13, catd, NULL, 
				DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, 
				&menutypes,asi_unplug_tool);

			/* setting networkcable variable in data base to true */ 
			network_cable = FALSE;
			putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
				&network_cable );
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
		if  ( da_input.console == CONSOLE_TRUE) 
			optics_da_display ();
	
		/* check if diskette package is running */
		diag_source = diag_exec_source(&mountcdrfs);
  		diskette_mode = ipl_mode(&diskette_based);
               	if ( (diskette_based == DIAG_FALSE) &&
		     (diag_source == 0) )
		{
			rc = stat ("/usr/lib/drivers/soldd", &stat_buf);
			if /* file does not exist */
			   (( rc == ERROR) || ( stat_buf.st_size == 0 ))
			{

				/* just send menugoal one time only */ 

				if  ((da_input.console == CONSOLE_TRUE) &&
				    ((da_input.loopmode == LOOPMODE_ENTERLM)
				   || (da_input.loopmode == LOOPMODE_NOTLM)))
				{
					/* displaying the device driver */
					/* file error 			*/
				    sprintf (msgstr,(char *)diag_cat_gets (catd, 
					      1, OPTICS_DD_FILE_BAD), NULL);
				    menugoal (msgstr);
				}
				DA_SETRC_STATUS (DA_STATUS_BAD);
				clean_up();
			}
		}

		rc =configure_own_device();
	 	if  /*  device cannot be configured	*/
			(rc  == ERROR )
		{
			if (slc_device)
			{
				strcpy (frub601.frus[0].fname, base_parent);
				report_frub(&frub601);
			}
			else
		 		report_frub(&frub501);
			DA_SETRC_STATUS (DA_STATUS_BAD);
		}	
		else       /* open device and running tests */
		{
			rc = open_device ();
			if ( rc == NO_ERROR) 
			{
				rc = performing_test ();
			}
				
		}
	}
	 
	/* if running problem determination, or error log analyis 	*/
	/* running error log analysis					*/

	if  (( rc == NO_ERROR) &&((( da_input.dmode ==  DMODE_PD) 
		|| (da_input.dmode == DMODE_ELA ))
		&& (( da_input.loopmode == LOOPMODE_NOTLM ) 
		||  ( da_input.loopmode == LOOPMODE_ENTERLM ))
		&& (da_input.exenv != EXENV_REGR)))
	{
		error_log_analysis ();
	}

	/* cleaning up and exit DA	*/

	clean_up ();
	
} /* main */

/*......................................................*/
/*      Procedure description : This procedure will 	*/
/*				call test units to test */
/*				the port		*/
/*......................................................*/
int performing_test ()
{
	int	rc =NO_ERROR;
	int	tu_rc;

	/* intialize test unit control block */
	tucb.mfg = 0;
	tucb.loop = 1;

	/* performing non interactive tests this does not include	*/
	/* port wrap test						*/

	rc =  non_interactive_test ();

	/* system true means non-interactive test 	*/
	/* for serial link chip		  		*/
	/* the interactive test is not needed		*/

	if (( rc == NO_ERROR) && ( da_input.advanced == ADVANCED_TRUE) 
			&& ( da_input.console == CONSOLE_TRUE)
			&& ( da_input.system == SYSTEM_FALSE)
			&& (slc_device == FALSE))
	{
		rc = interactive_test ();
		if ((rc == NO_ERROR) && ( da_input.exenv != EXENV_REGR) && 
			(( da_input.loopmode == LOOPMODE_NOTLM ) || 
			 ( da_input.loopmode == LOOPMODE_ENTERLM)))
		{
			rc = display_menu (TESTING_CABLE);
			if ( rc == QUIT)
			{
				return (NO_ERROR);
			}	
			rc = display_menu (PLUG_CABLE);
			display_menu (NEW_TESTING_ADVANCED_MODE_WAITING);
			sleep (60);
			tu_rc = run_tu4 ();
			if (tu_rc != NO_ERROR)
			{
		 		report_frub(&frub907);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				return (ERROR);
			}
		}
		wrap_plug= FALSE;
	   	getdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
					&wrap_plug);
		if ((wrap_plug) && ( da_input.exenv != EXENV_REGR) && 
			( da_input.loopmode == LOOPMODE_NOTLM ))
		{
			/* asking the user to unplug wrap plug 	*/
			/* and put the nework cable back	*/

			rc = diag_display(OPTICS_MENU_NUMBER_8, catd, 
				unplug_msg, DIAG_MSGONLY, 
				ASL_DIAG_KEYS_ENTER_SC,
               			&menutypes, asi_unplug_msg);
               		sprintf(msgstr, asi_unplug_msg[0].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_msg[0].text);
               		asi_unplug_msg[0].text = msgstr;
               		sprintf(msgstr1, asi_unplug_msg[1].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_msg[1].text);
               		asi_unplug_msg[1].text = msgstr1;

               		rc = diag_display(OPTICS_MENU_NUMBER_8, catd, NULL, 
				DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, 
				&menutypes,asi_unplug_msg);
			if ((check_asl_stat (rc)) == QUIT) return (QUIT);

			/* setting networkcable variable in data base to true */ 
			network_cable = FALSE;
			putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
				&network_cable );

		}

		network_cable= FALSE;
	   	getdavar(da_input.dname, "network_cable", DIAG_SHORT, 
					&network_cable);
		if ((network_cable) && ( da_input.exenv != EXENV_REGR) && 
			( da_input.loopmode == LOOPMODE_NOTLM ))
		{
			/* asking the user to unplug tool    g 	*/
			/* and put the nework cable back	*/

			rc = diag_display(OPTICS_MENU_NUMBER_13, catd, 
				unplug_tool, DIAG_MSGONLY, 
				ASL_DIAG_KEYS_ENTER_SC,
               			&menutypes, asi_unplug_tool);
               		sprintf(msgstr, asi_unplug_tool[0].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_tool[0].text);
               		asi_unplug_tool[0].text = msgstr;
               		sprintf(msgstr1, asi_unplug_tool[1].text, 
				da_input.dname, da_input.dnameloc);
			free (asi_unplug_tool[1].text);
               		asi_unplug_tool[1].text = msgstr1;

               		rc = diag_display(OPTICS_MENU_NUMBER_13, catd, NULL, 
				DIAG_IO, ASL_DIAG_KEYS_ENTER_SC, 
				&menutypes,asi_unplug_tool);

			/* setting networkcable variable in data base to true */ 
			network_cable = FALSE;
			putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
				&network_cable );
		}
	}
	return (rc);
}
/*......................................................*/
/*      Procedure interactive_test:  This procedure 	*/
/*				     will call the port */
/*				     wrap test		*/
/*      Requisite                    The wrap plug 	*/
/*				     needs to be plugged*/
/*				     the tested port    */
/*......................................................*/

int interactive_test ()
{
	int	rc=NO_ERROR;
	int	tu_rc=NO_ERROR;
 	ASL_SCR_TYPE	menutypes;



	/* if running regression with dmode = 0  
	   then interactive test is not run  
	   dmode = 1 means running with wrap plug */

	if (( da_input.exenv == EXENV_REGR ) && 
	    ( da_input.dmode != 1)) 
		return ( NO_ERROR);
	/* if in loop mode and the user choose that he does not have a wrap */
	/* then not running advanced test */
	if (da_input.loopmode == LOOPMODE_INLM)
	{
	   	getdavar(da_input.dname, "nowrap", DIAG_SHORT, &nowrap);
		if (nowrap )
			return (NO_ERROR);
	}

	/* running wrap test, tert unit # 4 */
	tu_rc = run_tu4 ();
	if (tu_rc == NO_ERROR)   
		return (NO_ERROR);
	rc = NO_ERROR;
	if ( da_input.console == CONSOLE_TRUE)
	{
		rc = check_asl_stat ( 
			diag_asl_read ( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0));
	}
	if ( rc == QUIT) return (QUIT);

	if  (( da_input.loopmode == LOOPMODE_INLM) ||
		(da_input.exenv == EXENV_REGR ))
	{
		check_rc_4 (tu_rc);
		return (ERROR);
	}

	if ( ( da_input.exenv != EXENV_REGR) && 
	      (( da_input.loopmode  == LOOPMODE_ENTERLM) ||
		( da_input.loopmode == LOOPMODE_NOTLM)) )
	{
		/* asking the user if he has  the wrap plug */

		rc = diag_display(OPTICS_MENU_NUMBER_6, catd, wrap_yes_no,
                    		DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                    		&menutypes, asi_wrap_yes_no);
                sprintf(msgstr, asi_wrap_yes_no[0].text, 
				da_input.dname, da_input.dnameloc);
		free (asi_wrap_yes_no[0].text);
                asi_wrap_yes_no[0].text = msgstr;

		/* set the cursor points to NO */
		menutypes.cur_index = NO;
                rc = diag_display(OPTICS_MENU_NUMBER_6, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,asi_wrap_yes_no);
		if ((check_asl_stat (rc)) == QUIT) return (QUIT);

		rc = DIAG_ITEM_SELECTED (menutypes);
		if      /* if user does not have the wrap NTF */
		   ( rc != YES)  
		{
			/* setting nowrap variable in data base to true */ 
			nowrap = TRUE;
			putdavar(da_input.dname, "nowrap", DIAG_SHORT, &nowrap);
			return (QUIT);
		}

		/* ask the user unplug the network cable if attached 	*/ 
		/* and plug the wrap plug into the tested port		*/
		nowrap = FALSE;
		putdavar(da_input.dname, "nowrap", DIAG_SHORT, &nowrap);

		rc = diag_display(OPTICS_MENU_NUMBER_7, catd, wrap_msg,
                    		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                    		&menutypes, asi_wrap_msg);
                sprintf(msgstr, asi_wrap_msg[0].text, 
				da_input.dname, da_input.dnameloc);
		free (asi_wrap_msg[0].text);
                asi_wrap_msg[0].text = msgstr;
                sprintf(msgstr1, asi_wrap_msg[1].text, da_input.dname,
					da_input.dnameloc);
		free (asi_wrap_msg[1].text);
                asi_wrap_msg[1].text = msgstr1;

                rc = diag_display(OPTICS_MENU_NUMBER_7, catd, NULL, DIAG_IO,
                      ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_wrap_msg);

		if ((check_asl_stat (rc)) == QUIT) return (QUIT);

		/* setting wrap_plug variable in data base to true */ 
		wrap_plug= TRUE;
		putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
			&wrap_plug);


	}
	/* wait for the laser to turn on */
	display_menu (NEW_TESTING_ADVANCED_MODE_WAITING);
	sleep (60);
	tu_rc = run_tu4 ();


	if  /* error then report fru and exit */
	  ( tu_rc != NO_ERROR)
	{
		check_rc_4 (tu_rc);
	}

	return (tu_rc);
}

/*......................................................*/
/*            	Perform non interactive test         	*/
/*		Running port wrap test			*/
/*......................................................*/

int non_interactive_test ()
{
	int	rc= NO_ERROR;
	int	asl_rc;

	/* run test unit 1 	*/
	rc = run_tu1 ();

	if ( rc != NO_ERROR)
	{
		check_rc_1_2 (rc);
	}
	else  /* running test 2 and 3 */
	{
		asl_rc = NO_ERROR;
		if ( da_input.console == CONSOLE_TRUE)
		{
			asl_rc = check_asl_stat ( diag_asl_read 
					( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0));
		}
		if ( asl_rc == QUIT) return (QUIT);

		rc = run_tu2();

		if ( rc != NO_ERROR)
		{
			check_rc_1_2 (rc);
		}
		else /* running test 3 */
		{
			asl_rc = NO_ERROR;
			if ( da_input.console == CONSOLE_TRUE)
			{
				asl_rc = check_asl_stat ( diag_asl_read 
					( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0));
			}
			if ( asl_rc == QUIT) return (QUIT);

			rc = run_tu3();
			if ( rc != NO_ERROR)
				check_rc_3(rc);	
			asl_rc = NO_ERROR;
			if ( da_input.console == CONSOLE_TRUE)
			{

				asl_rc = check_asl_stat ( diag_asl_read 
					( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0));
			}
			if ( asl_rc == QUIT) return (QUIT);
		}

	}
	return (rc);
}
/*......................................................*/
/*     Procedure description : display optics 2 port 	*/
/*			       or serial link chip	*/
/*			       testing on screen 	*/
/*......................................................*/

void optics_da_display ()
{

	if ( da_input.loopmode == LOOPMODE_INLM) 

		display_menu ( NEW_TESTING_LOOPMODE);
	
	else if ( da_input.advanced == ADVANCED_TRUE)
		display_menu ( NEW_TESTING_ADVANCED_MODE);

	else
		display_menu (NEW_TESTING_REGULAR) ;

}

/*......................................................*/
/*    Procedure description :   opening the device	*/
/*                              if device is busy a 	*/
/*				message is displayed	*/
/*                              to ask the input of 	*/
/*				the user	   	*/
/*......................................................*/

int open_device ()
{
	int	rc =NO_ERROR;	
	char	dname[]="/dev/";
	char	open_flag[] = "/D";
	char	forced_flag[] = "/F";
	char	device_open[]= "op0";	
	int	open_error;
	

	devname = (char *) malloc (NAMESIZE +( strlen (dname) +1));
	/* cannot allocate the memory */
	if (devname == NULL)
	{ 
		DA_SETRC_ERROR (DA_ERROR_OTHER);
		return (DA_SOFTWARE_ERROR);
	}

	strcpy (devname, dname);
	strcat (devname, base_port);
	strcat (devname, open_flag);

	/* open device			*/
	filedes = open(devname, O_RDWR);
	open_error = errno;

	if  (( filedes == ERROR) && ( open_error != EBUSY ))
	{
		switch (open_error)
		{
			case EIO:
			case ENODEV:
				if (slc_device)
				{
					strcpy (frub801.frus[0].fname, 
							base_parent);
					report_frub (&frub801);
				}
				else 
					report_frub (&frub701);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				break;
			default:
				DA_SETRC_ERROR (DA_ERROR_OPEN);                 
				break;

		}
		return (ERROR);

	}
	
	if ((open_error  == EBUSY ) && (da_input.console == CONSOLE_TRUE)
				&& (slc_device == FALSE))
	{
		/* displaying menu asking the user if he wants the 	*/
		/* open is forced by DA					*/

		rc = diag_display(OPTICS_MENU_NUMBER_4, catd, warning_yes_no,
                    		DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                    		&menutypes, asi_warning_yes_no);

		sprintf(msgstr, asi_warning_yes_no[0].text,
				da_input.dname, da_input.dnameloc);
		free (asi_warning_yes_no[0].text);
		asi_warning_yes_no[0].text = msgstr;
                sprintf(msgstr1, asi_warning_yes_no[3].text, 
				da_input.dname, da_input.dnameloc);
		free (asi_warning_yes_no[3].text);
                asi_warning_yes_no[3].text = msgstr1;


		/* set the cursor to NO */
		menutypes.cur_index = NO;
                rc = diag_display(OPTICS_MENU_NUMBER_4, catd, NULL, DIAG_IO,
                   ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,asi_warning_yes_no);
		rc = DIAG_ITEM_SELECTED (menutypes);

		if      /* if user choose let DA force to open */
		   ( rc == YES )  
		{
			/* display the confirmation of the user */

			rc = diag_display(OPTICS_MENU_NUMBER_5, catd, 
				confirm_yes_no,
                    		DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                    		&menutypes, asi_confirm_yes_no);


                	sprintf(msgstr,asi_confirm_yes_no[0].text,
					da_input.dname, da_input.dnameloc);
			free (asi_confirm_yes_no[0].text);
                	asi_confirm_yes_no[0].text = msgstr;

			/* set the cursor points to NO */
			menutypes.cur_index = NO;
                	rc = diag_display(OPTICS_MENU_NUMBER_5, catd, NULL, 
                   		DIAG_IO, ASL_DIAG_LIST_CANCEL_EXIT_SC, 
				&menutypes,asi_confirm_yes_no);
			if ((check_asl_stat (rc)) == QUIT) return (QUIT);

			rc = DIAG_ITEM_SELECTED (menutypes);
			if      /* if user wants to continue,  force open */
		   	  ( rc == YES)  
			{
				/* open the device with special flag */
				strcpy (devname, dname);
				strcat (devname, base_port);
				strcat (devname, forced_flag);
				filedes = open(devname, O_RDWR);
				open_error = errno;

			}
		}

	}
	if (filedes == ERROR  )
	{
		if ( open_error == EBUSY)
		{
			DA_SETRC_ERROR (DA_ERROR_OPEN);                 
		}
		else
			DA_SETRC_ERROR (DA_ERROR_OTHER);                 
		return (open_error);
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
	char	define_args[512];
	int	odm_rc;
	char	*cfg_outbuf, *eptr;
	char	odm_search_crit[40];
	struct	CuDv		*cudv;
	int	counter;
	struct	listinfo	obj_info;

	if (devname != NULL) 
		free (devname);

	if (filedes != ERROR)
		close (filedes);


	/*  restoring the device to the initial state */
	if (define_child)
	{
		initial_state (child_state, base_port);
		sprintf (define_args, " -l %s ", base_port);
                       		odm_rc = odm_run_method (undefine_method,
               			define_args, &cfg_outbuf, &eptr);
	}
	else if (child_state != NOT_CONFIG)
	{
		initial_state (child_state, base_port);
		if ( (found_child) && ( !otp_card_exist))
		{
			odm_change_obj(CuDv_CLASS,&child_cudv);
		}
	}

        if (!second_port_exist)
        {
	 	strcpy ( second_port, "op0");
                for ( counter =0; counter < 8; counter++)
                {
			rc = strcmp (second_port, base_port);
		 	if (rc != 0)
			{
                        	sprintf(odm_search_crit, 
					"name = %s",second_port);
                        	cudv = get_CuDv_list( CuDv_CLASS,
                                        odm_search_crit, &obj_info, 1, 2 );
				if(cudv != (struct CuDv *)-1)
                        		if(!strcmp ( parent, cudv->parent))
                        		{
                                		define_second_port = TRUE;
						odm_free_list(cudv, &obj_info);
                                		break;
                       		 	}
			}
                        strcpy ( second_port, test_port[counter]);
                }
                if (define_second_port)
                {
			initial_state (child_state, second_port);
                        sprintf (define_args, " -l %s ", second_port);
                        odm_rc = odm_run_method (undefine_method,
                                       define_args, &cfg_outbuf, &eptr);
                }
       	}
	else	
	{
		/* the second port exists before running DA	*/
		/* so we need to change the second port to 	*/
		/* the states of odm before we running	DA	*/

		if (!otp_card_exist)
			odm_change_obj(CuDv_CLASS,&second_child_cudv);
	}


	if (define_parent)
	{
		initial_state (parent_state, parent);
		sprintf (define_args, " -l %s ", parent);
             	odm_rc = odm_run_method (undefine_method,
                           define_args, &cfg_outbuf, &eptr);
	}
	else if (parent_state != NOT_CONFIG)
	{
		initial_state (parent_state, parent);
		if ((found_parent) && ( !otp_card_exist))
		{
			odm_change_obj(CuDv_CLASS,&parent_cudv);
		}
	}
	if (grand_parent_state != NOT_CONFIG)
	{
		initial_state (grand_parent_state, grand_parent);
	}

	/* terminate odm */
	term_dgodm();
	if ( da_input.console == TRUE )
	{
		/* closing optics 2 port catalog file , asl */
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

        /*..........................................................*/
        /* Look for an occurrence of any of the following errors in */
        /* the error log.  If one exists, exit_with_fru for ELA.    */
        /*..........................................................*/

        sprintf(crit,"%s -N %s -j %X,%X,%X,%X,%X,%X",
                  da_input.date,
                  da_input.dname,
                  ERRID_SLA_EXCEPT_ERR,
                  ERRID_SLA_PARITY_ERR,
                  ERRID_SLA_FRAME_ERR,
                  ERRID_SLA_CRC_ERR,
                  ERRID_SLA_SIG_ERR,
                  ERRID_SLA_DRIVER_ERR );

        ela_get_rc = error_log_get(INIT,crit,&errdata_str);
	if (ela_get_rc >0)
	{
		DA_SETRC_STATUS (DA_STATUS_BAD);
		switch (errdata_str.err_id)
		{
			case ERRID_SLA_EXCEPT_ERR:
				if (slc_device)
				{
					report_frub (&frub911);
				}
				else
					report_frub (&frub901);
				break;
			case ERRID_SLA_PARITY_ERR:
				report_frub (&frub902);
				break;
			case ERRID_SLA_FRAME_ERR:
				report_frub (&frub903);
				break;
			case ERRID_SLA_CRC_ERR:
				report_frub (&frub904);
				break;
			case ERRID_SLA_SIG_ERR:
				report_frub (&frub905);
				break;
			case ERRID_SLA_DRIVER_ERR:
				report_frub (&frub906);
				break;
			default:
				DA_SETRC_ERROR (DA_ERROR_OTHER);
				break;
		}
	}
        ela_rc = error_log_get(TERMI,crit,&errdata_str);
}


/*......................................................*/
/*     Procedure :      check return code according the */
/*			return code from test units     */
/*......................................................*/

check_rc_1_2 (rc)
int	rc;
{
	switch (rc)
	{
		case SOL_NO_OPTICAL_CARD_PLUGGED:
		{
			break;
		}
		case SOL_BUF_ACCESS_WRITE_FAILED:
		case SOL_BUF_ACCESS_READ_FAILED:
		case SOL_BUF_ACCESS_CMP_FAILED:
		{
			if (slc_device)
			{
				if (otp_card_exist)
				{
					strcpy (frub102.frus[0].fname, 
						base_parent);
					report_frub (&frub102);
				}
				else
					report_frub (&frub103);
			}
			else
				report_frub (&frub101);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			break;
		}
		case SOL_ACTIVATE_MODE_FAILED:
		case SOL_SCR_MODE_FAILED:
		case SOL_OLS_MODE_FAILED:
		case SOL_RHR_TEST_FAILED:
		case SOL_CRC_TEST_FAILED:
		{
			if (slc_device)
			{
				strcpy (frub202.frus[0].fname, base_parent);
				report_frub (&frub202);
			}
			else
				report_frub (&frub201);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			break;
		}
		default:
		{
			/* unidentified error from test unit */
			DA_SETRC_ERROR (DA_ERROR_OTHER);                 
			break;
		}
	}
	return (rc);

} /* check_rc_1_2 */

check_rc_3 (rc)
int	rc;
{
	switch (rc)
	{
		case SOL_LOCK_TO_XTAL_FAILED:
		case SOL_ACTIVATE_MODE_FAILED:
		case SOL_SCR_MODE_FAILED:
		case SOL_OLS_MODE_FAILED:
		case SOL_RHR_TEST_FAILED:
		case SOL_CRC_TEST_FAILED:
		{
			DA_SETRC_STATUS (DA_STATUS_BAD);
			if (slc_device)
			{
				strcpy (frub302.frus[0].fname, base_parent);
				report_frub (&frub302);
			}
			else
				report_frub (&frub301);
			break;
		}
		default:
		{
			/* unidentified error from test unit */
			DA_SETRC_ERROR (DA_ERROR_OTHER);                 
			break;
		}
	}

} /* check_rc_3 */

check_rc_4 (rc)
int	rc;
{
	switch (rc)
	{
		case SOL_LOCK_TO_XTAL_FAILED:
		case SOL_ACTIVATE_MODE_FAILED:
		case SOL_SCR_MODE_FAILED:
		case SOL_OLS_MODE_FAILED:
		case SOL_RHR_TEST_FAILED:
		case SOL_CRC_TEST_FAILED:
		case SOL_SYNC_OTP_FAILED:
		{
			report_frub (&frub401);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			break;
		}
		default:
		{
			/* unidentified error from test unit */
			DA_SETRC_ERROR (DA_ERROR_OTHER);                 
			break;
		}
	}

} /* check_rc_4 */


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

/*......................................................................*/
/*	Function description :	This function will set the tucb  	*/
/*				to 1 and loop to 1 so the test unit 1 	*/
/*				will be executed.			*/
/*......................................................................*/

int	run_tu1 ()
{
	int	rc=0;

	tucb.tu =1;
	rc = exectu (filedes, &tucb);

	return (rc);
}

/*......................................................................*/
/*	Function description :	This function will set the tucb  	*/
/*				to 2 and loop to 1 so the test unit 2 	*/
/*				will be executed.			*/
/*......................................................................*/

int	run_tu2 ()
{
	int	rc=0;

	tucb.tu =2;
	rc = exectu (filedes, &tucb);

	return (rc);
}

/*......................................................................*/
/*	Function description :	This function will set the tucb  	*/
/*				to 3 and loop to 1 so the test unit 3 	*/
/*				will be executed.			*/
/*......................................................................*/

int	run_tu3 ()
{
	int	rc=0;

	tucb.tu =3;
	rc = exectu (filedes, &tucb);
	return (rc);
}

/*......................................................................*/
/*	Function description :	This function will set the tucb  	*/
/*				to 4 and loop to 1 so the test unit 4 	*/
/*				will be executed.			*/
/*......................................................................*/

int	run_tu4 ()
{
	int	rc=0;

	tucb.tu =4;
	rc = exectu (filedes, &tucb);
	return (rc);
}

/*......................................................................*/
/*	Function description :	This function will display three 	*/
/*				different message. Advance, loopmode 	*/
/*				and regular testing menu		*/
/*				These menus display is divided		*/
/*				into display testing 			*/
/*				- Serial Link Chip			*/
/*				- Serial Optical Channel Converter	*/
/*......................................................................*/

int display_menu (menu_type)
int	menu_type;
{
	int	rc;

	switch (menu_type)
	{
		case NEW_TESTING_LOOPMODE:
			if ( slc_device)
			 	rc = diag_msg_nw ( OPTICS_MENU_NUMBER_3, catd, 
			   	1, NEW_TESTING_LOOPMODE_SLC,da_input.dname, 
				da_input.dnameloc, da_input.lcount,
				da_input.lerrors);

			else
			 	rc = diag_msg_nw ( OPTICS_MENU_NUMBER_3, catd, 
			   	1, NEW_TESTING_LOOPMODE,da_input.dname, 
				da_input.dnameloc, da_input.lcount,
				da_input.lerrors);
			break;
		case NEW_TESTING_ADVANCED_MODE:
			if (slc_device)
				rc = diag_msg_nw ( OPTICS_MENU_NUMBER_2, catd, 
			   		1, NEW_TESTING_ADVANCED_MODE_SLC,
					da_input.dname, da_input.dnameloc); 
			else
				rc = diag_msg_nw ( OPTICS_MENU_NUMBER_2, catd, 
			   		1, NEW_TESTING_ADVANCED_MODE,da_input.dname,
				da_input.dnameloc); 

			break;
		case NEW_TESTING_REGULAR:
			if (slc_device)
				rc = diag_msg_nw ( OPTICS_MENU_NUMBER_1, catd, 
			   		1, NEW_TESTING_REGULAR_SLC,da_input.dname,
				da_input.dnameloc);
			else
				rc = diag_msg_nw ( OPTICS_MENU_NUMBER_1, catd, 
			   		1, NEW_TESTING_REGULAR,da_input.dname,
				da_input.dnameloc);

			break;
		/* this advanced mode waiting just for serial optical	*/
		/* channel converter diagnostics     			*/

		case NEW_TESTING_ADVANCED_MODE_WAITING:
				rc = diag_msg_nw ( OPTICS_MENU_NUMBER_10, catd, 
			   		1, NEW_TESTING_ADVANCED_MODE_WAITING,
					da_input.dname, da_input.dnameloc); 
			break;

		
		case TESTING_CABLE:
		/* asking the user if he wants to test cable 	*/ 

		rc = diag_display(OPTICS_MENU_NUMBER_11, catd, 
			testing_cable_yes_no, DIAG_MSGONLY, 
			ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, 
			asi_testing_cable_yes_no);
                sprintf(msgstr, asi_testing_cable_yes_no[0].text, 
				da_input.dname, da_input.dnameloc);
		free (asi_testing_cable_yes_no[0].text);
                asi_testing_cable_yes_no[0].text = msgstr;

		/* set the cursor points to NO */
		menutypes.cur_index = NO;
                rc = diag_display(OPTICS_MENU_NUMBER_11, catd, NULL, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
			asi_testing_cable_yes_no);

		if ((check_asl_stat (rc)) == QUIT) return (QUIT);

		rc = DIAG_ITEM_SELECTED (menutypes);
		if      /* if user does not have the wrap NTF */
		   ( rc != YES)  
		{
			network_cable = FALSE;
			putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
			&network_cable );
			return (QUIT);
		}
		/* setting wrap_plug variable in data base to true */ 
		wrap_plug = FALSE;
		putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, 
			&wrap_plug);

		/* setting networkcable variable in data base to true */ 
		network_cable = TRUE;
		putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
			&network_cable );

		return (0);

		break;

		case PLUG_CABLE:

		/* ask the user unplug the wrap plug if attached 	*/ 
		/* and plug  wrap calbe into the tested port		*/

		rc = diag_display(OPTICS_MENU_NUMBER_12, catd, cable_wrap,
                    		DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                    		&menutypes, asi_cable_wrap);
                sprintf(msgstr, asi_cable_wrap[0].text, 
				da_input.dname, da_input.dnameloc);
		free (asi_cable_wrap[0].text);
                asi_cable_wrap[0].text = msgstr;
                sprintf(msgstr1, asi_cable_wrap[1].text, da_input.dname,
					da_input.dnameloc);
		free (asi_cable_wrap[1].text);
                asi_cable_wrap[1].text = msgstr1;

                rc = diag_display(OPTICS_MENU_NUMBER_12, catd, NULL, DIAG_IO,
                      ASL_DIAG_KEYS_ENTER_SC, &menutypes,asi_cable_wrap);

		if ((check_asl_stat (rc)) == QUIT) return (QUIT);



		/* setting networkcable variable in data base to true */ 
		network_cable = TRUE;
		putdavar(da_input.dname, "network_cable", DIAG_SHORT, 
			&network_cable );
		return (0);

	}
	/* check display return code 	*/
	check_asl_stat (rc);
}

/*......................................................................*/
/*	Function description :	This function will add the fru bucket 	*/
/*				into the data base			*/
/*......................................................................*/

int report_frub (frub_addr)
struct fru_bucket *frub_addr;
{
	/* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);
	/* update FRU Bucket with parent device name 		*/
       	insert_frub(&da_input, frub_addr);

	/* add a FRU bucket; 					*/
	addfrub(frub_addr);

}
/*......................................................................*/
/*	Function description :	This function will check if the slc   	*/
/*				is the dname then we should open        */
/*				op0 or op1 or op2 or op3 for 		*/
/* 				if one of these port exists. 		*/
/*				Because we want to use to test slc     	*/
/*				and op the same da			*/
/*	Return code:		TRUE : if dname is slc			*/
/*				FALSE: else.				*/
/*......................................................................*/

int	slc_exist ()
{
	
	char	slc[4] ;
	
	strcpy ( slc, da_input.dname);
	if 
	   ((strspn ( &slc[0], "slc")) == 0)
	{
		return (FALSE);
	}
	return (TRUE);

}
/*......................................................................*/
/*	Function:	configure_own_device				*/
/*			this function will get the parent of opx 	*/
/*			and then parent of opx ( otpx) and 		*/
/*			parent of otpx (slcx).				*/
/*			Then configure these devices			*/
/*	Return code	: NO_ERROR	successfully configured 	*/
/*			  ERROR		one of the device cannot be	*/
/*					configured			*/
/*......................................................................*/

int	configure_own_device ()
{


	char	odm_search_crit[40];
	struct	CuDv		*cudv;
	struct	listinfo	obj_info;
	int	counter;
	int	rc = ERROR;
	int	sub_rc;
	char	define_args[512];
	struct	PdDv	*pddv;
	int	odm_rc;
	char	*cfg_outbuf, *eptr;

	if  	/* serial link chip is tested */
	   (slc_device)
	{
		strcpy ( grand_parent, da_input.dname);
		strcpy ( base_parent, "otp0");

		/* find out which otpX whose parent is dname		*/
		for ( counter =0; counter < MAX_NUM_PORTS; counter++)
		{
			sprintf (odm_search_crit, "name = %s",base_parent); 
			cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, 
					&obj_info, 1, 2 );
			if(cudv != (struct CuDv *)-1)
				if(!strcmp ( da_input.dname, cudv->parent))
				{
					strcpy (parent, base_parent);
					found_parent = TRUE;
					odm_free_list(cudv, &obj_info);
					break;
				}
			strcpy ( base_parent, test_parent[counter]);
		}

		/* if the  parent (otpX) is not in CuDv	*/
		/* define the device 					*/
		if (!found_parent)
		{
			sprintf
			  (define_args, "-c adapter -s slc -t otp -p %s -w 1",
			   da_input.dname);
			odm_rc = odm_run_method (define_method, 
					define_args, &cfg_outbuf, &eptr);
			if (odm_rc != NO_ERROR)
			{
				DA_SETRC_ERROR (DA_ERROR_OTHER);
				return (DA_SOFTWARE_ERROR);
			}
			/* the global flag indicating that the device	*/
			/* was defined by diagnostics			*/
			define_parent = TRUE;
			strcpy ( parent, cfg_outbuf);
			parent[4]='\0';
		}

		strcpy ( base_port, "op0");
		/* find out which opX whose parent is parent  */
		for ( counter =0; counter < 8; counter++)
		{
			sprintf (odm_search_crit, "name = %s",base_port); 
			cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, 
					&obj_info, 1, 2 );
			if(cudv != (struct CuDv *)-1)
				if(!strcmp ( parent, cudv->parent))
				{
					found_child = TRUE;
					odm_free_list(cudv, &obj_info);
					break;
				}
			strcpy ( base_port, test_port[counter]);
		}

		/* if the child (opX) is not in CuDv	*/
		/* define the port   					*/
		if (!found_child)
		{
			sprintf
			  (define_args, "-c adapter -s otp -t op -p %s -w 1",
			   parent);
			odm_rc = odm_run_method (define_method, 
					define_args, &cfg_outbuf, &eptr);
			if (odm_rc != NO_ERROR)
			{
				DA_SETRC_ERROR (DA_ERROR_OTHER);
				return (DA_SOFTWARE_ERROR);
			}
			/* the global flag indicating that the device	*/
			/* was defined by diagnostics			*/
			/* so that the port will be undefine in cleanup	*/
			define_child = TRUE;
			strcpy ( base_port, cfg_outbuf);
			base_port[3]='\0';
		}

	}
	else	/* find the parent of dname, grandparent of dname */
	{
		strcpy ( base_port, da_input.dname);
		sprintf (odm_search_crit, "name = %s",base_port); 
		cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, 
				&obj_info, 1, 2 );
		strcpy ( parent, cudv->parent);
		sprintf (odm_search_crit, "name = %s",parent); 
		cudv = get_CuDv_list( CuDv_CLASS, odm_search_crit, 
				&obj_info, 1, 2 );
		strcpy ( grand_parent, cudv->parent);
		odm_free_list(cudv, &obj_info);
	}

	/* to find out if the second port exists before running DA	*/
	/* so that we need to restore it in its initial state in clean up */
	/* if the second port does not exist when configuring the 	*/
	/* parent. This second port is in define state and this 	*/
	/* second port needs to be removed in cleanup			*/
	strcpy ( second_port, "op0");
        for ( counter =0; counter < 8; counter++)
        {
		rc = strcmp (second_port, base_port);
		if (rc != 0)
		{
        		sprintf(odm_search_crit, "name = %s",second_port);
               		cudv = get_CuDv_list( CuDv_CLASS,
                                   odm_search_crit, &obj_info, 1, 2 );
			if(cudv != (struct CuDv *)-1)
               			if(!strcmp ( parent, cudv->parent))
               			{
               				second_port_exist = TRUE;
					odm_free_list(cudv, &obj_info);
                       			break;
                		}
		}
                strcpy ( second_port, test_port[counter]);
         }

	/* this needs to be save so it will be restore in clean up */

	sprintf (odm_search_crit, "name = %s", base_port);
	rc = odm_get_obj (CuDv_CLASS, odm_search_crit, &child_cudv, 
			  TRUE);
	sprintf (odm_search_crit, "name = %s", second_port);
	rc = odm_get_obj (CuDv_CLASS, odm_search_crit, 
	                  &second_child_cudv, TRUE);
	
	sprintf (odm_search_crit, "name = %s", parent);
	rc = odm_get_obj (CuDv_CLASS, odm_search_crit, 
	                  &parent_cudv, TRUE);
	
	if (parent_cudv.status == AVAILABLE)
	{
		otp_card_exist = TRUE;
	}
	
	/* save the state of devices so when clean  up		*/
	/* these states should be restored			*/


	if ((grand_parent_state = configure_device (grand_parent )) == ERROR )
	{
		return (ERROR);
	}
	if ((parent_state = configure_device (parent )) == ERROR )
	{
		return (ERROR);
	} if ((child_state = configure_device (base_port )) == ERROR )
	{
		return (ERROR);
	}

	return (NO_ERROR);
}

/*......................................................................*/
/*	Function:	check  asl     return code			*/
/*......................................................................*/
int check_asl_stat ( rc)
int	rc;
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
