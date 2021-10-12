static char sccsid[] = "@(#)60	1.2.1.3  src/bos/diag/da/iopsl/diopsl.c, daiopsl, bos411, 9428A410j 12/20/93 10:26:00";
/*
 * COMPONENT_NAME: daiopsl
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <fcntl.h>
#include <locale.h>
#include <nl_types.h>
#include <stdio.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <diag/diago.h>
#include <diag/diag_exit.h>
#include <diag/tmdefs.h>
#include <diag/tm_input.h>
#include "diopsl.h"             /* generic defines                */
#include "diop_msg.h"           /* message pointer                */
#include "../../tu/iopsl/misc.h"

#define MALLOC(x)    ((x *) malloc(sizeof(x)))

long  dev_fdes;                 /* dev file pointer               */
struct tm_input da_input;       /* data passed by diag controller */
nl_catd catd;                   /* poiter to catalog file         */
extern nl_catd diag_catopen();
long tmode;                     /* test mode from getdamode()     */
int battery_failed = 0;         /* Battery failed flag for ioctl  */
int key_pos;                    /* Op panel key position          */
int mach_type = HAS_KEY_LOCK;

TUCB *iopsl_tucb;               /* TU control block structure     */

/* ========================================================================= */
/*                         B E G I N    M A I N                              */
/* ========================================================================= */

void main(argc,argv,envp)
int argc;
char **argv;
char **envp;

{
        /* .................................... */
	/* Function declarations                */
        /* .................................... */
        int getdainput();
        void chk_asl_return();
        long getdamode();
        void DIOP_LOOP_TEST_message();
        void do_iop_tests();
        void clean_up();
		int ipl_mod;
		int cuat_mod;

        /* .................................... */
        /*              Initialize              */
        /* .................................... */

	setlocale(LC_ALL,"");
        DA_SETRC_TESTS(DA_TEST_FULL);
        DA_SETRC_STATUS(DA_STATUS_GOOD);
        DA_SETRC_ERROR(DA_ERROR_NONE);
        DA_SETRC_MORE(DA_MORE_NOCONT);
        iopsl_tucb = MALLOC(TUCB);

        /* .....................................*/
        /* init the diag object data manager    */
        /* .....................................*/

        if(init_dgodm() == -1)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* ..................................... */
        /* get the diagnostic data passed by the */
        /* diagnostic controller                 */
        /* ..................................... */

        if(getdainput(&da_input) != 0)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }


        /* .....................................*/
        /* initiate the diagnostic ASL          */
        /* .....................................*/

        if (da_input.console == CONSOLE_TRUE)
        {
                chk_asl_return(diag_asl_init(ASL_INIT_DEFAULT));

                /* .....................................*/
                /* Open the catalog file for the        */
                /* message text                         */
                /* .....................................*/

                catd = diag_catopen(MF_DIOP,0);
        }

        /* .....................................*/
        /* Get the mode (ALL,LOOP,NO_CONSOLE)   */
        /* from the da_input structure passed   */
        /* by the diagnostic controller         */
        /* .....................................*/

        tmode = getdamode(da_input);

        if(tmode == AIX_ERROR)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

        /* .............................................................. */
        /*     Testing is not done on exiting loop mode (D_EXITLM)        */
        /* .............................................................. */

        if (da_input.loopmode == LOOPMODE_EXITLM)
        {
                if (da_input.console == CONSOLE_TRUE)
                {
                        DIOP_LOOP_TEST_message();
                        sleep(2);
                }
                clean_up();
        }

		/*
		 * Find out if this is a system with or without a keylock
		 */
		ipl_mod = get_cpu_model(&cuat_mod);
		ipl_mod = ipl_mod & 0xff;

		if(ipl_mod == KEY_LESS )
		{
			mach_type = NO_KEY_LOCK;
		}
		else
			mach_type = HAS_KEY_LOCK;



        /* ........................................*/
        /* Begin doing the series of test required */
        /* for the type of mode (tmode)            */
        /* ........................................*/

        if(da_input.dmode != DMODE_ELA )
               do_iop_tests();

        if (da_input.console == CONSOLE_TRUE)
                chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        clean_up();

} /* end main() */

/*
 * NAME: void do_iop_tests()
 *
 * FUNCTION: Preform the tests: Dependent on the testing mode (tmode)
 *           If during a test (run_iop_test() ) a error is found
 *           then exit to the diagnostic controller. (thru clean_up() )
 *
 * EXECUTION ENVIRONMENT: called from main() (SEE: iop_main.c)
 *
 *
 * RETURNS: NONE
*/

void do_iop_tests()
{
        void assign_iop_frub();
        void run_tu_test();
        void do_key_lock_tests();
        void clean_up();
        int i, rc = 0 ;

        /* Open the device driver         */

        dev_fdes = open( DIOP_DD , O_RDWR );
        if (dev_fdes == AIX_ERROR)
        {
                DA_SETRC_ERROR(DA_ERROR_OPEN);
                clean_up();
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
                   	    run_tu_test(DEAD_BATTERY_TEST);
                        if (battery_failed == TRUE &&
                            tmode == INTERACTIVE_TEST_MODE)
                        {
                                rc = DEAD_BATTERY_TEST_menu();
                                if (rc != ANSWERED_YES)
                                        assign_iop_frub(BATTERY_TEST);
                        } 
                        run_tu_test(BATTERY_TEST);  
                        if( tmode == INTERACTIVE_TEST_MODE)
                                do_key_lock_tests();

                        clean_up();
                }

                /* ................................ */
                /* Display the default 'Stand by'   */
                /* message                          */
                /* ................................ */

                if(da_input.loopmode == LOOPMODE_NOTLM)
                        IOP_TESTING_message();
                else
                        DIOP_LOOP_TEST_message();

                /* ............................................... */
                /* Test the NVRAM                                  */
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
                                assign_iop_frub(BATTERY_TEST);
                }

                /* ............................................... */
                /* Test Time of Day RAM                            */
                /* ............................................... */

                run_tu_test(TOD_TEST);

                /* ............................................... */
                /* Test Battery level                              */
                /* ............................................... */

                run_tu_test(BATTERY_TEST);

                if(tmode == INTERACTIVE_TEST_MODE)
                {
                        /* ......................................... */
                        /* Tell the user tha '666' and '999' will    */
                        /* be displayed of the LED Readouts on the   */
                        /* chassis control panel.                    */
                        /* ......................................... */

                        LED_TEST_message();
                        run_tu_test( LED_TEST);

                        /* ......................................... */
                        /* Ask the user if the numbers appeared and  */
                        /* if not assign a FRU and exit back to the  */
                        /* diagnostic controller                     */
                        /* ......................................... */

                        rc = LED_TEST_menu();
                        if(rc != ANSWERED_YES)
                                assign_iop_frub(LED_TEST);

                }

                if( tmode == INTERACTIVE_TEST_MODE)
                        do_key_lock_tests();

                run_tu_test(VPD_TEST);

                break;
        case NO_VIDEO_TEST_MODE:

                /* ......................................................... */
                /* Check for concurrent diagnostics, if so only run the      */
                /* BATTERY_TEST.                                             */
                /* ......................................................... */

                if (da_input.exenv == EXENV_CONC)
                {
                        run_tu_test(DEAD_BATTERY_TEST);
                        run_tu_test(BATTERY_TEST);
                        DA_SETRC_TESTS(DA_TEST_FULL);
                        DA_SETRC_STATUS(DA_STATUS_GOOD);
                        clean_up();
                }

                /* ............................................... */
                /* Test the NVRAM                                  */
                /* ............................................... */

                run_tu_test( NVRAM_TEST);

                /* ............................................... */
                /* Test Time of Day RAM                            */
                /* ............................................... */

                run_tu_test( TOD_TEST);

                run_tu_test( BATTERY_TEST);

                run_tu_test(VPD_TEST);

                break;
        default :
                break;
        }
        close(dev_fdes);

} /* end of do_iop_tests */

/*
 * NAME: void run_tu_test()
 *
 * FUNCTION: Preform the TU tests.
 *
 * EXECUTION ENVIRONMENT: Called from do_iop_tests().
 *
 * RETURNS: NONE
*/

void run_tu_test(tn)
        int tn;
{
        void assign_iop_frub();
        extern int exectu();
        int rc = NO_ERROR;
        int battery_status = NO_ERROR;
        iopsl_tucb->tu = tn;
        iopsl_tucb->loop = 1;
        iopsl_tucb->mfg = 0;
	iopsl_tucb->r1 = 0;
	iopsl_tucb->r2 = 0;

        switch(tn)
        {
        case NVRAM_TEST:
        case LED_TEST:
        case TOD_TEST:
        case KEY_LOCK_TEST:
        case VPD_TEST:
        case BATTERY_TEST:
                rc = exectu(dev_fdes,iopsl_tucb);
                if (tn == KEY_LOCK_TEST && rc == key_pos)
                       	rc = NO_ERROR;
                break;
        case DEAD_BATTERY_TEST:
                /* ............................................ */
                /* This test is performed by making a call to   */
                /* the ioctl() function. This is not a TU test. */
                /* ............................................ */
                rc = ioctl(dev_fdes, MIONVCHCK , &battery_status);
                if (rc != NO_ERROR)
                {
                        /* egnore error from ioctl() */
                        rc = NO_ERROR;
			battery_status = 0;
                }
                else
                {
                         switch(battery_status)
                         {
                         case 0:
                                 rc = NO_ERROR;
                                 break;
                         case 1:
                                 rc = NO_ERROR;
                                 battery_failed = TRUE;
                                 break;
                         case 2:
                                 rc = NVRAM_TEST;
								 tn = NVRAM_TEST;
                                 break;
                         default:
                                 rc = NO_ERROR;
                                 break;
                         } /* end battery_status switch */
                }
                break;
        default:
                break;
        }

        if(rc != NO_ERROR)
                assign_iop_frub(tn);

} /* end run_tu_test() */

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
        run_tu_test(KEY_LOCK_TEST);

        key_pos = LOCKED_KEY_POSITION;
        key_position_message(LOCKED_KEY_POSITION);
        run_tu_test(KEY_LOCK_TEST);

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
        run_tu_test(KEY_LOCK_TEST);

} /* do_key_lock_tests() */

/*
 * NAME: long getdamode()
 *
 * FUNCTION:
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS:
*/

long getdamode( da_input)
        struct tm_input da_input;
{
        /* ........................................................... */
        /* the following variables were used to make the function more */
        /* readable and compact                                        */
        /* ........................................................... */

        short a = da_input.advanced;
        short c = da_input.console;
        short s = da_input.system;
        short l = da_input.loopmode;
        short e = da_input.exenv;

        int mode   = INVALID_TM_INPUT;

        if(c  == CONSOLE_TRUE  && ( s != SYSTEM_TRUE || l != LOOPMODE_INLM ) )
                mode = INTERACTIVE_TEST_MODE ;
        if( c == CONSOLE_FALSE )
                mode = NO_VIDEO_TEST_MODE;
        if( c == CONSOLE_TRUE && ( s == SYSTEM_TRUE ||  l == LOOPMODE_INLM))
                mode = TITLES_ONLY_TEST_MODE ;
            return(mode);

} /* end getdamode() */

/*
 * NAME:  void chk_asl_return(asl_return)
 *
 * FUNCTION: Check the value returned from the diag_asl_display() functions
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 * RETURNS: NONE
*/

void chk_asl_return(asl_return)
        long asl_return;
{
        void clean_up();

        switch(asl_return)
        {
        case DIAG_ASL_OK:
                break;
        case DIAG_ASL_ERR_SCREEN_SIZE:
        case DIAG_ASL_FAIL:
        case DIAG_ASL_ERR_NO_TERM:
        case DIAG_ASL_ERR_NO_SUCH_TERM:
        case DIAG_ASL_ERR_INITSCR:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
                break;
        case DIAG_ASL_EXIT:
                DA_SETRC_USER(DA_USER_EXIT);
                clean_up();
                break;
        case DIAG_ASL_CANCEL:
                DA_SETRC_USER(DA_USER_QUIT);
                clean_up();
                break;
        default:
                break;

        }

} /* chk_asl_return() */

/*
 * NAME: void assign_iop_frub(tn)
 *
 * FUNCTION: Assign a fru bucket (defined in iop_frus.h) to
 *           be passed to the diagnostic controller for dsiplay
 *
 * EXECUTION ENVIRONMENT: Called by do_iop_tu();
 *
 * RETURNS: NONE
*/

void assign_iop_frub(tn)
int tn;
{
        void chk_asl_return();
        void clean_up();

        int save_sn;
        int fru_index;


        if (da_input.console == CONSOLE_TRUE)
                chk_asl_return(diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL));

        /* ------------------------------------------------------------ */
        /* Assigns the appropriate code to the FRU Bucket               */
        /* ------------------------------------------------------------ */

        if ( tn >= 10 )
        {
                fru_index = (tn / 10) - 1;

				/* if this system has no keylock, call out mode switch */
                if( (fru_index == 3) && (mach_type == NO_KEY_LOCK) )
                    fru_index = 6;

                save_sn = frub[fru_index].sn;
                strcpy(frub[fru_index].dname,da_input.dname);
                insert_frub(&da_input,&frub[fru_index]);
                frub[fru_index].sn = save_sn;
                addfrub( &frub[fru_index] );
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_TESTS(DA_TEST_FULL);
        }
        else
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                DA_SETRC_TESTS(DA_TEST_FULL);
        }

        clean_up();

} /* end assign_iop_frub() */

/*
 * NAME: void clean_up()
 *
 * FUNCTION: Close all open files and exit back to the dianostic controller.
 *
 * EXECUTION ENVIRONMENT: Called from do_iop_tests().
 *
 * RETURNS: NONE
*/

void clean_up()
{
        int rc;

        term_dgodm();
        if(catd != CATD_ERR)
                catclose(catd);
        if(dev_fdes > 0 )
                close(dev_fdes);
        if (da_input.console == CONSOLE_TRUE)
                diag_asl_quit(NULL);
        DA_EXIT();

} /* end clean_up() */

