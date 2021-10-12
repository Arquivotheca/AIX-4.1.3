static char sccsid[] = "@(#)23	1.29  src/bos/diag/da/tok/dtoken.c, datok, bos41J, 9523B_all 6/5/95 15:46:04";
/*
 *   COMPONENT_NAME: DATOK
 *
 *   FUNCTIONS: chk_asl_stat
 *		clean_up
 *		intr_handler
 *		main
 *		run_ela
 *		run_tu
 *		runtests
 *		testsequence
 *		performing_tests
 *		
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <nl_types.h>
#include <limits.h>
#include <sys/cfgodm.h>
#include "tr_mon_errids.h"
#include <errno.h>
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/dcda_msg.h"
#include "diag/bit_def.h"
#include "toktst.h"
#include "trn.h"
#include "dtoken_msg.h"
#include <locale.h>
extern struct fru_bucket frub711;
extern struct fru_bucket frub811;
extern struct fru_bucket frub720;
extern struct fru_bucket frub820;
extern struct fru_bucket frub770;
extern struct fru_bucket frub880;
extern struct fru_bucket frub902; /* fru bucket for ELA */
extern struct fru_bucket frub904; /* fru bucket for ELA */
extern struct fru_bucket frub905; /* fru bucket for configuration error  */
extern struct fru_bucket frub906; /* fru bucket for EIO and ENODEV in open */

extern	nl_catd	diag_catopen();
void    intr_handler(int);	/* DA interrupt handler	*/
void 	clean_up();
extern getdainput();		/* gets input from ODM */
extern long getdamode();	/* transforms input into a mode */
extern int disp_menu();		/* displays menus */
extern int exectu();		/* test unit interface */
extern int errno;		/* reason device driver won't open */
extern int check_net();		/* find out why TU1 fails */
extern int try_again();		/* retry TU1 after failure */
extern test_display();		/* display "testing" message */
extern report_fru ();
extern invoke_method();
 
char	*msgstr;
char	option[256];
char	*new_out, *new_err;
int	diag_device_configured=FALSE;
int	diag_device_diagnose=FALSE;
struct tm_input da_input;	/* holds input from ODM */
long da_mode;			/* diagnostic modes */
int fdes = 0;			/* device driver file descriptor */
uchar wrap_attempt = FALSE;	/* whether a wrap test was attempted */
short testfailed = 0;		/* which test unit failed */
short wiring = DONT_KNOW;	/* type of wiring - tele or not */
short no_net = FALSE;		/* user indication that net is not up */
short speed;			/* ring speed at which test unit runs */
TUTYPE tucb;			/* test unit control block */
nl_catd catd;			/* pointer to token ring catalog file */
char *devname;			/* name of device to be tested */
char	dname[NAMESIZE];     	/* device name			*/
int 	prior_state;
int	alarm_timeout=FALSE;
void	timeout(int);
void	set_timer ();
struct  sigaction  invec;       /* interrupt handler structure  */
void	general_initialize ();
int	unconfigure_lpp_device();
int	configure_lpp_device();
int	unconfigure_diag_device();
int	configure_diag_device();


/***************************************************************************/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the Token Ring Network Adapter 
 *           Diagnostic Application.  It is invoked by the diagnostic
 *           Controller.                           
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:	DA_STATUS_GOOD
 * 			No problems were found.
 *		DA_STATUS_BAD
 *			A FRU Bucket or a Menu Goal was
 *			reported.
 *		DA_USER_NOKEY
 *			No special function keys were entered.
 *		DA_USER_EXIT
 *			The Exit Key was entered by the user.
 *		DA_USER_QUIT
 *			The Quit Key was entered by the user.
 *		DA_ERROR_NONE
 *			No errors were encountered performing a
 *			normal operation such as displaying a
 *			menu, accessing the object repository,
 *			allocating memory, etc.
 *		DA_ERROR_OPEN
 *			Could not open the device.
 *		DA_ERROR_OTHER 
 *			An error was encountered performing
 *			a normal operation.
 *		DA_TEST_NOTEST
 *			No tests were executed.
 *		DA_TEST_FULL
 *			The full-tests were executed.
 *		DA_TEST_SUB
 *			The sub-tests were executed.
 *		DA_TEST_SHR
 *			The shared tests were executed.
 *		DA_MORE_NOCONT
 *			The isolation process is complete.
 *		DA_MORE_CONT
 *			The path to the device should be
 *			tested.
 *
 */
                      
main()

{
	int 	rc = 0;			/* result of test unit execution */

	general_initialize ();

	/* if we are in repair mode or problem determination mode,
	 * we can perform testing.  Otherwise skip this part.
	*/
        if ((da_mode & DA_DMODE_PD) || (da_mode & DA_DMODE_REPAIR)) 
       	{


		/* if we have a console and aren't in exit loop mode
		 * we can display a "testing" message.  Which
		 * message depends on what mode we are in.
		*/
        	if ((da_mode & DA_CONSOLE_TRUE)
	       	    && (!(da_mode & DA_LOOPMODE_EXITLM)))
			test_display();

 		prior_state=get_device_status(da_input.dname);
		rc = unconfigure_lpp_device();
	
		if (prior_state != -1)
		{
			/* Perform testing and/or handle wrap plugs.
		 	*  rc = 0 means good path.
			*/
           		rc = performing_tests();
		}

         }

	/* if no test unit failed in problem determination mode, or we are in
	 * error log analysis mode, we run ELA.
	 * We don't run ELA if we are in loop mode or exit loop mode.
	*/ 
	if ((((da_mode & DA_DMODE_PD) && (rc <= 0)) || (da_mode & DA_DMODE_ELA))
		&& ((da_mode & DA_LOOPMODE_NOTLM) ||
			(da_mode & DA_LOOPMODE_ENTERLM)))
		/* rc indicates whether ELA found a problem.  If it 
		 * did not, rc remains the result of testing (if performed.)
		*/
		rc = run_ela(rc);
	clean_up ();
	
} /*  end main */
/*------------------------------------------------------------------------
 * NAME: performing_tests
 *
 * FUNCTION: Opens device driver and calls routine to handle execution
 * 	     of test units. 
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  -1		The device driver failed to open
 *	     0x9999	A software error occurred	
 *           0		All tests passed
 *           Other	A test unit failed
 *
 *-------------------------------------------------------------------------*/

int performing_tests()
{
	int rc = 0;  	/* test results */ 
	int reason;	/* reason device driver doesn't open */

	/* wrap has not yet been performed.  Initialize flag and store in
	 * ODM for loop mode.  */

	wrap_attempt = FALSE;
	if ((da_mode & DA_LOOPMODE_ENTERLM) && (da_mode & DA_CONSOLE_TRUE))
		putdavar(da_input.dname, "wrap_attempt", DIAG_SHORT,
			&wrap_attempt);

	/* if not in exit loop mode, testing can be performed. */
	if (!(da_mode & DA_LOOPMODE_EXITLM))
	{ 

		/* create device name */
		devname = (char * ) malloc(NAMESIZE + (strlen(dname)+1));
		if (devname == NULL)
		{
			DA_SETRC_ERROR(DA_ERROR_OTHER);
			clean_up();
		}
		sprintf(devname,"/dev/%s/D",da_input.dname);


		/* attempt to open device driver. */
		fdes = open(devname, O_RDWR | O_NDELAY);
		reason = errno;

		if (fdes == -1)         /* if device driver doesn't open */
		{
			switch(reason)
			{
				case EIO:
				case ENODEV:
					report_fru (&frub906);
					DA_SETRC_STATUS (DA_STATUS_BAD);
					clean_up();
					break;
				case EBUSY:
				case ENOTREADY:
				default:
					DA_SETRC_ERROR(DA_ERROR_OPEN);
					clean_up();
					break;
			}
		}
		/* run tests - rc indicates results of testing. */
		/* report fru bucket if necessary */
		rc = runtests();

	}
	if (((da_mode & DA_LOOPMODE_NOTLM) || (da_mode & DA_LOOPMODE_EXITLM))
	    && (da_mode & DA_CONSOLE_TRUE))
	{
		/* perform cleanup from wrap tests if necessary. */
		if (da_mode & DA_LOOPMODE_EXITLM)

			/* find out if a wrap test was performed during
			 * enter loop mode and during loop mode.  */

			getdavar(da_input.dname, "wrap_attempt", DIAG_SHORT,
				&wrap_attempt);
		
		if (wrap_attempt)
		{
			/* find out if telephone wiring was used */
			if (da_mode & DA_LOOPMODE_EXITLM)
				getdavar(da_input.dname, "wiring", DIAG_SHORT,
					&wiring);
			switch(wiring)
			{
				case NONTELE:
					/* have user attach cable to network */
					disp_menu(ATTACH_CABLE);
					break;
				case TELE:
					/* have user detach wrap plug 
					 * and attach cable */

					disp_menu(DETACH_WRAP);
					break;
			}
		}
	}
	return(rc);
}

/*------------------------------------------------------------------------
 * NAME: runtests
 *
 * FUNCTION:  Sets up ring speed on adapter and calls a routine that 
 *	      runs the other test units.  Processes errors from TUs 7 & 8.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: -1		The device driver failed to open
 *	    0x9999	A software error occurred	
 *          0		All tests passed
 *          Other	A test unit failed
 *
 *------------------------------------------------------------------------*/

int runtests()

{
	int rc = 0;		/* result of testing */

	if ((da_mode & DA_LOOPMODE_ENTERLM) && (da_mode & DA_CONSOLE_TRUE))
	{
		no_net = FALSE;
		putdavar(da_input.dname, "no_net", DIAG_SHORT, &no_net);
	}

	/* set flag indicating ring speed set by TU 7 to 4MB. */
	speed = 7;

	/* set up parms for  test units */
	tucb.header.mfg = 0;
	tucb.header.loop = 1;
	tucb.header.r1 = 0;

	tucb.header.tu = 7;

	/* run TU 7 */
	rc = run_tu();

	/* Process results of testing */
	/* Report a fru_bucket if rc is recognized error code */
	/* On return, rc = SW_ERR indicates a software error */
	if (rc != 0)
	{
		rc = check_rc_tu7(rc);
		return(rc);
	}

	/* execute TUs 1 to 3 at speed 4MB unless one fails.
	 * rc indicates test results
	*/
	rc = testsequence();
	if ((rc != 0) || (no_net))
		return(rc);
	
	/* all tests passed at 4MB ring speed.
	 * Have to close and open device driver before we
	 * can re-execute TU 1.  */

	/* ignore the interrrupt signal until close is done 	*/
	/* the reason it's done this way. If the close is done	*/
	/* and interrupt happen in clean up it's closed one	*/
	/* one more time and this going to prevent it happen	*/

	(void) signal (SIGINT, SIG_IGN);
	close(fdes);
	fdes = 0;
	/* restore the interrrupt 	*/
       	(void) signal (SIGINT, intr_handler);

	fdes = open(devname, O_RDWR | O_NDELAY);
	/* if open fails returns 		*/
	if (fdes == -1)
		return(fdes);

	/* change flag to indicate speed set by TU 8 to 16MB. */
	speed = 8;
	
	/* execute TU 8.  rc indicates results. */
	tucb.header.tu = 8;

	tucb.header.loop = 1;
	tucb.header.r1 = 0;
	rc = run_tu();

	/* Process results of testing */
	/* Report a fru_bucket if rc is recognized error code */
	/* On return, rc = SW_ERR indicates a software error */
	if (rc != 0)
	{
		rc = check_rc_tu8(rc);
		return(rc);
	}

	/* execute TUs 1 to 3 at ring speed 16 MB.
	 * rc indicates results.
	*/
	rc = testsequence();
	return(rc);
}
/*----------------------------------------------------------------------
 * NAME: testsequence
 *
 * FUNCTION:  runs TUs 1 through 3 and handles user interaction necessary
 *            for wrap tests.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: -1		The device driver failed to open
 *	    0x9999	A software error occurred	
 *          0		All tests passed
 *          Other	A test unit failed
 *
 *----------------------------------------------------------------------*/
int testsequence()

{

	int rc = 0;		/* result of testing */
	int temp_rc;		/* result of TU 3 */
	int menu_rc;		/* user input from menu */
	int counter;		/* using as counter	*/

	/* run test 1 */
	tucb.header.tu = 1;
	tucb.header.loop = 1;
	tucb.header.r1 = 0;

	rc = run_tu();

	/* if lobe media wrap test fails, find out why and attempt to fix */
	if ((rc == 0x01010211) && (speed == 7))   
	{
        	if ((da_mode & DA_CONSOLE_TRUE) && (da_mode & DA_SYSTEM_FALSE))
			rc = check_net(rc);
		else
		{
			no_net = TRUE;
			rc = 0;
		}
	}

	/* if the device driver would not reopen, quit */
	if (rc == -1)
		return(rc);

	/* if TU1 fails, check its return code and either report a fru or
	 * set rc to SW_ERR */
	if ((rc != 0) && (rc != BAD_NET))
		rc = check_rc_tu1(rc);
	
	if ((rc == 0) && (!(no_net)))
		/* TU1 passed.  no_net indicates that rc = 0 because
		 * user indicated network was not functioning
		*/
	{
		/* run test 2 */
		counter = 2048;
		tucb.header.tu = 2;
		tucb.header.r1 = counter;

		for (counter = 2048; counter >=2025; counter --)
		{
			tucb.header.r1 = counter;
			rc = run_tu(); 

			/* if TU2 fails, check its return code and either report
		 	 * a fru or set rc to SW_ERR */
			if (rc != 0)
			{
				rc = check_rc_tu2(rc);
				break;
			}

		}
	}

	/* run TU 3 . This test unit must be run no matter what */
	/* It just a end session test				*/
	tucb.header.r1 = 0;
	tucb.header.loop = 1;
	tucb.header.tu = 3;

	/* TU 3 has no FRU bucket associated.
	 * Results are stored in temp_rc to preserve rc from 
	 * previous test.  */

	temp_rc = run_tu();

	if (((rc == 0) && (!(no_net))) && (temp_rc != 0))
	{
		/* only TU 3 has failed. */
		/* rc = SW_ERR; */
		/* commented out because hw has already been tested
		   at this point
		*/
	}

	return(rc);
}
/*------------------------------------------------------------------------
 * NAME: run_tu
 *
 * FUNCTION:  runs the tu specified in tucb.header.tu
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: Return code from exectu
 *
 *-------------------------------------------------------------------------*/
int run_tu()
{
	int rc, test_unit;
	
	rc = 0;
	test_unit = tucb.header.tu;

	alarm_timeout = FALSE;
	set_timer ();
	rc = exectu(fdes, &tucb);          /* execute test unit */
	if (alarm_timeout)
	{
		switch (test_unit)
		{
			case 1:
			{
				switch (speed)
				{
					case 7:
						report_fru (&frub711);
						clean_up ();
						break;
					case 8:
						report_fru (&frub811);
						clean_up ();
						break;
					default:
						clean_up ();
						break;
				}
			}
			break;
			case 2:
			{
				switch (speed)
				{
					case 7:
						report_fru (&frub720);
						clean_up ();
						break;
					case 8:
						report_fru (&frub820);
						clean_up ();
						break;
					default:
						break;
				}
			}
			break;
			case 7:
				report_fru (&frub770);
				clean_up ();
				break;

			case 8:
				report_fru (&frub880);
				clean_up ();
				break;
			default:
				break;
		}
	}
	/* check if the F3 or F10 key is pressed		*/
        if ( da_input.console == CONSOLE_TRUE )
  		chk_asl_stat 
			( diag_asl_read ( ASL_DIAG_LIST_CANCEL_EXIT_SC, 0,0));
	
	return(rc);
}

/*------------------------------------------------------------------------
 * NAME: run_ela 
 *
 * FUNCTION: performs error log analysis.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 
 *
 *------------------------------------------------------------------------*/
int run_ela(rc)
	int rc;
{
	int ela_rc;
	char crit[256];
	struct errdata err_data;
	char msgstr[512]; 		/* holds message for substitution */


	sprintf(crit, "%s -N %s -j %X,%X,%X,%X", da_input.date,
	    da_input.dname, ERRID_CTOK_ADAP_CHECK, ERRID_CTOK_DOWNLOAD,
	     ERRID_CTOK_AUTO_RMV, ERRID_CTOK_WIRE_FAULT);
	ela_rc = error_log_get(INIT, crit, &err_data);
	error_log_get(TERMI, crit, &err_data);
	if (ela_rc > 0)
	{
		report_fru(&frub902);
		clean_up();
	}
	else if (da_mode & DA_CONSOLE_TRUE)
	{
		sprintf(crit, "%s -N %s -j  %X", da_input.date,
		    da_input.dname, ERRID_CTOK_WIRE_FAULT);	
		ela_rc = error_log_get(INIT, crit, &err_data);
		error_log_get(TERMI, crit, &err_data);
		if (ela_rc > 0)
		{
			if ((da_mode & DA_DMODE_PD) && (rc == 0))
				sprintf(msgstr, (char *) diag_cat_gets (catd,
				 1, ELA_NET_BAD_PD0), da_input.dnameloc);
			if ((da_mode & DA_DMODE_PD) && (rc == -1))
			{
				DA_SETRC_TESTS(DA_TEST_FULL);
				DA_SETRC_ERROR(DA_ERROR_NONE);
				sprintf(msgstr, (char *)diag_cat_gets(catd, 1,
			    	    ELA_NET_BAD_PD1), da_input.dnameloc);
			}
			if (da_mode & DA_DMODE_ELA)
				sprintf(msgstr, (char *)diag_cat_gets(catd, 1,
			    	    ELA_NET_BAD_ELA), da_input.dnameloc);
			menugoal(msgstr);
			clean_up();
		}
		else
		{
			sprintf(crit, "%s -N %s -j %X,%X,%X",
			    da_input.date, da_input.dname,
			    ERRID_CTOK_DUP_ADDR, ERRID_CTOK_RMV_ADAP);
			ela_rc = error_log_get(INIT, crit, &err_data);
			error_log_get(TERMI, crit, &err_data);
			if (ela_rc > 0)
			{
				if ((da_mode & DA_DMODE_PD) && (rc == 0))
					sprintf(msgstr, (char *) 
					    diag_cat_gets(catd, 1,
				    	    ELA_ADAPTER_OR_NET_BAD_PD0),
				    	    da_input.dnameloc);
				if ((da_mode & DA_DMODE_PD) && (rc == -1))
				{
					DA_SETRC_ERROR(DA_ERROR_NONE);
					DA_SETRC_TESTS(DA_TEST_FULL);
					sprintf(msgstr, (char *)
					    diag_cat_gets(catd, 1,
				    	    ELA_ADAPTER_OR_NET_BAD_PD1),
				    	    da_input.dnameloc);
				}
				if (da_mode & DA_DMODE_ELA)
					sprintf(msgstr, (char *) 
					    diag_cat_gets(catd, 1,
				    	    ELA_ADAPTER_OR_NET_BAD_ELA),
				    	    da_input.dnameloc);
				menugoal(msgstr);
				clean_up();
			}
		}
	}
	return(rc);
}
/*----------------------------------------------------------------------*/
/*	Function	: clean_up					*/
/*----------------------------------------------------------------------*/
void clean_up ()
{
	int	rc;

	/* if the tu is 1 then we need to run tu 3 and exit 	*/
	if (tucb.header.tu == 1)
	{
		tucb.header.tu = 3;
	 	exectu(fdes, &tucb);          /* execute test unit */
	}
	

	if (fdes != NULL) close (fdes);

	/*  Unload the diagnostics device driver and set the lpp device */
	/*  to DEFINE state						*/
	configure_lpp_device();

        if (da_mode & DA_CONSOLE_TRUE)
	{
		/* quit ASL */
		diag_asl_quit(NULL);

		/* close the catalog file */
		catclose(catd);
	}
	/* terminate the ODM */
	term_dgodm();
	DA_EXIT();
 
}

/*------------------------------------------------------------------------
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
 *-------------------------------------------------------------------------*/

void intr_handler(int sig)
{
        if ( da_input.console == CONSOLE_TRUE )
                diag_asl_clear_screen();
        clean_up();
}
	

/*-------------------------------------------------------------------------*/
/*	chk_asl_stat	: this function will read the keyboard if	   */
/*			  F3 or F10 key is pressed			   */
/*			  if it's press then  cleaning up and return	   */
/*-------------------------------------------------------------------------*/
chk_asl_stat(returned_code)
long    returned_code;
{
        switch ( returned_code )
        {
                case DIAG_ASL_OK:
			break;
                case DIAG_ASL_CANCEL:
                {
                        DA_SETRC_USER(DA_USER_QUIT);
                        clean_up();
                        break;
                }
                case DIAG_ASL_EXIT:
                {
                        DA_SETRC_USER(DA_USER_EXIT);
                        clean_up();
                        break;
                }
                default:
                        break;
        }
}
/*---------------------------------------------------------------------------
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
 *--------------------------------------------------------------------------*/

void timeout(int sig)
{
        alarm(0);
        alarm_timeout = TRUE;
}

/*---------------------------------------------------------------------------
 * NAME: set_timer()
 *
 * FUNCTION: Set up the handler when SIGALARM is triggered 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/


void set_timer()
{
        invec.sa_handler = timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(300);
}
/*----------------------------------------------------------------------
 * NAME: general_initialize ()
 *
 * FUNCTION: Initialization for Diagnostics Application 
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *------------------------------------------------------------------------*/
void general_initialize ()
{
	char	file_error;
	char	diagenv[128];
	char	microcode_path[128];
	int 	diskette_mode, diskette_based, rc;
	int	diag_source=0;
	struct 	stat tmpbuf;

	/* initializing interrupt handler */
        (void) signal (SIGINT, intr_handler);

        /*..............................*/
        /* General Initialization       */
        /*..............................*/

       	DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_FULL);
        DA_SETRC_MORE (DA_MORE_NOCONT);




	setlocale (LC_ALL, "");

	/* initialize ODM */
	init_dgodm();

	/* get input from ODM */
        getdainput(&da_input);
	strcpy (dname, da_input.dname);

	/* transform input into mode */
        da_mode = getdamode(&da_input);

	/* Try to open catalog file.  If it won't open set error code and exit
	 * to diagnostic controller.
	*/
        if (da_mode & DA_CONSOLE_TRUE)
	{
		catd = diag_catopen(MF_DTOKEN, 0);
		/* initialize ASL */
		diag_asl_init("NO_TYPE_AHEAD");
	}
	/* checking the microde files */
	msgstr = (char *) malloc (1200 *sizeof (char));

	if(msgstr == (char *) NULL) 
	{
		clean_up();
	} 
	diag_source=diag_exec_source(&diagenv);

	diskette_mode = ipl_mode(&diskette_based);
        if (diskette_based == DIAG_FALSE)
	{
		/* checking the microcode file 	*/
		rc = findmcode ("8fc8m.00",&microcode_path,0, ""); 

		file_error = FALSE;
		rc =stat(&microcode_path, &tmpbuf);
		if ( (rc == -1) || ( tmpbuf.st_size == 0))		
		{
			file_error = TRUE;
		}
		/* checking the microcode file 	*/
		memset (&microcode_path, 0, 128);
		rc = findmcode ("8fc8p.00",&microcode_path,0, ""); 
		rc = stat(&microcode_path, &tmpbuf);
		if ( (rc == -1) || ( tmpbuf.st_size == 0))		
		{
			file_error = TRUE;
		}
		if (file_error == TRUE)
		{
			/* display a menugoal 		*/

			if  ((da_input.console == CONSOLE_TRUE) &&
                                   ((da_input.loopmode == LOOPMODE_ENTERLM)
                                   || (da_input.loopmode == LOOPMODE_NOTLM)))
			{
 			     	sprintf( msgstr, (char *) diag_cat_gets(catd, 1,
                                      	MICROCODE_BAD), da_input.dnameloc);
				menugoal(msgstr);
			}
			DA_SETRC_STATUS (DA_STATUS_BAD);
			/* cleanup and exit DA	*/
			clean_up ();			

		}

		/* checking for diagnostics device driver	*/
		/* but not if running from mounted CDROM	*/
		if(diag_source == 0){
			rc = stat ("/usr/lib/drivers/tokdiag", &tmpbuf);
       	 		if ((rc == NOT_GOOD) || (tmpbuf.st_size == 0))
        		{
                		if  ((da_input.console == CONSOLE_TRUE) &&
                     		(da_input.loopmode & 
			        ( LOOPMODE_ENTERLM | LOOPMODE_NOTLM)))
                		{
                        		sprintf(msgstr, (char *)diag_cat_gets
                               		   (catd, 1,TOK_DD_FILE_BAD), NULL);
                        		menugoal (msgstr);
                		}
                		DA_SETRC_STATUS (DA_STATUS_BAD);
                		clean_up();
			}
		}
	}
}

/*---------------------------------------------------------------------------
 * NAME: configure_lpp_device () 
 *
 * FUNCTION: configure_lpp_device ()
 *	     Unload the diagnostic device driver 
 *	     setting the device to the DEFINE state (No matter what state
 *	     it is in before running the test. We do not want left it in
 *	     DIAGNOSE state after running diagnostics 
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
		strcpy (criteria,"/usr/lib/methods/cfgddtok");
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
/*      NAME: unconfigure_lpp_device                            */
/*      Description:    					*/
/*	Return		: 0 Good 				*/
/*			  -1 BAD				*/
/*--------------------------------------------------------------*/
int	unconfigure_lpp_device()
{
	int	result;
        char    criteria[128];

	result = diagex_cfg_state ( da_input.dname );

	switch (result) {
		case 2: 
			/* sprintf(msgstr, (char *)diag_cat_gets (
					catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
				 	da_input.dname, da_input.dnameloc);
			menugoal (msgstr); */
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
	strcpy (criteria,"/usr/lib/methods/cfgddtok");
	result = invoke_method(criteria,option, &new_out,&new_err);
	free(new_out);
	free(new_err);
	if (result == NO_ERROR)
	{
		diag_device_configured=TRUE;
	}
	else
	{
		/* report frubucket and exit DA		*/
		report_fru(&frub905);
		DA_SETRC_STATUS (DA_STATUS_BAD);
		clean_up();
	}

	return (0);
}
