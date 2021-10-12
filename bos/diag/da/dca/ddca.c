static char sccsid[] = "@(#)30	1.16  src/bos/diag/da/dca/ddca.c, dadca, bos411, 9428A410j 2/19/93 15:39:30";
/*
 *   COMPONENT_NAME: DADCA
 *
 *   FUNCTIONS: cable_problem
 *		call_help
 *		clean_up
 *		intr_handler
 *		main
 *		run_tu
 *		tu_ctl
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#define NEW_DIAG_ASL	1
#define NEW_DIAG_CONTROLLER	1


#include <stdio.h>
#include <fcntl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/bit_def.h"
#include "tcatst.h"
#include "ddca_msg.h"
#include "dca.h"
#include <locale.h>

void intr_handler(int);
extern getdainput();		/* gets input from ODM */
extern long getdamode();	/* transforms input into a mode */
extern disp_menu();		/* displays menus */
extern user_quit();		/* cleanup routine */
extern int exectu();		/* test unit interface */
extern int errno;	
extern nl_catd	diag_catopen();

struct tm_input da_input;	/* initialized by getdainput */
long da_mode;			/* diagnostic modes */
int fdes = 0;			/* file descriptor from device driver */
nl_catd catd;			/* pointer to message catalog  */

short no_TU_5;			/* whether TU 5 should be executed */
int 	prior_state = NOT_CONFIG;

ASL_SCR_TYPE menutype = DM_TYPE_DEFAULTS;

char	dname[NAMESIZE]; 	/* device name to be tested */
char	msgstr[512];

/* This is the fru bucket structure for the DCA/TCA */
struct fru_bucket frub[MAX_BUCKET]=
{
  	{"", FRUB1, 0x854, 0x110, DCA_RMSG_1,
		{
	 		{60, "", "", 0, DA_NAME, EXEMPT},
			{40, "", "", 0, PARENT_NAME, EXEMPT},
		},
	},

  	{"", FRUB1, 0x854, 0x120, DCA_RMSG_2,
		{
	 		{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},
 
  	{"", FRUB1, 0x854, 0x130, DCA_RMSG_3,
		{
	 		{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},

  	{"", FRUB1, 0x854, 0x140, DCA_RMSG_4,
		{
	 		{100, "", "", 0, DA_NAME, EXEMPT},
		},
	},

  	{"", FRUB1, 0x854, 0x150, DCA_RMSG_5,
		{
	 		{95, "", "", 0, DA_NAME, EXEMPT},
			{5, "3270net", "", DCANETMSG, NOT_IN_DB, EXEMPT}
		},
	},

  	{"", FRUB1, 0x854, 0x900, DCA_RMSG_6,
		{
	 		{60, "", "", 0, DA_NAME, EXEMPT},
			{40, "", "", 0, PARENT_NAME, EXEMPT},
		},
	},

	/* fru bucket for configuration fails 	*/
  	{"", FRUB1, 0x854, 0x901, DCA_RMSG_7,
		{
	 		{80, "", "", 0, DA_NAME, EXEMPT},
			{10, "", "", 0, PARENT_NAME, EXEMPT},
			{10, " ", "", DCA_RMSG_9, NOT_IN_DB, EXEMPT},
		},
	},

	/* fru bucket for open device with return code EIO or ENODEV 	*/
  	{"", FRUB1, 0x854, 0x902, DCA_RMSG_8,
		{
	 		{80, "", "", 0, DA_NAME, EXEMPT},
			{20, "", "", 0, PARENT_NAME, EXEMPT},
		},
	},

};

/***************************************************************************/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the 3278/79 (TCA/DCA) Adapter 
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
 */

main()

{
	struct stat	statbuf;
	int rc = 0;		/* result of test unit execution */
				/* 0 indicates good path */
				/* -1 means device driver wouldn't open */
				/* SW_ERR means a software problem occurred */
				/* default means a test unit failed */

	struct sigaction invec;

        /* initialize interrupt handler structure       */
        invec.sa_handler = intr_handler;
	sigaction (SIGINT, &invec, (struct sigaction *) NULL);

	setlocale (LC_ALL, "");


        /*..............................*/
        /* General Initialization       */
        /*..............................*/

        DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_FULL);
	DA_SETRC_MORE (DA_MORE_NOCONT);

	/* initialize the ODM */
	init_dgodm();

	/* get diagnostic modes */
	getdainput(&da_input);
	strcpy (dname, da_input.dname);
	da_mode = getdamode(&da_input);

	if (da_mode & DA_CONSOLE_TRUE) 
		{
		/* open the catalog file for DCA */
		catd =  diag_catopen(CAT_NAME, 0);
		diag_asl_init(NULL);
		}


	/* if we are in either Problem Determination or Repair mode
         * and we are not in exit loop mode, then perform testing
	 * Otherwise skip this part
	*/
	if (((da_mode & DA_DMODE_PD) || (da_mode & DA_DMODE_REPAIR)) &&
	    (!(da_mode & DA_LOOPMODE_EXITLM)))
	{
		/* If there is a console then display a "testing" menu.
		 * Which one depends on the mode
		*/
		if (da_mode & DA_CONSOLE_TRUE) 
			if ((da_mode & DA_LOOPMODE_INLM) ||
			    (da_mode & DA_LOOPMODE_ENTERLM))
				disp_menu(TESTING_INLM);
			else if (da_mode & DA_ADVANCED_TRUE)
				disp_menu(STANDBY_ADVANCED);
			else disp_menu(STANDBY);

		rc = stat ("/usr/lib/drivers/c327dd", &statbuf);

		if 	/* device driver file missing or file size zero	*/
		   ((rc == -1 ) || ( statbuf.st_size ==0))
		{
			/* just send menugoal one time only */

                      	if  ((da_input.console == CONSOLE_TRUE) &&
                               ((da_input.loopmode == LOOPMODE_ENTERLM)
                               || (da_input.loopmode == LOOPMODE_NOTLM)))
                        {
				/* displaying the device driver */
				/* file error                   */
				sprintf (msgstr,(char *) diag_cat_gets (catd, 1,
				DCA_DRIVER_FILE_BAD), NULL);
				menugoal (msgstr);
			}
			DA_SETRC_STATUS (DA_STATUS_BAD);
			clean_up();

		}
		
		prior_state = configure_device(da_input.dname);
		if (prior_state != -1)
		{
			/* Execute test units */
			rc = tu_ctl();
		}
		else 
		{
			strcpy (frub[CONFIG_FRU].dname, dname);
			insert_frub(&da_input, &frub[CONFIG_FRU]);
			addfrub(&frub[CONFIG_FRU]);
			DA_SETRC_STATUS (DA_STATUS_BAD);
			clean_up();
		}

		/* Process the results */
		switch(rc)    
		{
		case -1:
			/* The device driver was busy */
			DA_SETRC_ERROR(DA_ERROR_OPEN);
			break;
		case 0:
			/* Good path */
			DA_SETRC_TESTS(DA_TEST_FULL);
			break;
		case SW_ERR:
			/* A software error occurred */
			DA_SETRC_TESTS(DA_TEST_FULL);
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			break;
		default:
			/* Testing found errors */
			DA_SETRC_TESTS(DA_TEST_FULL);
			DA_SETRC_STATUS(DA_STATUS_BAD);
			break;

		}
	}

	clean_up();

	/* Terminate the ODM */

} /*end main*/
/***********************************************************************/
/*
 * NAME: tu_ctl
 *
 * FUNCTION: An attempt is made to open the device driver.  If it opens,
 *           then run_tu is called sequentially for each test (1 to 5) 
 *           until all tests have been executed or an error is found.
 *
 * EXECUTION ENVIRONMENT:
 * 
 * RETURNS:		-1       The device driver failed to open
 *                       0       All tests passed
 *                       Other   A test unit failed
 *
 */

int tu_ctl()
{
	int rc;         /* return code from run_tu */
	long test;      /* test to be executed     */
	long menu_rc;	/* returned from keyboard check */
	int reason;	/* reason device driver won't open */
	char *devname;	/* name of device to be tested */
	char dname[] = "/dev/";	/* first part of device name */
	char openflag[] = "/D";	/* flag for diagnostic open */

	/* create device name */
	devname = (char * ) malloc(NAMESIZE + (strlen(dname)+1));
	if (devname == NULL)
		{
		rc = SW_ERR;
		return(rc);
		}
	strcpy(devname, dname);
	strcat(devname, da_input.dname);
	strcat(devname, openflag);
		
	/* attempt to open device driver */
	fdes = open(devname, O_RDWR);
	reason = errno;

	if (fdes == -1)         /* if device driver doesn't open */
	{
		switch(reason)
		{
			case EIO:
			case ENODEV:
			{
				strcpy (frub[OPEN_FRU].dname, da_input.dname);
				insert_frub(&da_input, &frub[OPEN_FRU]);
				addfrub(&frub[OPEN_FRU]);
				DA_SETRC_STATUS (DA_STATUS_BAD);
				rc = OPEN_ERROR;
			}
			case EBUSY:
			{
				if (da_mode && EXENV_CONC)
					rc = fdes;
				else 
					rc = SW_ERR;
				break;
			}
			default:
			{
				rc = SW_ERR;
				break;
			}
		}
		return (rc);
	}
	else 
	{
		rc = 0;
		test = 1;
		/*while more tests and no errors*/
		while ((rc == 0) && (test <= 5))
		{
			if (((da_mode & DA_CONSOLE_FALSE)
				|| (da_mode & DA_SYSTEM_TRUE)) && (test == 5))
				break;
			if ((da_mode & DA_LOOPMODE_INLM) && (test == 5))
			{
				/* no_TU_5 indicates whether TU 5 should be
			 	* executed.  If we are in loop mode, we
			 	* must find out what was determined in enter
			 	* loop mode before deciding whether to run
			 	* this test.
				*/
				getdavar(da_input.dname, "no_TU_5", DIAG_SHORT,
				    &no_TU_5);
				if (no_TU_5 == FALSE)
					rc = run_tu(test);
			}
			else rc = run_tu(test);
			if (rc == 0)
				{
				test ++;
				/* Check for ESC key entered by user. */
				disp_menu(CHECK_KBD);
				}
		}


		return(rc);
	}

}

/***********************************************************************/
/*
 * NAME: run_tu
 * 
 * FUNCTION: Calls exectu for the test specified as input.  If the test
 *           number is 5 (Connection Test) and the test fails, a series 
 *           of menus is displayed to isolate between the adapter, the
 *           cable and the control unit.  If the cable is missing or 
 *           damaged, the user will be asked whether a good cable is 
 *           available.  If one is, the user will be prompted to connect it
 *           and test 5 will be repeated.  Otherwise a goal menu will be
 *           returned to the diagnostic controller.   A fru bucket is reported
 *           if the trouble seems to be with the card.  
 *              
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:	0		No trouble found
 *              Nonzero		The return code from the failing test 
 */

int run_tu(test)

long test;                /* Test unit that will be executed */

{
	uchar nocab = FALSE;	/* whether a problem was found with the   */
				/* cable (missing or damaged)             */
	int rc = 0;             /* return code from test unit             */
	TUTYPE tucb;		/* test unit control block                */
	long menu_rc;		/* return value from menu display routine */

	/* Set up parameters for test unit execution */
	tucb.header.tu = test;
	tucb.header.mfg = 1;    /* must run in mfg mode instead of htx mode */
	tucb.header.loop = 1;

	/* execute test unit */
	rc = exectu(fdes, &tucb);

	if (da_mode & DA_LOOPMODE_ENTERLM)
	{
		/* in enter loop mode, initially assume we can run TU 5 */
		/* Store this value in the ODM for loop mode use */
		no_TU_5 = FALSE;
		putdavar(da_input.dname, "no_TU_5", DIAG_SHORT, &no_TU_5);
	}

	if ((da_mode & DA_CONSOLE_TRUE) &&
	    ((da_mode & DA_LOOPMODE_NOTLM) ||
	    (da_mode & DA_LOOPMODE_ENTERLM)))
	{
		if ((test == 5) && (rc != 0))
		{
			/* Connection Test failed */
			/* Ask user if the adapter should be communicating
			 * with the control unit */
			menu_rc = disp_menu(SHOULD_BE_COMM);

			if (menu_rc == NO)
			{
				rc = 0;
				if (da_mode & DA_LOOPMODE_ENTERLM)
				{
					/* User indicated there is not supposed
					 * to be communication.  Set flag to
					 * store this for later. Also, store in
					 * the ODM so that loop mode will know.
					*/
					no_TU_5 = TRUE;
					putdavar(da_input.dname, "no_TU_5",
					    DIAG_SHORT, &no_TU_5);
				}
			}
			else 
			{
				/* User indicated that there is supposed
				 * to be communication.  Find out if there
				 * is a cable connecting the adapter to the
				 * control unit.
				*/
				menu_rc = disp_menu(IS_CABLE_CONNECTED);

				if (menu_rc == YES)
				{
					/* cable is connected */
					/* find out if cable is damaged */
					menu_rc = disp_menu(IS_CABLE_DAMAGED);
					if (menu_rc == YES)
						/* Cable is damaged.  Ask user
						 * to replace. Non-zero rc
						 * indicates that either no
						 * cable was available or it
						 * didn't fix the problem.
						 * nocab = TRUE means no cable
						 * was available.
						*/
						rc = cable_problem(rc, &nocab,
						    &tucb);
					else
						/* Cable isn't damaged.
						 * Have user call help desk.
						 * Non-zero rc indicates that
						 * there is no problem with
						 * control unit or system
						*/
						rc = call_help(rc);
				}
				else
					/* Cable not connected. Ask user
					 * to connect. Non-zero rc
					 * indicates that either no
					 * cable was available or it
					 * didn't fix the problem.
					 * nocab = TRUE means no cable
					 * was available.
					*/
					rc = cable_problem(rc, &nocab, &tucb);
			}
		}
	}
	if ((rc != 0) && (!nocab))
	{
		/* A test unit failed.
		 * If the connection test failed, a goal menu was reported
		 * if no cable was available.  (No fru bucket reported.) 
		 * Report a fru bucket.
		*/
		strcpy (frub[test-1].dname, dname);
		insert_frub(&da_input, &frub[test-1]);
		addfrub(&frub[test-1]);
	}
	return(rc);
}
/***********************************************************************/
/* 
 * NAME: cable_problem
 * 
 * FUNCTION: This routine is called if the cable is missing or damaged.
 *           If there is no cable available for replacement (according
 *           to user response to displayed menu) a goal menu is returned
 *           to the diagnostic controller and a variable (*nocabptr) is 
 *           set to TRUE.  Otherwise, the user is prompted to replace the
 *           cable and the Connection test is run again.  If the test
 *           fails (for the second time), the user is asked to call the
 *           help desk to find out if there is a problem with the host
 *           system or the control unit.
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  0     - (1) the user connected a cable and that
 *                       corrected the problem
 *                   (2) the user connected the cable and the
 *                       problem still existed.i  However the
 *                       user then called the help desk and
 *                       was informed of a host sytem or
 *                       control unit problem.
 *           nonzero - (1) a cable was not available and a 
 *                         menugoal was returned to the 
 *                         diagnostic controller
 *                     (2) the user connected a cable and
 *                         that did not fix the problem.
 *                         In addition, the help desk re-
 *                         ported no problem with the host
 *                         system or with the control unit.
 */

int cable_problem(rc, nocabptr, tucbptr)
int rc;                   /* return code from TU 5              */
uchar *nocabptr;          /* pointer to flag indicating availability
                             of replacement cable               */
TUTYPE *tucbptr;  	  /* pointer to test unit control block */

{
	char *connect_cable;      /* pointer to goal menu message    */ 
	long menu_rc;             /* return code from menu display e */

	/* Ask user if there is a cable available */
	menu_rc = disp_menu(IS_CABLE_AVAILABLE);

	if (menu_rc == NO)
	{
		/* if no cable is available for replacement  */
		/* return goal menu to diagnostic controller */
		*nocabptr = TRUE;
		if (da_mode & DA_LOOPMODE_ENTERLM)
		{
			/* Set flag so TU 5 won't be run */
			/* In enter loop mode, store flag in ODM to let
			 * loop mode know not to run TU 5 (Connection Test
			*/
			no_TU_5 = TRUE;
			putdavar(da_input.dname, "no_TU_5", DIAG_SHORT,
			    &no_TU_5);
		}
		/* Pass goal menu to controller which will ask user to get
		 * a replacement cable as a possible fix.
		*/
		menu_rc = disp_menu(CONNECT_CABLE);
	}
	else 
	{
		/* User indicated a replacement cable was available.
		 * Prompt user to connect cable.
           	 * Then rerun test unit.  If still bad, 
          	 * have user call help desk.
		*/
		menu_rc = disp_menu(PLEASE_CONNECT);
		rc = exectu(fdes,tucbptr);
		if (rc != 0)
			rc = call_help(rc);
	}
	return(rc);
}
/***********************************************************************/
/*
 * NAME: call_help
 * 
 * FUNCTION: This routine displays a menu requesting the user to call
 *           the help desk to find out if there is a problem with the
 *           control unit or host system.  If there is, the return code,
 *           whose value was returned from test unit 5 (Connection Test),
 *           is changed to 0 to indicate that there is no problem with
 *           the adapter and no frus need be reported. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
 *              0	- the help desk reported a problem
 *		nonzero - the help desk reported no problems
 *
 */

int call_help(rc)
int rc;		/* Return code from previously run Connection Test.
		 * If the problem is the Control Unit or system,
		 * it will be changed to 0.  Otherwise it is returned as is.
		*/
{
	long menu_rc;		/* user input returned from menu display */

	/* Ask user to call help desk and find out if the problem is
	 * with the Control Unit or system.
	*/
	menu_rc = disp_menu(IS_CU_PROBLEM);
	if (menu_rc == YES)
	{
		/* Problem is Control Unit or system.  If in enter loop mode
		 * tell ODM that TU5 shouldn't be run in loop mode.
		*/
		if (da_mode & DA_LOOPMODE_ENTERLM)
		{
			no_TU_5 = TRUE;
			putdavar(da_input.dname, "no_TU_5", DIAG_SHORT,
			    &no_TU_5);
		}
		/* Change return code so that no fru bucket is returned. */
		rc = 0;
	}
	return(rc);
}
/***********************************************************************/

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

/*----------------------------------------------------------------------*/
/*      Procedure               : cleanup                               */
/*      Description             : this will clean up DA before exit     */
/*                                DA. Like unconfigure, close odm       */
/*----------------------------------------------------------------------*/

clean_up ()
{
        if (da_input.console == CONSOLE_TRUE)
        {
                close (catd);
                diag_asl_quit ((char *)NULL);
        }
        if (fdes != 0)
                close (fdes);

        if ( prior_state != NOT_CONFIG)
                initial_state (prior_state, da_input.dname);
        DA_EXIT ();
}
