static char sccsid[] = "@(#)25	1.7  src/bos/diag/da/tok/extra.c, datok, bos411, 9428A410j 4/27/94 09:57:41";
/*
 *   COMPONENT_NAME: DATOK
 *
 *   FUNCTIONS: check_net
 *		test_display
 *		try_again
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
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag_exit.h"
#include "diag/bit_def.h"
#include "toktst.h"
#include "trn.h"
#include "dtoken_msg.h"

extern short no_net;
extern short wiring;
extern disp_menu();		/* displays menus */
extern long da_mode;		/* diagnostic modes */
extern int run_tu();		/* runs a test unit */
extern int errno;		/* reason device driver won't open */
 
extern struct tm_input da_input;/* holds input from ODM */
extern int fdes;		/* device driver file descriptor */
extern uchar wrap_attempt;	/* whether a wrap test was attempted */
extern TUTYPE tucb;			/* test unit control block */
extern char *devname;			/* name of device to be tested */

/*
 * NAME: check_net
 *
 * FUNCTION: Handles menu sequence to check and set up cables when
 *	     TU1 fails because of wrap failure.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:  
 *
 */
int check_net(rc)

int rc;

{
int menu_rc;
		
	if (da_mode & DA_LOOPMODE_INLM)
	{
		/* if failure occurs during loop mode, find out whether
		 * user previously specified that the network was not up.
		 * if so, ignore the failure.  Otherwise find out whether
		 * wrap is set up.         
		*/
		getdavar(da_input.dname, "no_net", DIAG_SHORT, &no_net);
		if (no_net == TRUE)
			rc = 0;
		else
			getdavar(da_input.dname, "wrap_attempt", DIAG_SHORT,
				&wrap_attempt);
		return(rc);
	}
	else
	{
		/* not in loop mode.  Ask user whether network is
		 * supposed to be working.  If not, ignore return code.
		 * If enter loop mode, save flag for later
		*/
		menu_rc = disp_menu(NETWORK_UP);
		if (menu_rc == NO)
		{
			no_net = TRUE;
			rc = 0;
			if (da_mode & DA_LOOPMODE_ENTERLM)
				putdavar(da_input.dname, "no_net", DIAG_SHORT,
				    &no_net);
			test_display();
			return(rc);
		}
		/* find out whether telephone wiring is used.
		* Also store flag in ODM if enter loop mode. */
		wiring = disp_menu(WIRING_TYPE); 
		if (da_mode & DA_LOOPMODE_ENTERLM)
			putdavar(da_input.dname, "wiring", DIAG_SHORT, &wiring);
		if ((wiring == DONT_KNOW) || ((wiring == TELE)
	    	    && (da_mode & DA_ADVANCED_FALSE)))
		{
			/* call out adapter, cable, and net */
			test_display();
			return(rc);
		}

		/* change to two menus, one for tele, one not */
		if (wiring == NONTELE)
			disp_menu(CHECK_CABLE); 
		else if (wiring == TELE)
			disp_menu(CHECK_CABLE_TELE);
 		/* close device and try TU 1 again */
		test_display();
		rc = try_again(); 
		if (rc == 0x01010211) 
			/* try again only through cable */
			/* instead of through network */
		{
			/* set flag indicating a wrap test
				was run */
			wrap_attempt = TRUE;
			/* store flag in ODM if enter loop
				mode */
			if (da_mode & DA_LOOPMODE_ENTERLM)
				putdavar(da_input.dname, "wrap_attempt",
				    DIAG_SHORT, &wrap_attempt);

			/* set up for wrap testing */
			switch (wiring)
			{
				case NONTELE:
					disp_menu(DETACH_CABLE);
					break;
				case TELE:
					/* add code here to ask if user has 
					 wrap plug (i.e. disp_menu(HAVE_WRAP))
 					 if response is no, then test_display()
					 and return with rc == 0 
					*/

					menu_rc = disp_menu(HAVE_WRAP);
					   
					if (menu_rc == NO)
					{
						test_display();
						wrap_attempt = FALSE;
						putdavar(da_input.dname, 
						    "wrap_attempt",
				    		    DIAG_SHORT, &wrap_attempt);
						no_net = TRUE;
						putdavar(da_input.dname, 
						   	"no_net", DIAG_SHORT,
				    			&no_net);
						return(0);
					}
					disp_menu(ATTACH_WRAP);
					break;
			}
			test_display();
			rc = try_again(); 
			if (rc == 0)
			/* the test passed with tokering cable not 	*/
			/* hooked up to network				*/
			/* But fails with tokenring cable hooked up to	*/
			/* the network.					*/
			{
				disp_menu(NET_BAD);
				rc = BAD_NET;
			}
		}
		return(rc);
	}
			
}
/*************************************************************************/
/*
 * NAME: test_display
 *
 * FUNCTION:  Display "testing" menu.  Which one depends on mode.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *
 */

test_display()
{
	if (da_mode & DA_LOOPMODE_INLM) 
		disp_menu(TESTING_INLM);
	else if (da_mode & DA_LOOPMODE_ENTERLM)
		disp_menu( STANDBY_ADVANCED);
	else if (da_mode & DA_ADVANCED_TRUE)
		disp_menu( STANDBY_ADVANCED);
       	else disp_menu(STANDBY);
}
/***********************************************************************/
/*
 * NAME: try_again
 *
 * FUNCTION:  runs TU 3 to halt device and then runs TU 1 to restart it.
 * 
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: 0		Test unit 1 passed
 *	    -1		The device driver would not re-open.
 *          Other	Test unit 1 failed
 *
 */

int try_again()

{
int rc = 0;		/* results of tests */
int temp_rc;

	/* Run TU 3 */
	tucb.header.tu = 3;
	temp_rc = run_tu();

	/* Close and open device driver as it requires. */
	close(fdes);
	fdes = open(devname, O_RDWR);
	if (fdes == -1)
	{
		DA_SETRC_ERROR(DA_ERROR_OPEN);
		return(fdes);
	}

	/* run TU 1 */
	tucb.header.tu = 1;
	rc = run_tu();

	return(rc);	
}

/*********************************************************************/
