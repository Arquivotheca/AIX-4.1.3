static char sccsid[] = "@(#)66	1.7.1.10  src/bos/diag/da/iop/doioptest.c, daiop, bos41J, 9513A_all 3/9/95 09:25:21";
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: do_iop_tests
 *		do_key_lock_tests
 *		
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
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/devinfo.h>

#include "iop.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/modid.h"

#include "diag/diago.h"         /* ASL functions                        */
#include "diag/diag_exit.h"     /* exit codes                           */
#include "ioptuname.h"          /* iop tu define names                  */

extern long tmode;
extern long dev_fdes;
extern int battery_failed;
extern struct tm_input da_input ;
int key_pos;
int epow_test = 0;
int dualp_test = 0;


void do_key_lock_tests();

/*
 * NAME: void do_iop_tests()
 *
 * FUNCTION: Preform the tests: Dependent on the testing mode (tmode)
 *           If during a test (run_iop_test() ) a error is found
 *           then exit to the diagnostic controller. 
 *
 * EXECUTION ENVIRONMENT: called from main() (SEE: iop_main.c)
 *
 *
 * RETURNS: NONE
*/


void do_iop_tests()
{
        int i, rc = 0 ;
	unsigned int model;
	unsigned int aix_modcode;
	char ddname[32];

	/* Figure out the correct dd name to open. */
	/* For ioplanar0 , ddname = /dev/bus0 */
	/* For ioplanar1 , ddname = /dev/bus1 */
	if (da_input.dname[8] == '0')
	{
		dualp_test = 0;
		strcpy(ddname,"/dev/bus0");
	}
	else
	{
		dualp_test = 1;
		strcpy(ddname,"/dev/bus1");
	}

	model = get_cpu_model(&aix_modcode);
	if (IsPowerPC_SMP(model))
		strcpy(ddname,"/dev/nvram");

        /* Open the device driver */
        dev_fdes = open( ddname , O_RDWR );
        if (dev_fdes == AIX_ERROR)
        {
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                clean_up();
        }

	/* Check the machine model type. */
	if ((aix_modcode >= 0x36 && aix_modcode <= 0x39) ||
	    (aix_modcode >= 0x60 && aix_modcode <= 0x64))
	{
		/* Test the EPOW connector. */
		epow_test = 1;
	}


        switch(tmode)
        {
        case INTERACTIVE_TEST_MODE:
        case TITLES_ONLY_TEST_MODE:

                /* ......................................................... */
                /* Check for concurrent diagnostics, if so only run the      */
                /* battery tests.                                            */
                /* ......................................................... */

                if (da_input.exenv == EXENV_CONC)
                {
                        IOP_TESTING_message();
			if (dualp_test) 
			{
				/* Only test run on the 2nd planar. */
				run_tu_test(EC_REG_TEST2);
				/* Return to main */
				return;
			}

                        run_tu_test(DEAD_BATTERY_TEST);
                        if (battery_failed == TRUE &&
                            tmode == INTERACTIVE_TEST_MODE)
                        {
                                rc = DEAD_BATTERY_TEST_menu();
                                if (rc != ANSWERED_YES)
                                        assign_iop_frub(BATTERY_TEST_FAILURE,
                                                        BATTERY_TEST);
                        }
                        run_tu_test(BATTERY_TEST);
                        if( tmode == INTERACTIVE_TEST_MODE)
                                do_key_lock_tests();

                	/* ............................................... */
			/* If stillwell or one of the gomez clan, check    */
			/* the EPOW connector. See do_iop_tu.c for more.   */
                	/* ............................................... */

                	run_tu_test( EC_REG_TEST);

			/* run the batter backup tests */
			run_tu_test(BBU_TEST);

			/* run the fan_check if this is multiprocessor sys*/
			if (IsPowerPC_SMP(model))
		      		run_tu_test(FAN_CHECK);

			/* ............................................... */
			/* If Pegasus, Test Expansion Cabinet Power Status */
			/* ............................................... */
			if (IsPowerPC_SMP(model))
				run_tu_test(GET_CAB_PS_TABLE);

			/* Return to main */
			return;
                }

                /* ................................ */
                /* Display the default 'Stand by'   */
                /* message                          */
                /* ................................ */

                if(da_input.loopmode == LOOPMODE_NOTLM)
                        IOP_TESTING_message();
                else
                        DIOP_LOOP_TEST_message();

		if (dualp_test) 
		{
			/* This is the only test run on the 2nd planar. */
			run_tu_test(EC_REG_TEST2);
			/* Return to main */
			return;
		}

                /* ............................................... */
                /* Test the NVRAM at location 0x00300 thru 0x00303 */
                /* ............................................... */

                run_tu_test(NVRAM_TEST);

                /* ............................................... */
                /* Check for very low or missing battery.  This    */
                /* code has been added because the BATTERY_TEST    */
                /* cant detect a dead, very low or missing battery.*/
                /* ............................................... */

                run_tu_test(DEAD_BATTERY_TEST);
                if (battery_failed == TRUE && tmode == INTERACTIVE_TEST_MODE)
                {
                        rc = DEAD_BATTERY_TEST_menu();
                        if (rc != ANSWERED_YES)
                                assign_iop_frub(BATTERY_TEST_FAILURE,
                                                BATTERY_TEST);
                }

                /* ............................................... */
                /* Test Time of Day RAM                            */
                /* ............................................... */

                run_tu_test(TIME_OF_DAY_TEST1);

                /* ............................................... */
                /* Test Battery level                              */
                /* ............................................... */

                run_tu_test(BATTERY_TEST);

                /* ............................................... */
                /* Test the TOD to check for a not running state   */
                /* If it is not running the try to reset the clock */
                /* check again to test for POR state               */
                /* (See run_tu_test(x) ... do_iop_tu.c             */
                /* ............................................... */

                run_tu_test(TIME_OF_DAY_TEST2);

                /* ............................................... */
		/* If stillwell or one of the gomez clan, check    */
		/* the EPOW connector. See do_iop_tu.c for more.   */
                /* ............................................... */

                run_tu_test( EC_REG_TEST);

                if(tmode == INTERACTIVE_TEST_MODE && !(IsPowerPC_SMP(model)))
                {
                        /* ......................................... */
                        /* Tell the user tha '666' and '999' will    */
                        /* be displayed of the LED Readouts on the   */
                        /* chassis control panel.                    */
                        /* ......................................... */

                        LED_TEST_message();
                        run_tu_test( LED_TEST);
                        sleep(2);
                        run_tu_test( LED_TEST);
                        sleep(2);

                        /* ......................................... */
                        /* Ask the user if the numbers appeared and  */
                        /* if not assign a FRU and exit back to the  */
                        /* diagnostic controller                     */
                        /* ......................................... */


                        run_tu_test(LED_RESET);

                        rc = LED_TEST_menu();
                        if(rc != ANSWERED_YES)
                                assign_iop_frub(LED_TEST_FAILURE,LED_TEST);

                        /* ......................................... */
                        /* Reset the LEDs back to what they were     */
                        /* ......................................... */

                
		}
                if ((tmode == INTERACTIVE_TEST_MODE) && (IsPowerPC_SMP(model)))
                {
                        /* ......................................... */
                        /* Tell the user that 'U*' and 'VAS' will    */
                        /* be displayed of the LCD Readouts on the   */
                        /* chassis control panel.                    */
                        /* ......................................... */

                        LCD_TEST_message();
                        run_tu_test( LCD_TEST);
                        sleep(2);
                        run_tu_test( LCD_TEST);
                        sleep(2);

                        /* ......................................... */
                        /* Reset the LCDs back to what they were     */
                        /* ......................................... */

                        run_tu_test(LCD_RESET);

                        /* ......................................... */
                        /* Ask the user if the numbers appeared and  */
                        /* if not assign a FRU and exit back to the  */
                        /* diagnostic controller                     */
                        /* ......................................... */

                        rc = LCD_TEST_menu();
                        if(rc != ANSWERED_YES)
                                assign_iop_frub(LCD_TEST_FAILURE,LCD_TEST);

                }

                /* ............................................... */
                /* Read the power status word and return the first */
                /*  byte (0x4000e4)                                */
                /* ............................................... */

                run_tu_test( POWER_STATUS_1);

                /* ............................................... */
                /* Read the power status word and return the 2nd   */
                /*  byte (0x4000e5)                                */
                /* ............................................... */

                run_tu_test( POWER_STATUS_2);

		/* run the batter backup tests */
		run_tu_test(BBU_TEST);

		/* run the fan_check if this is multiprocessor sys*/
		if (IsPowerPC_SMP(model))
		      run_tu_test(FAN_CHECK);

                /* ............................................... */
                /* If Pegasus, Test Expansion Cabinet Power Status */
                /* ............................................... */
		if (IsPowerPC_SMP(model))
			run_tu_test(GET_CAB_PS_TABLE);

                if( tmode == INTERACTIVE_TEST_MODE)
                        do_key_lock_tests(); 

		/* some of these new TUs are not applicable to Pegasus model */
		/* this is managed at TU level */
		run_new_tu(dev_fdes);

                break;
        case NO_VIDEO_TEST_MODE:

		if (dualp_test) 
		{
			/* This is the only test run on the 2nd planar. */
			run_tu_test(EC_REG_TEST2);
			/* Return to main */
			return;
		}

		/* run the batter backup tests */
		run_tu_test(BBU_TEST);

		/* run the fan_check if this is multiprocessor sys*/
		if (IsPowerPC_SMP(model))
			run_tu_test(FAN_CHECK);

                /* ............................................... */
                /* If Pegasus, Test Expansion Cabinet Power Status */
                /* ............................................... */
		if (IsPowerPC_SMP(model))
			run_tu_test(GET_CAB_PS_TABLE);

                /* ......................................................... */
                /* Check for concurrent diagnostics, if so only run the      */
                /* BATTERY_TEST.                                             */
                /* ......................................................... */

                if (da_input.exenv == EXENV_CONC)
                {

                        run_tu_test(DEAD_BATTERY_TEST);
                        run_tu_test(BATTERY_TEST);
                	/* ............................................... */
			/* If stillwell or one of the gomez clan, check    */
			/* the EPOW connector. See do_iop_tu.c for more.   */
                	/* ............................................... */
                	run_tu_test( EC_REG_TEST);
			/* Return to main */
			return;
                }

                /* ............................................... */
                /* Test the NVRAM at location 0x00300 thru 0x00303 */
                /* ............................................... */

                run_tu_test( NVRAM_TEST);

                /* ............................................... */
                /* Test Time of Day RAM                            */
                /* ............................................... */

                run_tu_test( TIME_OF_DAY_TEST1);

                /* ............................................... */
                /* Test the TOD to check for a not running state   */
                /* If it is not running the try to reset the clock */
                /* check again to test for POR state               */
                /* (See run_tu_test(x) ... do_iop_tu.c             */
                /* ............................................... */

                run_tu_test( TIME_OF_DAY_TEST2);

                /* ............................................... */
		/* If stillwell or one of the gomez clan, check    */
		/* the EPOW connector. See do_iop_tu.c for more.   */
                /* ............................................... */

                run_tu_test(EC_REG_TEST);

                /* ............................................... */
                /* Read the power status word and return the first */
                /*  byte (0x4000e4)                                */
                /* ............................................... */

                run_tu_test( POWER_STATUS_1);

                /* ............................................... */
                /* Read the power status word and return the 2nd   */
                /*  byte (0x4000e5)                                */
                /* ............................................... */

                run_tu_test( POWER_STATUS_2);

                /* ............................................... */
                /* Test Battery level                              */
                /* ............................................... */

                run_tu_test( BATTERY_TEST);

		/* some of these new TUs are not applicable to Pegasus model */
		/* this is managed at TU level */
		run_new_tu(dev_fdes);

                break;
        default :
                break;
        }
        close(dev_fdes);
} /* end of do_iop_tests */



/*
 * NAME: void do_key_lock_tests()
 *
 * FUNCTION: Preform the key lock tests.  Ask the user to put the key
 *           in the normal, secure, and locked positions to test the key
 *           lock.
 *
 * EXECUTION ENVIRONMENT: Called from do_iop_tests().
 *
 *
 * RETURNS: NONE
*/

void do_key_lock_tests()
{

        /* ............................................... */
        /* Read the power status word and return the 4th   */
        /*  byte (0x4000e7) Key position data              */
        /* Test for all three positions                    */
        /* ............................................... */

        key_lock_test_start_message();

        if (da_input.exenv == EXENV_CONC)
        {
                key_pos = SERVICE_KEY_POSITION;
                key_position_message(SERVICE_KEY_POSITION);
        }
        else
        {
                key_pos = NORMAL_KEY_POSITION;
                key_position_message(NORMAL_KEY_POSITION);
        }
        run_tu_test( POWER_STATUS_4);

        key_pos = LOCKED_KEY_POSITION;
        key_position_message(LOCKED_KEY_POSITION);
        run_tu_test( POWER_STATUS_4);

        if (da_input.exenv == EXENV_CONC)
        {
                key_pos = NORMAL_KEY_POSITION;
                key_position_message(NORMAL_KEY_POSITION);
        }
        else
        {
                key_pos = SERVICE_KEY_POSITION;
                key_position_message(SERVICE_KEY_POSITION);
        }
        run_tu_test( POWER_STATUS_4);

}
