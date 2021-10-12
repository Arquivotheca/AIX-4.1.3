static char sccsid[] = "@(#)60  1.11  src/bos/diag/da/artic/dartic.c, daartic, bos41J, 9521A_all 5/19/95 18:06:36";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: check_asl_stat
 *              check_driver_and_ucode
 *              clean_malloc
 *              clean_up
 *              clear_variables
 *              configure_diag_dd
 *              restore_configuration
 *              da_display
 *              error_log_analysis
 *              general_initialize
 *              generic_report_frub
 *              interactive_tests
 *              intr_handler
 *              main
 *              configure_lpp_device
 *              unconfigure_lpp_device
 *              non_interactive_tests
 *              open_device
 *              performing_test
 *              poll_kb_for_quit
 *              report_frub
 *              set_adapter_name
 *              set_timer
 *              test_display
 *              test_tu
 *              timeout
 *              unconfigure_diag_dd
 *              verify_rc
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
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
#include <sys/errids.h>
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/modid.h"
#include "diag/diago.h"
#include "diag/bit_def.h"
#include "dartic.h"
#include "dartic_gbl.h"
#include "dartic_msg.h"
#include "dmultp_msg.h"
#include "artictst.h"
#include <locale.h>

int     performing_test();
int     t1e1j1_interactive_tests();
int     non_interactive_tests();
int     interactive_tests();
int     test_tu(int, int, int);
int     verify_rc(int*,int);
int     open_device();
int     poll_kb_for_quit();
int     Having_wrap_plug();
int     Having_cable_tested();
int     check_asl_stat(int);
void    intr_handler(int);
void    error_log_analysis ();
int     test_display();
void    da_display();
void    report_frub();
void    generic_report_frub();
void    clean_up();
void    timeout(int);
void    clear_variables();
int     unplug_cable_and_put_wrap ();
void    check_driver_and_ucode ();
void    general_initialize ();
void    clean_malloc ();
void    set_timer();
int     restore_configuration();
int     unconfigure_diag_dd();
int     configure_diag_dd();
int     unconfigure_lpp_device ();
int     configure_lpp_device ();
int     Having_wrap_plug_mpqp_78_pins();
int     set_adapter_name();

short   wrap_plug;
short   wrap_plug_78_pin;
short   cable_tested;
short   wrap_plug_232 = FALSE;
short   wrap_plug_422 = FALSE;
short   wrap_plug_V35 = FALSE;
extern  nl_catd diag_catopen();
extern  getdainput();           /* gets input from ODM                  */
extern  long getdamode();       /* transforms input into a mode         */
extern  int errno;              /* reason device driver won't open      */
short   rack_model;

volatile int    alarm_timeout=FALSE;
struct  sigaction  invec;       /* interrupt handler structure  */

short   odm_flg = 0;            /* return code from init_dgodm() */
short   asl_flg = NOT_GOOD;     /* return code from diag_asl_init() */

TUTYPE  artic_tucb;
int     diag_dd_is_defined = FALSE;
int     diag_dd_status= NOT_CONFIG;
int     adapter_state= NOT_CONFIG;
int     adapter_is_configured=FALSE;
int     other_dd_state= NOT_CONFIG;
char    otherdd_location[16];

int     parent_state = -1;      /* config state of parent (bus0 or bus1) */
int     child_state[8];
int     grandchild_state[16];
char    *child_location[8];
char    *grandchild_location[16];
char    *other_child[8];         /* name of lpp device driver's  child   */
char    *other_grandchild[16];
char    adapter_name[64];

struct  tm_input da_input;      /* initialized by getdainput            */
int     filedes=NOT_GOOD;       /* file descriptor from device driver   */
nl_catd catd;                   /* pointer to message catalog           */
int     card_type=99999;
int     EIB_type;
int     unconfigure_lpp = FALSE;
int     isabus = FALSE;
char    *p_tmp;
char    *otherdd_name;
char    *lpp_unconfigure_method;
char    *diag_unconfigure_method;
char    *lpp_configure_method;
char    *diag_configure_method;
char    *diag_define_method;
char    *diag_undefine_method;
char    *lpp_unconfigure_device_name;
char    *diag_configure_device_name;
char    *other_dd_configure_method;
char    *other_grandchild_configure_method[16];
char    *other_dd_unconfigure_method;
char    *other_port_configure_method;
char    *other_port_unconfigure_method;
int     diag_device_is_defined = FALSE;
char    *new_out, *new_err;
char    test_device[256];
char    option[256];
int     diskette_mode;
int     diskette_based;
struct  stat stat_buf;
int     which_type= COMMON_GROUP_TEST;



/*......................................................................*/
/*
 * NAME: main
 *
 * FUNCTION: The main routine for the T1J1E1 Diagnostics Application
 *
 * EXECUTION ENVIRONMENT: Invoked by the diagnostic controller.
 *
 * RETURNS:                     DA_STATUS_GOOD
 *                                      No problems were found.
 *                              DA_STATUS_BAD
 *                                      A FRU Bucket or a Menu Goal was
 *                                      reported.
 *                              DA_USER_EXIT
 *                                      The Exit Key was entered by the user.
 *                              DA_USER_QUIT
 *                                      The Quit Key was entered by the user.
 *                              DA_ERROR_NONE
 *                                      No errors were encountered performing a
 *                                      normal operation such as displaying a
 *                                      menu, accessing the object repository,
 *                                      allocating memory, etc.
 *                              DA_ERROR_OPEN
 *                                      Could not open the device.
 *                              DA_ERROR_OTHER
 *                                      An error was encountered performing
 *                                      a normal operation.
 *                              DA_TEST_FULL
 *                                      The full-tests were executed.
 *                              DA_MORE_NOCONT
 *                                      The isolation process is complete.
 *                              DA_MORE_CONT
 *                                      The path to the device should be
 *                                      tested.
 *
 ..........................................................................*/


main (argc, argv)
int argc;
char **argv;
{
        int     rc=0;                   /* return code                  */
        int     rc1=0;


        /* Initialize variable  */
        general_initialize ();
        da_display ();

        /* executing test if running problem determination      */
        /* or repair mode                                       */
        if ( da_input.dmode & (DMODE_PD | DMODE_REPAIR) )

        {
                unconfigure_lpp_device ();

                /* opening the diagnostics device for testing   */
                (void) signal (SIGINT, SIG_IGN);
                rc = open_device ();
                (void) signal (SIGINT, intr_handler);
                EIB_type= gettype (filedes);
                set_adapter_name ();

                /* check if diskette package is running */
                diskette_mode = ipl_mode(&diskette_based);
                if (diskette_based == DIAG_FALSE)
                {
                        check_driver_and_ucode ();
                }

                if ( rc == NO_ERROR)
                {
                        if  ( da_input.console == CONSOLE_TRUE)
                                da_display ();
                        rc = performing_test ();
                }

        }

        /* if running problem determination, or error log analyis       */
        /* running error log analysis                                   */

        if  (rc == NO_ERROR && (( da_input.dmode ==  DMODE_PD)
                || (da_input.dmode == DMODE_ELA )) &&
                (da_input.loopmode & (LOOPMODE_NOTLM | LOOPMODE_ENTERLM)) )
        {
                  error_log_analysis ();
        }
        clean_up ();

} /* main */


/*--------------------------------------------------------------------------
|       NAME: da_display
|
|       FUNCTION: Update display for appropriate mode
|
|       EXECUTION ENVIRONMENT:
|
|       NOTES:  Other local routines called --
|               test_display()
|
|       RETURNS:
|               None
----------------------------------------------------------------------------*/

void da_display ()
{
        if (da_input.console)
        {
                if (( da_input.loopmode == LOOPMODE_INLM) ||
                    ( da_input.loopmode == LOOPMODE_EXITLM))
                        test_display ( TESTING_LOOPMODE);
                else if ( da_input.advanced == ADVANCED_TRUE)
                        test_display ( TESTING_ADVANCED_MODE);
                else
                        test_display (TESTING_REGULAR_GENERIC) ;
        }
}


/*--------------------------------------------------------------------------
|       NAME: Performing_test
|
|       FUNCTION: Invokes appropriate test units
|
|       EXECUTION ENVIRONMENT:
|
|       NOTES:  Other local routines called --
|               non_interactive_tests()
|               t1e1j1_interactive_tests()
|
|       RETURNS:
|               same return code of exectu()
----------------------------------------------------------------------------*/
int performing_test ()
{
        int     rc =NO_ERROR;

        rc =  non_interactive_tests ();

        /* system true means non-interactive test       */

        if (( rc == NO_ERROR) && ( da_input.advanced == ADVANCED_TRUE)
                        && (!(da_input.loopmode & (LOOPMODE_INLM | LOOPMODE_EXITLM)) )
                        && ( da_input.console == CONSOLE_TRUE )
                        && ( da_input.system == SYSTEM_FALSE ))
                rc = interactive_tests();
        return (rc);
}

/*--------------------------------------------------------------------------
|       NAME: interactive_tests()
|
|       FUNCTION: Run interactive tests
|
|       EXECUTION ENVIRONMENT:
|
|       NOTES:  Other local routines called --
|               Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
|
|       RETURNS:
|               NO_ERROR - test passed
|               otherwise -test failed
----------------------------------------------------------------------------*/
interactive_tests()
{
        switch (card_type)
        {
                case MPQP_4PORT:
                        mpqp_advanced_tests ();
                        break;
                case PM:
                        portmaster_advanced_tests();
                        break;
                case MP_2:
                        multiport_2_advanced_tests();
                        break;
                case X_25:
                        x25_advanced_tests();
                        break;
                case T1E1J1:
                        t1e1j1_interactive_tests();
                        break;
        }
}

/*--------------------------------------------------------------------------
|       NAME: non_interactive_test
|
|       FUNCTION: Run non interactive test ( Not required wrap )
|
|       EXECUTION ENVIRONMENT:
|
|       NOTES:  Other local routines called --
|
|       RETURNS:
|               same return code of exectu()
----------------------------------------------------------------------------*/

int non_interactive_tests ()
{
        struct  tu_fru_pair *start;
        struct mpqp_tu_fru_pair *mpqp_start;
        struct x25_tu_fru_pair *x25_start;
        int     rc;

        artic_tucb.header.mfg = 0;
        artic_tucb.header.loop = 1;

        start = tus_test;

        which_type = COMMON_GROUP_TEST;
        for (; start->tu != -1; ++start) {
                   rc = test_tu(start->tu,start->fru, COMMON_GROUP_TEST);
                if ( rc != NO_ERROR)
                        break;
                 if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
                        return(QUIT);
        }
        if (rc != 0) return (rc);
        switch (card_type)
        {
                        case MPQP_4PORT:
                        case PM:
                                which_type = MPQP_4PORT;
                                mpqp_start = mpqp_pm_test;
                                for (; mpqp_start->tu != -1; ++mpqp_start)
                                {
                                   rc = test_tu(mpqp_start->tu,mpqp_start->fru,
                                        MPQP_4PORT );
                                   if ( rc != NO_ERROR)
                                        break;
                                   if (da_input.console == TRUE &&
                                        poll_kb_for_quit()==QUIT)
                                        return(QUIT);
                                }
                        break;

                        case X_25:
                        case MP_2:
                                which_type = X_25;
                                x25_start = x25_mp_2_test;
                                for (; x25_start->tu != -1; ++x25_start)
                                {
                                   rc = test_tu(x25_start->tu,x25_start->fru,
                                        X_25);
                                   if ( rc != NO_ERROR)
                                        break;
                                   if (da_input.console == TRUE &&
                                        poll_kb_for_quit()==QUIT)
                                        return(QUIT);
                                }
                        break;
        }

        return(rc);
}

/*--------------------------------------------------------------------------
|       NAME: test_tu
|
|       FUNCTION: run test unit and report FRU specified by fru_index
|           for failed test; and if fru_index equals NO_REPORT_FRU (-1) then
|           no FRU report is needed.
|
|       EXECUTION ENVIRONMENT:
|
|       NOTES:  Other local routines called --
|         report_frub()
|
|       RETURNS:
|               same return code of exectu()
----------------------------------------------------------------------------*/
int test_tu (int test_unit, int fru_index, int which_test)
{
        int rc;

        artic_tucb.header.tu = test_unit;

        set_timer ();
        rc = exectu(filedes, &artic_tucb);

        if (alarm_timeout )     /* some how test hangs  */
        {
                report_frub (fru_index);
                DA_SETRC_STATUS (DA_STATUS_BAD);
                return (NOT_GOOD);
        }

        if (rc  != NO_ERROR)    /* Report fru   */
        {
                if (which_test == COMMON_GROUP_TEST)
                {
                        if (verify_rc(tu_rctbl[fru_index],rc) == TRUE )
                        {
                                if (card_type == PM)
                                {
                                     if ((rc == 0x11) || (rc == 0x12))
                                                fru_index = 9;
                                     else
                                        fru_index = (core_pm_frus [fru_index]);
                                 }

                                report_frub (fru_index);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
                else if (which_test == MPQP_4PORT)
                {
                        if (verify_rc(mpqp_pm_rctbl[fru_index],rc) == TRUE )
                        {
                                report_frub (fru_index);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
                else if (which_test == X_25)
                {
                        if (verify_rc(x25_mp_2_rctbl[fru_index],rc) == TRUE )
                        {
                                report_frub (fru_index);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
                else if (which_test == MPQP_4PORT_ADVANCED)
                {
                        if (verify_rc(mpqp_advanced_rctbl[fru_index],rc)
                                                        == TRUE )
                        {
                                report_frub (FRU_MPQP_INTERNAL);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
                else if (which_test == PORTMASTER_ADVANCED)
                {
                        if (verify_rc(portmaster_advanced_rctbl[fru_index],rc)
                                                        == TRUE )
                        {
                                fru_index = EIB_type - 101;
                                report_frub (fru_index);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
                else if (which_test == MP_2)
                {
                        if (verify_rc(mp2_advanced_rctbl[fru_index],rc)
                                                        == TRUE )
                        {
                                if ((rc == 0x701) || (rc == 0x702))
                                        fru_index = 5;
                                else
                                        fru_index = EIB_type - 1;
                                report_frub (fru_index);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                        }
                }
        }
        return(rc);
}

/*--------------------------------------------------------------------------
| NAME: verify_rc
|
| FUNCTION: verify the return code from exectu() of the specified
|           test unit is a valid return code
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       TRUE -- is in valid return code range
|       FALSE - is not a valid return code
----------------------------------------------------------------------------*/
verify_rc(int *rc_ptr,int rc)
{
        rc &= 0x0000FFFF;

        for (; *rc_ptr; rc_ptr++)
        {
                if (*rc_ptr == rc )
                        return(TRUE);
        }

        DA_SETRC_ERROR (DA_ERROR_OTHER);
        return(FALSE);
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
|       errno after open()
----------------------------------------------------------------------------*/
int open_device ()
{
        int     open_error;
        char    devname[32];



        strcat (devname,diag_configure_device_name);
        sprintf(devname,"/dev/%s",diag_configure_device_name);

        /* this is the workaround where config method put an extra blank  */
        /* after diagricx  then elimiate this extrac blank. Otherwise     */
        /* it will fail open                                            */

        if (devname[12] == ' ')
                devname[12]='\0';
        else
                devname[13]='\0';
        errno =0;
        set_timer ();
        filedes = open(devname, O_RDWR | O_NDELAY);

        open_error = errno;

        if (alarm_timeout || filedes == NOT_GOOD)
        {
                switch (open_error)
                {
                case EIO:
                case ENXIO:
                case ENODEV:
                        report_frub(FRU_OPEN);
                        DA_SETRC_TESTS(DA_TEST_FULL);
                        DA_SETRC_STATUS (DA_STATUS_BAD);
                        clean_up ();
                        break;
                case EBUSY:
                        DA_SETRC_TESTS(DA_TEST_NOTEST);
                        DA_SETRC_ERROR(DA_ERROR_OPEN);
                        clean_up ();
                        break;
                    default:
                        DA_SETRC_TESTS(DA_TEST_NOTEST);
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up ();
                        break;
                } /* end switch */
        }

        return (NO_ERROR);
} /* open_device */

/*......................................................*/
/*     Procedure description :  Error logging analysis  */
/*......................................................*/

void error_log_analysis ()
{
        /* this is for future feature   */
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
| NAME: test_display
|
| FUNCTION: This function will display three different message. Advanced,
|           loopmode and regular testing menu.
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None
----------------------------------------------------------------------------*/

int test_display (menu_type)
int     menu_type;
{
        char    *str;
        nl_catd cudv_cat;
        int     rc;
        struct CuDv     *cudv_ptr;
        char    criteria[64];
        struct listinfo v_info;
        int     menu_number;

        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        if (da_input.advanced)
                                menu_number = 0x855002;
                        else
                                menu_number = 0x855001;
                        break;
                case MP_2:
                case X_25:
                        if (da_input.advanced)
                                menu_number = 0x849002;
                        else
                                menu_number = 0x849001;
                        break;
                case T1E1J1:
                        if (da_input.advanced)
                                menu_number = 0x851002;
                        else
                                menu_number = 0x851001;
                        break;
                default:
                        if (strstr (da_input.dname, "mpx"))
                                menu_number = 0x849000;
                        else
                                menu_number = 0x855000;
                        break;
        }

        switch (menu_type)
        {
                case TESTING_LOOPMODE:
                                diag_msg_nw ( menu_number, catd,
                                1, TESTING_LOOPMODE_GENERIC,da_input.dname,
                                adapter_name,da_input.dnameloc, da_input.lcount,
                                da_input.lerrors);
                        break;
                case TESTING_ADVANCED_MODE:
                        diag_msg_nw ( menu_number, catd, 1,
                                TESTING_ADVANCED_GENERIC,da_input.dname,
                                adapter_name, da_input.dnameloc);
                        break;

                case TESTING_REGULAR_GENERIC:
                        diag_msg_nw ( menu_number, catd, 1,
                                TESTING_REGULAR_GENERIC,da_input.dname,
                                adapter_name, da_input.dnameloc);
                        break;

                case PLUG_T1_CABLE:
                        /* ask the user to plug the cable in    */
                        rc = diag_display(0x851005,
                                catd, t1_cable_plug_in,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_t1_cable_plug_in);
                        sprintf(msgstr, asi_t1_cable_plug_in[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_t1_cable_plug_in[0].text);
                        asi_t1_cable_plug_in[0].text = msgstr;
                        sprintf(msgstr1, asi_t1_cable_plug_in[1].text,
                                da_input.dnameloc);
                        free (asi_t1_cable_plug_in[1].text);
                        asi_t1_cable_plug_in[1].text = msgstr1;

                        rc = diag_display(0x851005, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_t1_cable_plug_in);

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

                case PLUG_E1_CABLE:
                        /* ask the user to plug the cable in    */
                        rc = diag_display(0x851005,
                                catd, e1_cable_plug_in,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_e1_cable_plug_in);
                        sprintf(msgstr, asi_e1_cable_plug_in[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_e1_cable_plug_in[0].text);
                        asi_e1_cable_plug_in[0].text = msgstr;
                        sprintf(msgstr1, asi_e1_cable_plug_in[1].text,
                                da_input.dnameloc);
                        free (asi_e1_cable_plug_in[1].text);
                        asi_e1_cable_plug_in[1].text = msgstr1;

                        rc = diag_display(0x851005, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_e1_cable_plug_in);

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

                case UNPLUG_WRAP_AND_CABLE:
                        /* ask the user to unplug cable and put wrap    */
                        /* plug in the adapter for further testing      */
                        rc = diag_display(0x851006,
                                catd, unplug_wrap_and_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_unplug_wrap_and_cable);
                        sprintf(msgstr, asi_unplug_wrap_and_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_unplug_wrap_and_cable[0].text);
                        asi_unplug_wrap_and_cable[0].text = msgstr;
                        sprintf(msgstr1, asi_unplug_wrap_and_cable[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_unplug_wrap_and_cable[1].text);
                        asi_unplug_wrap_and_cable[1].text = msgstr1;

                        rc = diag_display(0x851006, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_wrap_and_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                clear_variables();
                                clean_up();
                        }



                        /* setting cable_tested variable in database to true */
                        wrap_plug= TRUE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;
                case UNPLUG_WRAP:
                        /* ask the user to unplug wrap                  */
                        /* and put the cable back into the network      */
                        rc = diag_display(0x851010,
                                catd, unplug_wrap,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_unplug_wrap);
                        sprintf(msgstr, asi_unplug_wrap[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_unplug_wrap[0].text);
                        asi_unplug_wrap[0].text = msgstr;
                        sprintf(msgstr1, asi_unplug_wrap[1].text,
                                da_input.dnameloc, da_input.dnameloc);
                        free (asi_unplug_wrap[1].text);
                        asi_unplug_wrap[1].text = msgstr1;

                        rc = diag_display(0x851010, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_wrap);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                clear_variables();
                                clean_up();
                        }

                        /* setting wrap_plug variable in database to false */
                        wrap_plug= FALSE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
                        break;

                case UNPLUG_CABLE:
                        /* ask the user to unplug wrap from the cable   */
                        /* and put the cable back into the network      */
                        rc = diag_display(0x851007,
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

                        rc = diag_display(0x851007, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_unplug_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                clear_variables();
                                clean_up();
                        }

                        /* setting cable_tested variable in database to true */
                        cable_tested= FALSE;
                        putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                                &cable_tested);
                        break;
                case HAVE_WRAP_PLUG:
                        rc = diag_display(0x851008, catd,
                                have_wrap_plug, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_have_wrap_plug);
                        sprintf(msgstr, asi_have_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_have_wrap_plug[0].text);
                        asi_have_wrap_plug[0].text = msgstr;


                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(0x851008, catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_have_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                clear_variables();
                                clean_up();
                        }

                        rc = DIAG_ITEM_SELECTED (menutypes);
                        if      /* if user does not have the wrap NTF */
                                ( rc != YES)
                        {
                                cable_tested= FALSE;
                                putdavar(da_input.dname, "cable_tested",
                                        DIAG_SHORT, &cable_tested);
                                return (NO);
                        }
                        /* setting wrap_plug variable in data base to true */
                        wrap_plug = TRUE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);

                        rc = YES;
                        break;

                case PLUG_WRAP:
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x851009,
                                catd, put_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_wrap_plug);
                        sprintf(msgstr, asi_put_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_wrap_plug[0].text);
                        asi_put_wrap_plug[0].text = msgstr;
                        sprintf(msgstr1, asi_put_wrap_plug[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_put_wrap_plug[1].text);
                        asi_put_wrap_plug[1].text = msgstr1;

                        rc = diag_display(0x851009, catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                clear_variables();
                                clean_up();
                        }

                        /* setting cable_tested variable in database to true */
                        wrap_plug= TRUE;
                        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT,
                                &wrap_plug);
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
|       None
----------------------------------------------------------------------------*/

void report_frub (frub_idx)
int frub_idx;
{
        struct fru_bucket *frub_addr;
        switch (which_type)
        {
                case    COMMON_GROUP_TEST:
                        frub_addr = &artic_frus[frub_idx];
                        break;
                case    MPQP_4PORT:
                        frub_addr = &mpqp_frus[frub_idx];
                        break;
                case    MPQP_4PORT_ADVANCED:
                        frub_addr = &mpqp_advanced_frus[frub_idx];
                        break;
                case    X_25:
                        frub_addr = &x25_mp2_frus[frub_idx];
                        break;
                case    PORTMASTER_ADVANCED:
                        frub_addr = &pm_advanced_frus[frub_idx];
                        break;
                case    MP_2:
                        frub_addr = &mp2_advanced_frus[frub_idx];
                        break;
                case    T1E1J1:
                        break;
        }

        /* copy device name into the field device name of fru bucket */
        strcpy ( frub_addr->dname, da_input.dname);
        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        frub_addr->sn = 0x855;
                        break;
                case X_25:
                case MP_2:
                        frub_addr->sn = 0x849;
                        break;
                case T1E1J1:
                        frub_addr->sn = 0x851;
                        break;
        }

        insert_frub(&da_input, frub_addr);
        /* the reason why there is another switch because insert_frub   */
        /* will reset frub_addr->sn                                     */
        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        frub_addr->sn = 0x855;
                        break;
                case X_25:
                case MP_2:
                        frub_addr->sn = 0x849;
                        break;
                case T1E1J1:
                        frub_addr->sn = 0x851;
                        break;
        }
        addfrub(frub_addr);
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

        int     return_code =0;
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
} /* check_asl_stat     */

/*--------------------------------------------------------------------------
 | NAME: poll_kb_for_quit
 |
 | FUNCTION: Poll the keyboard input to see whether user want to
 |          quit
 |
 | EXECUTION ENVIRONMENT:
 |
 | NOTES:  Other local routines called --
 |      check_asl_stat()
 |
 | RETURNS:
 |      QUIT ---- user entered CANCEL,EXIT
 |      NO_ERROR -otherwise
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
/*--------------------------------------------------------------------------
| NAME: clean_up
|
| FUNCTION: Cleanup before exit DA ; closing catalog file ;
|           quiting asl; restore the database to the initial
|           state
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None
----------------------------------------------------------------------------*/

void clean_up ()
{
        int     rc = NO_ERROR;
        if (da_input.console)
                da_display();

        if (filedes != NOT_GOOD)
                close (filedes);

        restore_configuration();
        if (parent_state != -1)
                initial_state (parent_state, da_input.parent);

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

/*--------------------------------------------------------------------------
| NAME: check_driver_and_ucode
|
| FUNCTION:  This function will check the device driver file and
|            the microcode files.
|
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|       None
----------------------------------------------------------------------------*/
void check_driver_and_ucode ()
{
        int rc;
        char    base_microcode_path[64];
        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        rc = findmcode ("708fd",base_microcode_path,0,"");
                        break;
                case X_25:
                case MP_2:
                        rc = findmcode ("f0efd",base_microcode_path,0,"");
                        break;
                case T1E1J1:
                        rc = findmcode ("718fd.00",base_microcode_path,0,"");
                        break;
        }
        if (rc != 1)
        {
                if  ((da_input.console == CONSOLE_TRUE) &&
                     (da_input.loopmode & ( LOOPMODE_ENTERLM | LOOPMODE_NOTLM)))
                {
                        sprintf(msgstr, (char *)diag_cat_gets
                                  (catd, 1,DIAG_MICROCODE_MISSING), NULL);
                        menugoal (msgstr);
                }
                DA_SETRC_STATUS (DA_STATUS_BAD);
                clean_up();

        }


}       /* check driver and microcode   */
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
        struct CuDv     *cudv_p;
        struct listinfo v_info;
        char    criteria[128];
        int     i,rc;

        setlocale(LC_ALL,"");

        /* initializing interrupt handler */
        invec.sa_handler = intr_handler;
        sigaction(SIGINT, &invec, (struct sigaction *)NULL);
        invec.sa_handler = timeout;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );

        DA_SETRC_STATUS (DA_STATUS_GOOD);
        DA_SETRC_ERROR (DA_ERROR_NONE);
        DA_SETRC_TESTS (DA_TEST_FULL);
        DA_SETRC_MORE (DA_MORE_NOCONT);
        DA_SETRC_USER (DA_USER_NOKEY);


        for (i=0; i<=7; i++)
                child_state[i] = NOT_CONFIG;
        for (i=0; i<=15; i++)
                grandchild_state[i] = NOT_CONFIG;
        /* initialize odm       */
        odm_flg = init_dgodm();

        /* get TMInput object from database     */
        rc = getdainput (&da_input);
        if (odm_flg == -1 || rc != 0)
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                clean_up();
        }


        if      /* running under console */
           ( da_input.console == CONSOLE_TRUE)
        {
                /* initialize asl  & open catalog       */
                if (da_input.loopmode == LOOPMODE_INLM)
                        asl_flg = diag_asl_init (ASL_INIT_DEFAULT);
                else
                        asl_flg = diag_asl_init ("NO_TYPE_AHEAD");

                catd = diag_catopen (MF_DARTIC,0);
                if ( (asl_flg == NOT_GOOD) || (catd == CATD_ERR))
                        clean_up();
        }

        for (i=0; i<=7; i++)
        {
                other_child[i] = (char *) malloc (32 *sizeof (char));
                child_location[i] = (char *) malloc (32 *sizeof (char));
                if ((other_child[i] == NULL) || (child_location[i] == NULL))
                   clean_up();
        }

        for (i=0; i<=15; i++)
        {
                other_grandchild[i] = (char *) malloc (32 *sizeof (char));
                grandchild_location[i] = (char *) malloc (32 *sizeof (char));
                other_grandchild_configure_method[i] =(char *) malloc (64*sizeof (char));
                if ((other_grandchild[i] == NULL) || (grandchild_location[i] == NULL)
                     || (other_grandchild_configure_method[i] == NULL))
                   clean_up();

        }

        sprintf(criteria,"name=%s", da_input.dname);
        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
        if ( cudv_p == (struct CuDv *) NULL )
        {
                sprintf(msgstr, (char *)diag_cat_gets (
                   catd, 1,DEVICE_DOES_NOT_EXIST),
                   da_input.dname, da_input.dnameloc);
                if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                {
                        menugoal (msgstr);
                }
                clean_up();
        }

        if (strstr(cudv_p->PdDvLn_Lvalue, "isa"))
        {
           isabus = TRUE;
        }

        if (strstr (da_input.dname, "mpx"))      /* X.25 / MP/2 */
        {
           if (isabus == TRUE)
                sprintf(adapter_name, (char *)diag_cat_gets ( catd, 1,
                                X25_OR_MP_GENERIC));
           else
                sprintf(adapter_name, (char *)diag_cat_gets ( catd, 1,
                                X25_OR_MP_2_GENERIC));
        }
        else if (strstr (da_input.dname,"apm")) /* PORTMASTER  */
        {
                sprintf(adapter_name, (char *)diag_cat_gets ( catd, 1,
                                PORTMASTER_GENERIC));
        }

        p_tmp = (char *) malloc (32 *sizeof (char));
        msgstr = (char *) malloc (1200 *sizeof (char));
        msgstr1 = (char *) malloc (1200 *sizeof (char));
        lpp_unconfigure_method = (char *) malloc (256*sizeof (char));
        diag_unconfigure_method =(char *) malloc (256*sizeof (char));
        lpp_configure_method =(char *) malloc (256*sizeof (char));
        diag_configure_method =(char *) malloc (256*sizeof (char));
        diag_define_method =(char *) malloc (256*sizeof (char));
        diag_undefine_method=(char *) malloc (256*sizeof (char));
        lpp_unconfigure_device_name=(char *) malloc (256*sizeof (char));
        diag_configure_device_name=(char *) malloc (256*sizeof (char));
        otherdd_name =(char *) malloc (256*sizeof (char));
        other_dd_configure_method =(char *) malloc (256*sizeof (char));
        other_dd_unconfigure_method =(char *) malloc (256*sizeof (char));
        other_port_configure_method =(char *) malloc (256*sizeof (char));
        other_port_unconfigure_method =(char *) malloc (256*sizeof (char));

        if((msgstr == (char *) NULL) ||
           (msgstr1 == (char *) NULL) ||
           (lpp_unconfigure_method == (char *) NULL) ||
           (diag_unconfigure_method == (char *) NULL) ||
           (lpp_configure_method == (char *) NULL) ||
           (diag_configure_method ==(char *) NULL) ||
           (diag_define_method ==(char *) NULL) ||
           (diag_undefine_method == (char *)NULL) ||
           (lpp_unconfigure_device_name == (char *) NULL) ||
           (p_tmp == (char *) NULL) ||
           (otherdd_name == (char *) NULL) ||
           (other_dd_configure_method == (char *) NULL) ||
           (other_dd_unconfigure_method == (char *) NULL) ||
           (other_port_configure_method == (char *) NULL) ||
           (other_port_unconfigure_method == (char *) NULL) ||
           (diag_configure_device_name == (char *) NULL))
        {
                clean_up();
        }
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
   int i;

        for (i = 0; i < 16; i++)
        {
           if (other_grandchild[i] != (char *) NULL)
              free(other_grandchild[i]);
           if (grandchild_location[i] != (char *) NULL)
              free(grandchild_location[i]);
           if (other_grandchild_configure_method[i] != (char *) NULL)
              free(other_grandchild_configure_method[i]);
        }

        for (i = 0; i < 8; i++)
        {
           if (other_child[i] != (char *) NULL)
              free(other_child[i]);
           if (child_location[i] != (char *) NULL)
              free(child_location[i]);
        }

        if(p_tmp != (char *) NULL)
                free (p_tmp);
        if(msgstr != (char *) NULL)
                free (msgstr);
        if(msgstr1 != (char *) NULL)
                free (msgstr1);
        if(diag_unconfigure_method != (char *) NULL)
                free (diag_unconfigure_method);
        if(lpp_configure_method != (char *) NULL)
                free (lpp_configure_method);
        if(diag_configure_method !=(char *) NULL)
                free (diag_configure_method);
        if(diag_define_method !=(char *) NULL)
                free (diag_define_method);
        if(diag_undefine_method != (char *)NULL)
                free (diag_undefine_method);
        if(lpp_unconfigure_device_name != (char *) NULL)
                free (lpp_unconfigure_device_name);
        if(diag_configure_device_name != (char *) NULL)
                free (diag_configure_device_name);
        if (otherdd_name != (char *) NULL)
                free (otherdd_name);
        if (other_dd_configure_method != (char *) NULL)
                free(other_dd_configure_method);
        if (other_dd_unconfigure_method != (char *) NULL)
                free(other_dd_unconfigure_method);
        if (other_port_configure_method != (char *) NULL)
                free(other_dd_configure_method);
        if (other_port_unconfigure_method != (char *) NULL)
                free(other_dd_unconfigure_method);
}
/* -------------------------------------------------------------------------
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
 ------------------------------------------------------------------------- */

void set_timer()
{
        invec.sa_handler = timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(60);
}

/*-------------------------------------------------------------------------*/
int     restore_configuration()
{
        struct CuDv     *cudv_p;
        struct listinfo v_info;
        struct PdDv     *pddv_p;
        char    criteria[128];
        char    *result_p;
        int     result;

        /* configure diagnostics device */
        result  = unconfigure_diag_dd ();
        result = configure_lpp_device ();

}
/*---------------------------------------------------------------------------
 * NAME: configure_diag_dd()
 *
 * FUNCTION: configure_diag_dd()
 *           Define the diagnostics device driver if it is not
 *           Make the diagnostics device driver if it is not AVAILABLE
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/

int     configure_diag_dd ()
{
        struct CuDv     *cudv_p;
        struct listinfo v_info;
        struct PdDv     *pddv_p;
        char    criteria[128];
        char    *result_p;
        int     result;

        if (strstr (da_input.dname, "mpx"))      /* X.25 / MP/2 */
           if (isabus == TRUE)
              sprintf (criteria,
                       "uniquetype=driver/articmpx/articdgisa");
           else
              sprintf (criteria,
                       "uniquetype=driver/articmpx/articdiag");
        else
           if (isabus == TRUE)
              sprintf (criteria,
                       "uniquetype=driver/portmaster/articdgisa");
           else
              sprintf (criteria,
                       "uniquetype=driver/portmaster/articdiag");
        pddv_p = get_PdDv_list(PdDv_CLASS, criteria, &v_info, 1, 1);
        if ( pddv_p == (struct PdDv *) NULL )
        {
                sprintf(msgstr, (char *)diag_cat_gets (
                   catd, 1,DIAG_DEVICE_CANNOT_BE_DEFINED),
                   da_input.dname, da_input.dnameloc, adapter_name);
                if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                {
                        menugoal (msgstr);
                }
                clean_up();
        }

        sprintf (criteria,
                "location like %s* AND name like diagric* AND chgstatus != 3",
                da_input.dnameloc);

        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
        /* there is no device driver available in the system    */
        /* define the device driver                             */
        if ( cudv_p == (struct CuDv *) NULL )
        {
                /*      Define the device driver into the system        */

                if (strstr (da_input.dname, "mpx"))      /* X.25 / MP/2 */
                {
                    if (isabus == TRUE)
                        sprintf (option,
                          " -c driver -s articmpx -t articdgisa -p '%s' -w 0",
                                da_input.dname);
                    else
                        sprintf (option,
                          " -c driver -s articmpx -t articdiag -p '%s' -w 0",
                                da_input.dname);

                }
                else            /* Port master          */
                {
                   if (isabus == TRUE)
                        sprintf (option,
                          " -c driver -s portmaster -t articdgisa -p '%s' -w 0",
                                da_input.dname);
                   else
                        sprintf (option,
                          " -c driver -s portmaster -t articdiag -p '%s' -w 0",
                                da_input.dname);
                }
                result = odm_run_method(pddv_p->Define,
                        option, &new_out,&new_err);
                if (result == NO_ERROR )
                {
                        strcpy (diag_configure_device_name, new_out);
                        diag_dd_is_defined = TRUE;
                }
                else
                {
                        sprintf(msgstr, (char *)diag_cat_gets (
                           catd, 1,DIAG_DEVICE_CANNOT_BE_DEFINED),
                           da_input.dname, da_input.dnameloc, adapter_name);
                        if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                        {
                                menugoal (msgstr);
                        }
                        clean_up();
                }
        }
        else
        {
                /* diagnostics device is in the system */
                /* try to unconfigure i                 */
                strcpy (diag_configure_device_name, cudv_p->name);
                sprintf (option," -l %s", cudv_p->name);
                strcpy (criteria,pddv_p->Unconfigure);
                result = invoke_method(criteria,option, &new_out,&new_err);
                if (result !=  NO_ERROR)
                {
                        sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                DIAG_DEVICE_CANNOT_UNCONFIGURED),
                                da_input.dname, da_input.dnameloc, adapter_name);
                        if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                        {
                                menugoal (msgstr);
                        }
                        /* cannot do clean up here because this routine */
                        /* is called by clean up                        */
                }
        }

        /* The diagnostics device driver needs to be loaded into the system */
        /* Configure the device driver into the system                      */

        sprintf (option," -l %s", diag_configure_device_name);
        strcpy (criteria,pddv_p->Configure);
        result = invoke_method(criteria,option, &new_out,&new_err);
        if (result == NO_ERROR)
        {
                diag_dd_status = AVAILABLE;
        }
        else
        {
                /* report frubucket and exit DA         */
                /* report_fru(&frub905);                */
                DA_SETRC_STATUS (DA_STATUS_BAD);
                clean_up();
        }
}

int     unconfigure_diag_dd()
{
        struct CuDv     *cudv_p;
        struct listinfo v_info;
        struct PdDv     *pddv_p;
        char    criteria[128];
        char    *result_p;
        int     result;

        if (strstr (da_input.dname, "mpx"))      /* X.25 / MP/2 */
           if (isabus == TRUE)
              sprintf (criteria,
                       "uniquetype=driver/articmpx/articdgisa");
           else
              sprintf (criteria,
                       "uniquetype=driver/articmpx/articdiag");
        else
           if (isabus == TRUE)
              sprintf (criteria,
                       "uniquetype=driver/portmaster/articdgisa");
           else
              sprintf (criteria,
                       "uniquetype=driver/portmaster/articdiag");
        pddv_p = get_PdDv_list(PdDv_CLASS, criteria, &v_info, 1, 1);
        if ( pddv_p == (struct PdDv *) NULL )
        {
                sprintf(msgstr, (char *)diag_cat_gets (
                   catd, 1,DIAG_DEVICE_CANNOT_BE_UNDEFINED),
                   da_input.dname, da_input.dnameloc, adapter_name);
                if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                {
                        menugoal (msgstr);
                }
                clean_up();
        }

        /* Unconfigure diagnostics device. Unload the device from system */
        if(diag_dd_status == AVAILABLE)
        {
                sprintf (option," -l %s", diag_configure_device_name);
                strcpy (criteria,pddv_p->Unconfigure);
                result = invoke_method(criteria,option, &new_out,&new_err);
                if (result !=  NO_ERROR)
                {
                        sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                DIAG_DEVICE_CANNOT_UNCONFIGURED),
                                da_input.dname, da_input.dnameloc, adapter_name);
                        if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                        {
                                menugoal (msgstr);
                        }
                        /* cannot do clean up here because this routine */
                        /* is called by clean up                        */
                }
                else
                {
                        diag_dd_status = NOT_CONFIG;
                }
        }

        /* if the diagnostic device is defined in the system undefine it */

        if (diag_dd_is_defined)
        {
                sprintf (option," -l %s", diag_configure_device_name);
                strcpy (criteria,pddv_p->Undefine);
                result = odm_run_method(criteria,option,
                         &new_out,&new_err);
                if (result == NO_ERROR)
                {
                        diag_dd_is_defined = FALSE;
                }
        }
}
int     unconfigure_lpp_device ()
{
        /* unconfigure mpqx whose parent is the child of da_input.dname */

        struct CuDv     *cudv_p;
        struct listinfo v_infoc, v_infop;
        struct PdDv     *pddv_p;
        char    criteria[128];
        char    *result_p;
        int     result,i, j;


        strcpy (otherdd_name, "non_exist");
        /* First to look if the adapter apmX is available or defined    */

        sprintf (criteria,
                "location like %s* AND name=%s AND chgstatus != 3",
                da_input.dnameloc, da_input.dname);
        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_infoc, 1, 1);

        if ( cudv_p != (struct CuDv *) NULL )
        {
                /* getting all the information about this adapter       */
                /* like config and unconfig methods for later use       */

                adapter_state = cudv_p->status;

                sprintf (criteria, "uniquetype=%s",
                                        cudv_p->PdDvLn_Lvalue);
                pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                        &v_infop, 1, 1);
                if (pddv_p == (struct PdDv *) NULL)
                        /* this should never happen hopefully   */
                {
                        sprintf(msgstr, (char *)diag_cat_gets (
                                catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                cudv_p->name, cudv_p->location);
                        if(da_input.loopmode &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                        {
                                menugoal (msgstr);
                        }
                        clean_up();
                }

                strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
                strcpy (lpp_configure_method, pddv_p->Configure);

                if (adapter_state != AVAILABLE)
                {
                        parent_state = configure_device (da_input.parent);
                        /* bring up adapter     */
                        sprintf (option," -l %s", da_input.dname);
                        strcpy (criteria,lpp_configure_method);
                        result = odm_run_method(criteria,option,
                                &new_out,&new_err);
                        if (result !=  NO_ERROR)
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                        LPP_DEVICE_CANNOT_CONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                if(da_input.loopmode &
                                        (LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                {
                                        menugoal (msgstr);
                                }
                                clean_up();
                        }
                        adapter_is_configured = TRUE;
                        adapter_state = AVAILABLE;

                }


                /* Second look if a device driver      */
                /* is the child of  ampX               */

                sprintf (criteria,"parent=%s", da_input.dname);
                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_infoc, 1, 1);
                if ( cudv_p != (struct CuDv *) NULL )
                {

                        other_dd_state = cudv_p->status;
                        /* get all the information about this device driver */

                        strcpy (otherdd_name, cudv_p->name);
                        strcpy (otherdd_location, cudv_p->location);
                        sprintf (criteria, "uniquetype=%s",
                                        cudv_p->PdDvLn_Lvalue);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                &v_infop, 1, 1);
                        if (pddv_p == (struct PdDv *) NULL)
                                /* this should never happen hopefully   */
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                if (da_input.loopmode
                                        &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                {
                                        menugoal (msgstr);
                                }
                                clean_up();
                        }
                        strcpy(other_dd_unconfigure_method, pddv_p->Unconfigure);
                        strcpy(other_dd_configure_method, pddv_p->Configure);

                }

                /* Third look if any ports are the        */
                /* children of device driver              */
                sprintf (criteria,"status=1  AND parent=%s", otherdd_name);
                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_infoc, 1, 1);
                if ( cudv_p != (struct CuDv *) NULL )
                {
                        /* getting all config and unconfig information */
                        sprintf (criteria, "uniquetype=%s",
                                        cudv_p->PdDvLn_Lvalue);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                &v_infop, 1, 1);
                        if (pddv_p == (struct PdDv *) NULL)
                                /* this should never happen hopefully   */
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                if (da_input.loopmode
                                        &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                {
                                        menugoal (msgstr);
                                }
                                clean_up();
                        }
                        strcpy(other_port_unconfigure_method,
                                        pddv_p->Unconfigure);
                        strcpy(other_port_configure_method,
                                        pddv_p->Configure);

                        for (i=0; i<v_infoc.num; i++)
                        {
                                strcpy (other_child[i], cudv_p->name);
                                child_state[i] = cudv_p->status;
                                strcpy (child_location[i],cudv_p->location);

                                cudv_p++;
                        }
                }


                /* Fourth . If  ports (child1, child2, child3 exist */
                /* Then unconfigure these ports                         */
                j = 0;
                for (i=0; i<=7; i++)
                {
                        if (child_state[i] == AVAILABLE)
                        {
                                sprintf (criteria,"status=1  AND parent=%s",
                                         other_child[i]);
                                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_infoc, 1, 1);
                                while ( j < v_infoc.num )
                                {
                                   sprintf (criteria, "uniquetype=%s",
                                            cudv_p->PdDvLn_Lvalue);
                                   pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                                           &v_infop, 1, 1);
                                   if (pddv_p == (struct PdDv *) NULL)
                                   /* this should never happen hopefully   */
                                   {
                                      sprintf(msgstr, (char *)diag_cat_gets (
                                              catd, 1,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                      cudv_p->name, cudv_p->location);
                                      if (da_input.loopmode
                                          &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                      {
                                          menugoal (msgstr);
                                      }
                                      clean_up();
                                   }

                                   grandchild_state[j] = AVAILABLE;
                                   strcpy(other_grandchild[j], cudv_p->name);
                                   strcpy(grandchild_location[j], cudv_p->location);
                                   strcpy(other_grandchild_configure_method[j], pddv_p->Configure);
                                   sprintf (option," -l %s", cudv_p->name);
                                   strcpy (criteria,pddv_p->Unconfigure);
                                   result = odm_run_method(criteria,option,
                                                           &new_out,&new_err);
                                   if (result !=  NO_ERROR)
                                   {
                                        sprintf(msgstr,
                                                (char *)diag_cat_gets ( catd, 1,
                                                 LPP_DEVICE_CANNOT_UNCONFIGURED),
                                                 other_grandchild[j], grandchild_location[j]);
                                        if (da_input.loopmode
                                           &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                        {
                                                menugoal (msgstr);
                                        }
                                        clean_up();
                                   }
                                   ++j;
                                   ++cudv_p;

                                }


                                sprintf (option," -l %s", other_child[i]);
                                strcpy (criteria,other_port_unconfigure_method);
                                result =
                                        odm_run_method(criteria,option,
                                                &new_out,&new_err);
                                if (result !=  NO_ERROR)
                                {
                                        sprintf(msgstr,
                                            (char *)diag_cat_gets ( catd, 1,
                                             LPP_DEVICE_CANNOT_UNCONFIGURED),
                                             other_child[i], child_location[i]);
                                        if (da_input.loopmode
                                           &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                        {
                                                menugoal (msgstr);
                                        }
                                        clean_up();
                                }
                        }
                }

                /* Fifth .. Unconfigure other device driver               */
                if (other_dd_state == AVAILABLE)
                {

                        sprintf (option," -l %s", otherdd_name);
                        strcpy (criteria,other_dd_unconfigure_method);
                        result =
                            odm_run_method(criteria,option, &new_out,&new_err);
                        if (result !=  NO_ERROR)
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                        LPP_DEVICE_CANNOT_UNCONFIGURED),
                                        otherdd_name, otherdd_location);
                                if (da_input.loopmode
                                   &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                {
                                        menugoal (msgstr);
                                }
                                clean_up();
                        }

                }


        }

        else
        {
                /* Menu goal     Hope this will never happen */
                clean_up();
        }

        /* Configure diagnostics device driver                  */
        result  = configure_diag_dd ();
}

int     configure_lpp_device ()
{

        struct CuDv     *cudv_p;
        struct listinfo v_info;
        struct PdDv     *pddv_p;
        char    criteria[128];
        char    *result_p;
        int     result;
        int     i;
        int     error_message=FALSE;


        /* Last to look if the adapter apmX is available or defined     */
        if ((adapter_state == AVAILABLE) && (adapter_is_configured))
        {
                /* Make adapter in define state and returned */
                sprintf (option," -l %s", da_input.dname);
                strcpy (criteria,lpp_unconfigure_method);
                result = odm_run_method(criteria,option, &new_out,&new_err);
                if (result !=  NO_ERROR)
                {
                        sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_UNCONFIGURED),
                                cudv_p->name, cudv_p->location);
                        if (da_input.loopmode
                           &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                        {
                                menugoal (msgstr);
                        }
                        error_message = TRUE;
                }
        }

        /* Configure other device driver if it is available before testing */
        if (other_dd_state == AVAILABLE)
        {

                sprintf (option," -l %s", otherdd_name);
                strcpy (criteria,other_dd_configure_method);
                result = odm_run_method(criteria,option, &new_out,&new_err);
                if (result !=  NO_ERROR)
                {
                        sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                LPP_DEVICE_CANNOT_CONFIGURED),
                                otherdd_name, otherdd_location);
                        if (!error_message )
                        {
                                if (da_input.loopmode
                                &(LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                {
                                        menugoal (msgstr);
                                }
                                error_message = TRUE;
                        }
                }

        }


        /* Configure all the children that are AVAILABLE before testing  */
        for (i=0; i<=7; i++)
        {
                if (child_state[i] == AVAILABLE)
                {
                        sprintf (option," -l %s", other_child[i]);
                        strcpy (criteria,other_port_configure_method);
                        result =
                            odm_run_method(criteria,option, &new_out,&new_err);
                        if (result !=  NO_ERROR)
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                        LPP_DEVICE_CANNOT_CONFIGURED),
                                        other_child[i], child_location[i]);
                                if (!error_message )
                                {
                                        if (da_input.loopmode &
                                            (LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                        {
                                                menugoal (msgstr);
                                        }
                                        error_message = TRUE;
                                }
                        }
                }
        }

        /* Configure all the grandchildren that are AVAILABLE before testing  */
        for (i=0; i<=15; i++)
        {
                if (grandchild_state[i] == AVAILABLE)
                {
                        sprintf (option," -l %s", other_grandchild[i]);
                        strcpy (criteria,other_grandchild_configure_method[i]);
                        result =
                            odm_run_method(criteria,option, &new_out,&new_err);
                        if (result !=  NO_ERROR)
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                        LPP_DEVICE_CANNOT_CONFIGURED),
                                        other_child[i], child_location[i]);
                                if (!error_message )
                                {
                                        if (da_input.loopmode &
                                            (LOOPMODE_NOTLM|LOOPMODE_ENTERLM))
                                        {
                                                menugoal (msgstr);
                                        }
                                        error_message = TRUE;
                                }
                        }
                }
        }
}

/*--------------------------------------------------------------*/
/*      NAME: Set adapter name                                  */
/*      Description:                                            */
/*      Return          : 0 Good                                */
/*                        -1 BAD                                */
/*--------------------------------------------------------------*/
int     set_adapter_name()
{
        int     rc;
        int     message_number;
        rack_model = FALSE;
        switch (EIB_type)
        {
                case 0:         /* X.25 adapter         */

                        message_number = X25_ADAPTER;
                        card_type = X_25;
                        break;

                case 1:         /* MP/2 4-Port 232      */
                        message_number = MP2_4PORT_232;
                        card_type = MP_2;
                        break;

                case 2:         /* MP/2 6-Port Sync.    */
                        message_number = MP2_6PORT_232;
                        card_type = MP_2;
                        break;

                case 3:         /* MP/2 8-Port 232      */
                        message_number = MP2_8PORT_232;
                        card_type = MP_2;
                        break;
                case 4:         /* MP/2 8-Port 422      */
                        message_number = MP2_8PORT_422;
                        card_type = MP_2;
                        break;
                case 5:         /* MP/2 4-port 232 / 4-Port 422 */
                        message_number = MP2_4PORT_232_C4;
                        card_type = MP_2;
                        break;
                case 99:        /* MP/2 Unrecognised EIB        */
                        message_number =UNRECOGNISED_MP2_EIB;
                        card_type = MP_2;
                        break;

                case 100:       /* 4-Port MPQP          */
                        message_number = MPQP_4PORT_ADAPTER;
                        card_type = MPQP_4PORT;
                        break;

                case 101:       /* Portmaster 6-Port V.35*/
                        message_number = PM_6PORT_V35;
                        card_type = PM;
                        break;

                case 102:       /* Portmaster 6-Port X.21*/
                        message_number = PM_6PORT_X21;
                        card_type = PM;
                        break;

                case 103:       /* Portmaster 8-Port 232 */
                        message_number = PM_8PORT_232;
                        card_type = PM;
                        break;

                case 104:       /* Portmaster 8-Port 422 */
                        message_number = PM_8PORT_422;
                        card_type = PM;
                        break;
                case 199:       /* Portmaster Unreconized EIB   */
                        message_number = UNRECOGNISED_PM_EIB;
                        card_type = PM;
                        break;
                case 200:       /* T1E1J1                       */
                        message_number = T1E1J1_ADAPTER;
                        card_type = T1E1J1;
                        break;

                case 299:       /* Unrecognised EIB for T1J1E1  */
                        message_number = UNRECOGNISED_T1E1J1_EIB;
                        card_type = T1E1J1;
                        break;
                case 300:         /* C1X adapter         */

                        message_number = C1X_ADAPTER;
                        card_type = X_25;
                        break;
                case 399:        /* MP Unrecognised EIB        */
                        message_number = UNRECOGNISED_MP_EIB;
                        card_type = MP_2;
                        break;


                default:
                        break;
        }
        strcpy(adapter_name, (char *)diag_cat_gets ( catd, 1, message_number));
        init_dgodm();
        get_cpu_model(&rc);

        if (IS_RACK(rc))
        {
                rack_model = TRUE;
        }

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
        if (da_input.loopmode & (LOOPMODE_NOTLM | LOOPMODE_ENTERLM))
        {
                wrap_plug = FALSE;
                putdavar(da_input.dname, "wrap_plug",DIAG_SHORT, &wrap_plug);
                cable_tested = FALSE;
                putdavar(da_input.dname,
                "cable_tested", DIAG_SHORT, &cable_tested);
                wrap_plug_78_pin = FALSE;
                putdavar(da_input.dname, "wrap_plug_78_pin",DIAG_SHORT,
                        &wrap_plug_78_pin);
        }
}
/*-------------------------------------------------------------------------
 * NAME: generic_report_frub
 *
 * FUNCTION: reports the fru bucket whose address is specified as frub_addr
 *
 * EXECUTION ENVIRONMENT:
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/

void generic_report_frub(frub_addr)
struct fru_bucket *frub_addr;
{
        strcpy ( frub_addr->dname, da_input.dname);
        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        frub_addr->sn = 0x855;
                        break;
                case X_25:
                case MP_2:
                        frub_addr->sn = 0x849;
                        break;
                case T1E1J1:
                        frub_addr->sn = 0x851;
                        break;
        }
        insert_frub(&da_input, frub_addr);

        /* the reason why there is another switch because insert_frub   */
        /* will reset frub_addr->sn                                     */
        switch (card_type)
        {
                case MPQP_4PORT:
                case PM:
                        frub_addr->sn = 0x855;
                        break;
                case X_25:
                case MP_2:
                        frub_addr->sn = 0x849;
                        break;
                case T1E1J1:
                        frub_addr->sn = 0x851;
                        break;
        }
        addfrub(frub_addr);
        DA_SETRC_STATUS (DA_STATUS_BAD);
}
