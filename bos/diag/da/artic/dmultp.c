static char sccsid[] = "@(#)67  1.2  src/bos/diag/da/artic/dmultp.c, daartic, bos41J, 9511A_all 3/3/95 11:13:50";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: cable_test_port0
 *              cable_test_port1
 *              cable_test_port2
 *              cable_test_port3
 *              mpqp_advanced_tests
 *              ports_testing
 *              report_frub_mpqp
 *              report_frub_mpqp_cable
 *              running_78_pin_wrap_plug_tests
 *              test_232
 *              test_422
 *              test_V35
 *              test_X21
 *              which_port
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
#include <sys/errids.h>
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/da.h"
#include "diag/diag_exit.h"
#include "diag/diag.h"
#include "diag/diago.h"
#include "diag/bit_def.h"
#include "dartic.h"
#include "dartic_msg.h"
#include "dartic_ext.h"
#include "dmultp_msg.h"
#include "artictst.h"
#include <locale.h>

extern short    wrap_plug;
extern short    cable_tested;
extern  nl_catd diag_catopen();
extern  getdainput();           /* gets input from ODM                  */
extern  long getdamode();       /* transforms input into a mode         */
extern  int errno;              /* reason device driver won't open      */
extern  int which_type;
extern short   wrap_plug_V35;
extern short   wrap_plug_232;
extern short   wrap_plug_422;

extern volatile int alarm_timeout;
struct  sigaction  invec;       /* interrupt handler structure  */

extern short    wrap_plug_78_pin;


short   port_0_V35_tested;
short   port_1_V35_tested;
short   port_0_X21_tested;
short   port_0_232_tested;
short   port_1_232_tested;
short   port_2_232_tested;
short   port_3_232_tested;
short   port_0_422_tested;
short   port_2_422_tested;

short   cable_isolate_test=FALSE;


extern TUTYPE   artic_tucb;
extern struct   tm_input da_input;
extern int      filedes;
extern nl_catd catd;
nl_catd mpqp_catd;
extern int      card_type;
extern int      exectu ();


extern int      test_tu(int, int, int);
extern int      verify_rc(int*,int);
extern int      poll_kb_for_quit();
extern int      check_asl_stat(int);
extern void     intr_handler(int);
extern void     report_frub();
extern void     clean_up();
extern void     timeout(int);
extern void     clear_variables();
extern void    set_timer();
void    mpqp_unconfigure_lpp_device ();

int     port0_tests ();
int     port1_tests ();
int     port2_tests ();
int     port3_tests ();

int     test_V35();
int     test_X21();
int     test_232();
int     test_422();

int     cable_test_port0();
int     cable_test_port1();
int     cable_test_port2();
int     cable_test_port3();
void report_frub_mpqp_cable ();
void report_frub_mpqp();


/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/

extern char *msgstr;
extern char *msgstr1;


/*--------------------------------------------------------------------------
| NAME: mpqp_advanced_tests
|
| FUNCTION: Advanced test for MPQP 4 port adapter
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               0 :     GOOD
|               Other : BAD
----------------------------------------------------------------------------*/
mpqp_advanced_tests ()
{
        int     rc = NO_ERROR;
        int     rc1;

        artic_tucb.header.mfg = 0;
        artic_tucb.header.loop = 1;

        mpqp_catd = diag_catopen (MF_DMULTP,0);

        if (da_input.loopmode == LOOPMODE_INLM)
        {
                /* running 78 pin wrap plug test */
                getdavar(da_input.dname, "wrap_plug_78_pin", DIAG_SHORT,
                        &wrap_plug_78_pin);
                if (wrap_plug_78_pin)
                        rc = running_78_pin_wrap_plug_tests ();
        }
        else if ((da_input.loopmode == LOOPMODE_NOTLM) ||
                (da_input.loopmode == LOOPMODE_ENTERLM))
        {
                rc = display (DO_YOU_HAVE_78_PIN_WRAP_PLUG);
                da_display ();
                if (rc != NO)
                {
                        rc = display (INSERT_78_PIN_WRAP_PLUG);
                        if (rc != QUIT)
                        {
                                /* running 78 pin wrap plug test */
                                rc = running_78_pin_wrap_plug_tests ();
                                if (!(da_input.loopmode & LOOPMODE_ENTERLM))
                                        rc1 = display (REMOVE_78_PIN_WRAP_PLUG);
                                if ((!(da_input.loopmode & LOOPMODE_ENTERLM)) &&
                                        ( rc == NO_ERROR))
                                {
                                        rc = ports_testing ();
                                }
                        }
                }
                da_display ();
        }
        else if (da_input.loopmode == LOOPMODE_EXITLM)
        {
                getdavar(da_input.dname, "wrap_plug_78_pin", DIAG_SHORT,
                        &wrap_plug_78_pin);
                if (wrap_plug_78_pin)
                        display (REMOVE_78_PIN_WRAP_PLUG);
        }
        close (mpqp_catd);
        return (rc);
}
/*----------------------------------------------------------------------------*/
/*      Function :      running_78_pin_wrap_plug_tests ()                     */
/*      Description:    This funtion will  run all the test that require      */
/*                      78 pin wrap plug                                      */
/*      Return          0 : good                                              */
/*                      -1 : bad                                              */
/*----------------------------------------------------------------------------*/
int running_78_pin_wrap_plug_tests ()
{
        int rc;
        int     fru_index=0;
        struct  mpqp_internal_fru_pair *start;

        start = mpqp_internal_tests;
        da_display ();

        which_type = MPQP_4PORT_ADVANCED;

        /* Running test 32 33 34 35 36 37 38 39 40 41 */
        for (; start->tu != -1; ++start, ++fru_index)
        {
                rc = test_tu(start->tu,fru_index, which_type);
                if ( rc != NO_ERROR)
                        break;
                 if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
                        return(QUIT);
        }
        if (rc != 0) return (rc);


        da_display ();
        return (0);

}
/*----------------------------------------------------------------------------*/
/*      Function :      ports_testing                                         */
/*      Description:    This function will test all interfaces of the port    */
/*      Return          0 : good                                              */
/*                      -1 : bad                                              */
/*----------------------------------------------------------------------------*/
int     ports_testing ()
{
        int rc  = NO_ERROR;
        int rc1 = NO_ERROR;
        int cable_tested ;

        /* ask if testing cable is required     */
        rc1 = display (CABLE_NOTE);
        if (rc1 == YES)
        {
                while  ((rc != QUIT )  && ( rc == NO_ERROR))
                {
                        cable_tested = TRUE;
                        rc = display (CABLE_MENU_TESTING);
                        da_display ();
                        if (rc == 1)    /* port 0 V35 tested is selected */
                                rc1 = cable_test_port0 (1);
                        else if (rc == 2)
                                rc1 = cable_test_port0 (2);
                        else if ( rc == 3)
                                rc1 = cable_test_port0 (3);
                        else if ( rc == 4)
                                rc1 = cable_test_port0 (4);
                        else if ( rc == 5)
                                rc1 = cable_test_port1 (1);
                        else if ( rc == 6)
                                rc1 = cable_test_port1 (2);
                        else if ( rc == 7)
                                rc1 = cable_test_port2 (1);
                        else if ( rc == 8)
                                rc1 = cable_test_port2 (2);
                        else if ( rc == 9)
                                rc1 = cable_test_port3 ();
                        if ((rc1 != QUIT) && (rc != QUIT))
                                rc = rc1;
                        else if (rc != QUIT)
                                rc = NO_ERROR;

                        if ((port_0_V35_tested) && (port_0_232_tested) &&
                        (port_0_X21_tested) && (port_0_422_tested) &&
                        (port_2_232_tested) && (port_2_422_tested) &&
                        (port_1_V35_tested) && (port_1_232_tested) &&
                        (port_3_232_tested))
                        {
                                return (0);
                        }
                }

        }
        if ((rc != QUIT) && (rc != NO_ERROR))
        {
                return (QUIT);
        }


        /* Running port connectors testing      */
        /* Ask the user if testing port connectors is necessary */
        rc1 = display(TESTING_INDIVIDUAL_PORT_CONNECTOR);
        if ((rc1 != NO ) && (rc1 != QUIT))
        {
                rc = NO_ERROR;
                while  ((rc != QUIT )  && ( rc == NO_ERROR))
                {
                        cable_tested = FALSE;
                        rc = display (PORTS_MENU_TESTING);
                        if (rc == 1)    /* port 0 V35 tested is selected */
                                rc1 = test_V35(0,cable_tested );
                        else if (rc == 2)
                                rc1 = test_232(0, cable_tested);
                        else if ( rc == 3)
                                rc1 = test_X21 (cable_tested);
                        else if ( rc == 4)
                                rc1 = test_422(0, cable_tested);
                        else if ( rc == 5)
                                rc1 = test_232(1, cable_tested);
                        else if ( rc == 6)
                                rc1 = test_V35(1,cable_tested );
                        else if ( rc == 7)
                                rc1 = test_232(2, cable_tested);
                        else if ( rc == 8)
                                rc1 = test_422(2, cable_tested);
                        else if ( rc == 9)
                                rc1 = test_232 (3,cable_tested);
                        if ((rc1 != QUIT) && (rc != QUIT))
                                rc = rc1;
                        else if (rc != QUIT)
                                rc = NO_ERROR;

                        if ((port_0_V35_tested) && (port_0_232_tested) &&
                        (port_0_X21_tested) && (port_0_422_tested) &&
                        (port_2_232_tested) && (port_2_422_tested) &&
                        (port_1_V35_tested) && (port_1_232_tested) &&
                        (port_3_232_tested))
                        {
                                break;
                        }
                }
        }
}
/*--------------------------------------------------------------------------
| NAME: test_V35
|
| FUNCTION: test V35 port
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               0 :     GOOD
|               9999 :  QUIT
|               Other : BAD
----------------------------------------------------------------------------*/

int test_V35(int which_port, int cable_tested)
{
        int     rc= NO_ERROR;
        int     rc1= NO_ERROR;
        int     rc2= NO_ERROR;

        /* ask the user if he has V.35 wrap plug        */
        if ((!cable_isolate_test) && (!wrap_plug_V35))
        {
                rc1 = display_generic_wrap_plug (WRAP_PLUG_V35);
                if (rc1 == YES)
                        wrap_plug_V35 = TRUE;
        }
        else
                rc1 = YES;

        if (rc1 != YES)
                return (0);
        else
        {
                switch (which_port)
                {
                        case 0:
                            if ((!cable_isolate_test)&& (cable_tested))
                                rc = display (INSERT_V35_CABLE_PORT_0);
                            else if (!cable_isolate_test)
                                rc = display (INSERT_V35_WRAP_PLUG_PORT_0);
                            da_display ();
                            if (rc != QUIT)
                            {

                                 /* running the test */
                                 artic_tucb.header.tu = 45;
                                 rc2 = exectu(filedes, &artic_tucb);
                                 if ((rc2 != NO_ERROR) && (!cable_tested))
                                 {
                                        if (verify_rc (&tu45[0],rc2))
                                                report_frub_mpqp ();

                                 }
                                 else if (rc2 == NO_ERROR)
                                        port_0_V35_tested= TRUE;
                                 if ((rc2 != NO_ERROR) && (cable_tested))
                                 {
                                        rc = rc2;
                                 }
                                 else if ((!cable_isolate_test)&&
                                                (cable_tested))
                                        rc=display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0);
                                 else if (!cable_isolate_test)
                                        rc=display(REMOVE_V35_WRAP_PLUG_PORT_0);
                                 da_display ();
                            }
                            break;
                        case 1:

                            if ((!cable_isolate_test)&& (cable_tested))
                                rc = display (INSERT_V35_CABLE_PORT_1);
                            else if (!cable_isolate_test)
                                rc = display (INSERT_V35_WRAP_PLUG_PORT_1);
                            da_display ();
                            if (rc != QUIT)
                            {
                                /* running the test */
                                artic_tucb.header.tu = 47;
                                rc2 = exectu(filedes, &artic_tucb);
                                if ((rc2 != NO_ERROR)  & (!cable_tested))
                                {
                                        if (verify_rc (&tu47[0],rc2))
                                                report_frub_mpqp ();

                                }
                                else if (rc2 == NO_ERROR)
                                        port_1_V35_tested= TRUE;
                                if ((rc2 != NO_ERROR) && (cable_tested))
                                {
                                        rc = rc2;
                                }
                                else if ((!cable_isolate_test)&&(cable_tested))

                                   rc =
                                    display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_1);
                                else if (!cable_isolate_test)
                                   rc=display(REMOVE_V35_WRAP_PLUG_PORT_1);
                                da_display ();
                             }
                             break;

                }
                da_display ();
        }
        if (rc2 != NO_ERROR)            /* Set the return code  */
        {
                return (rc2);
        }
        else if (rc != QUIT)
        {
                return (0);
        }
        else
                return (rc);
}
/*--------------------------------------------------------------------------
| NAME: test_232
|
| FUNCTION: test 232 port
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               0 :     GOOD
|               9999 :  QUIT
|               Other : BAD
----------------------------------------------------------------------------*/
int test_232(int which_port, int cable_tested)
{
        int     rc = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     rc2 = NO_ERROR;

        if ((!cable_isolate_test) && (!wrap_plug_232))
        {
                /* ask the user if he has 232 wrap plug         */
                rc1 = display_generic_wrap_plug (WRAP_PLUG_232);
                if (rc1 == YES)
                        wrap_plug_232 = TRUE;
        }
        else    /* Running Isolate cable test.          */
                rc1 = YES;
        if (rc1 != YES)
                        return (0);
        else
        {
                switch (which_port)
                {
                        case 0:
                                if ((!cable_isolate_test)&& (cable_tested))
                                     rc = display (INSERT_232_CABLE_PORT_0);
                                else if (!cable_isolate_test)
                                     rc = display (INSERT_232_WRAP_PLUG_PORT_0);
                                da_display ();
                                if (rc != QUIT)
                                {

                                     /* running the test */
                                     artic_tucb.header.tu = 42;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR) && (!cable_tested))
                                     {
                                        if (verify_rc (&tu42[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_0_232_tested= TRUE;

                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        /* purpose is not to display */
                                        rc = rc2;
                                     }
                                     else if ((!cable_isolate_test)&& (cable_tested))
                                         rc = display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0);
                                     else if (!cable_isolate_test)
                                         rc = display(REMOVE_232_WRAP_PLUG_PORT_0);
                                     da_display ();
                                }
                                break;
                        case 1:
                                if ((!cable_isolate_test)&& (cable_tested))
                                     rc = display (INSERT_232_CABLE_PORT_1);
                                else if (!cable_isolate_test)
                                     rc = display (INSERT_232_WRAP_PLUG_PORT_1);
                                da_display ();
                                if (rc != QUIT)
                                {

                                     /* running the test        */
                                     artic_tucb.header.tu = 46;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR) && (!cable_tested))
                                     {
                                        if (verify_rc (&tu46[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_1_232_tested= TRUE;

                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        rc = rc2;
                                     }

                                     else if ((!cable_isolate_test)&& (cable_tested))
                                      rc = display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_1);
                                     else if (!cable_isolate_test)
                                      rc = display(REMOVE_232_WRAP_PLUG_PORT_1);
                                     da_display ();
                                }
                                break;
                        case 2:
                                if ((!cable_isolate_test)&& (cable_tested))
                                     rc = display (INSERT_232_CABLE_PORT_2);
                                else if (!cable_isolate_test)
                                     rc = display (INSERT_232_WRAP_PLUG_PORT_2);
                                da_display ();
                                if (rc != QUIT)
                                {

                                     /* running the test        */
                                     artic_tucb.header.tu = 48;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR) && (!cable_tested))
                                     {
                                        if (verify_rc (&tu48[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_2_232_tested= TRUE;

                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        rc = rc2;
                                     }
                                     else if ((!cable_isolate_test)&& (cable_tested))
                                         rc = display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_2);
                                     else if (!cable_isolate_test)
                                         rc = display(REMOVE_232_WRAP_PLUG_PORT_2);
                                     da_display ();
                                }
                                break;

                        case 3:
                                if ((!cable_isolate_test)&& (cable_tested))
                                     rc = display (INSERT_232_CABLE_PORT_3);
                                else if (!cable_isolate_test)
                                     rc = display (INSERT_232_WRAP_PLUG_PORT_3);
                                da_display ();
                                if (rc != QUIT)
                                {

                                     /* running the test        */
                                     artic_tucb.header.tu = 50;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR) && (!cable_tested))
                                     {
                                        if (verify_rc (&tu50[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_3_232_tested= TRUE;

                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        rc = rc2;
                                     }
                                     else if ((!cable_isolate_test)&& (cable_tested))
                                        rc = display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_3);
                                     else if (!cable_isolate_test)
                                        rc = display (REMOVE_232_WRAP_PLUG_PORT_3);
                                     da_display ();
                                }
                                break;

                }
                da_display ();
        }
        if (rc2 != NO_ERROR)            /* Set the return code  */
        {
                return (rc2);
        }
        else if (rc != QUIT)
        {
                return (0);
        }
        else
                return (rc);
}
/*--------------------------------------------------------------------------
| NAME: test_422
|
| FUNCTION: test 422 port
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               0 :     GOOD
|               9999 :  QUIT
|               Other : BAD
----------------------------------------------------------------------------*/
int test_422(int which_port, int cable_tested)
{
        int     rc  = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     rc2 = NO_ERROR;

        if ((!cable_isolate_test) && (!wrap_plug_422))
        {
                /* ask the user if he has 422 wrap plug         */
                rc1 = display_generic_wrap_plug (WRAP_PLUG_422);
                if (rc1 == YES)
                        wrap_plug_422 = TRUE;
        }
        else
                rc1 = YES;
        if (rc1 != YES)
                return (0);
        else
        {
                switch (which_port)
                {
                        case 0:
                                if ((!cable_isolate_test)&& (cable_tested))
                                    rc = display (INSERT_422_CABLE_PORT_0);
                                else if (!cable_isolate_test)
                                    rc = display (INSERT_422_WRAP_PLUG_PORT_0);
                                da_display ();
                                if (rc != QUIT)
                                {
                                     /* running the test        */
                                     artic_tucb.header.tu = 43;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR)  && (!cable_tested))
                                     {
                                        if (verify_rc (&tu43[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_0_422_tested = TRUE;

                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        rc = rc2;
                                     }
                                     else if ((!cable_isolate_test)&&
                                                (cable_tested))
                                       rc = display (REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0);
                                     else if (!cable_isolate_test )
                                        rc = display (REMOVE_422_WRAP_PLUG_PORT_0);
                                     da_display ();
                                }
                                break;
                        case 2:
                                if ((!cable_isolate_test)&& (cable_tested))
                                    rc = display (INSERT_422_CABLE_PORT_2);
                                else if (!cable_isolate_test)
                                    rc = display (INSERT_422_WRAP_PLUG_PORT_2);
                                da_display ();
                                if (rc != QUIT)
                                {
                                     /* running the test        */
                                     artic_tucb.header.tu = 49;
                                     rc2 = exectu(filedes, &artic_tucb);
                                     if ((rc2 != NO_ERROR)  && (!cable_tested))
                                     {
                                        if (verify_rc (&tu49[0],rc2))
                                                report_frub_mpqp ();

                                     }
                                     else if (rc2 == NO_ERROR)
                                        port_2_422_tested = TRUE;
                                     if ((rc2 != NO_ERROR) && (cable_tested))
                                     {
                                        rc = rc2;
                                     }
                                     else if ((!cable_isolate_test)&&
                                                (cable_tested))
                                       rc = display (REMOVE_WRAP_PLUG_FROM_CABLE_PORT_2);
                                     else if (!cable_isolate_test )
                                        rc = display (REMOVE_422_WRAP_PLUG_PORT_2);
                                }
                                da_display ();
                                break;

                }
                da_display ();
        }
        if (rc2 != NO_ERROR)            /* Set the return code  */
        {
                return (rc2);
        }
        else if (rc != QUIT)
        {
                return (0);
        }
        else
                return (rc);
}
/*--------------------------------------------------------------------------
| NAME: test_X21
|
| FUNCTION: test X21 port
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
|               0 :     GOOD
|               9999 :  QUIT
|               Other : BAD
----------------------------------------------------------------------------*/
int test_X21(int cable_tested)
{
        int     rc = NO_ERROR;
        int     rc1= NO_ERROR;
        int     rc2= NO_ERROR;

        /* ask the user if he has X21 wrap plug         */
        if (!cable_isolate_test)
        {
                /* ask the user if he has 232 wrap plug         */
                rc1 = display_generic_wrap_plug (WRAP_PLUG_X21);
        }
        else    /* Running Isolate cable test.          */
                rc1 = YES;
        if (rc1 != YES)
                return (0);
        else
        {
                if ((!cable_isolate_test)&& (cable_tested))
                        rc = display (INSERT_X21_CABLE_PORT_0);
                else if (!cable_isolate_test)
                        rc = display (INSERT_X21_WRAP_PLUG_PORT_0);
                da_display ();
                if (rc != QUIT)
                {
                        artic_tucb.header.tu = 44;
                        rc2 = exectu(filedes, &artic_tucb);
                        if ((rc2 != NO_ERROR) && (!cable_tested))
                        {
                                if (verify_rc (&tu44[0],rc2))
                                         report_frub_mpqp ();
                        }
                        else if (rc2 == NO_ERROR)
                                port_0_X21_tested = TRUE;

                        if ((rc2 != NO_ERROR) && (cable_tested))
                        {
                                /* for intension of not display remove wrap */
                                /* it is done in cable_test_portX           */
                                rc = rc2;
                        }
                        else if ((!cable_isolate_test) &&(cable_tested))
                                rc =display(REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0);
                        else if (!cable_isolate_test)
                                rc = display (REMOVE_X21_WRAP_PLUG_PORT_0);
                }
                da_display ();
        }
        if (rc2 != NO_ERROR)            /* Set the return code  */
        {
                return (rc2);
        }
        else if (rc != QUIT)
        {
                return (0);
        }
        else
                return (rc);
}


/*----------------------------------------------------------------------------*/
/*      Function :      which_port                                            */
/*      Description:    This function will calculate the real port            */
/*                      for example mpq4 is port 0 and mpq7 is port 3         */
/*      Return           rc  : the port which is tested                       */
/*                      -1 : bad                                              */
/*----------------------------------------------------------------------------*/

int     which_port ()
{
        char    tmp[64];
        int     rc, rc2;

        if (strstr (da_input.child1, "mpq"))
        {
                strcpy (tmp, da_input.child1);
                tmp[5] = '\0';
                rc = atoi(&tmp[3]);     /* rc will be port number */
                rc2 = rc % 4;
                return (rc2);

        }
        else
                return (NOT_GOOD);
}

/*----------------------------------------------------------------------------*/
/*      Function :     cable_test_port0                                       */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int     cable_test_port0(int which_menu)
{

        int     rc = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     cable_tested = TRUE;

        switch (which_menu)
        {
                case 1: /* V.35 cable testing */

                rc1 = test_V35 (0, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail.  Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_0_V35);
                        rc1= test_V35 (0,cable_tested);
                        rc = display(REMOVE_V35_WRAP_PLUG_PORT_0);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_V35_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                case 2: /* RS 232 Cable testing */
                rc1 = test_232 (0, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail. Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_0_232);
                        rc1= test_232 (0,cable_tested);
                        rc = display(REMOVE_232_WRAP_PLUG_PORT_0);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_232_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                case 3: /* X.21  cable testing */
                rc1 = test_X21 (cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail. Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_0_X21);
                        rc1= test_X21 (cable_tested);
                        rc = display(REMOVE_X21_WRAP_PLUG_PORT_0);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_X21_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                case 4: /* R2 422 cable testing */
                rc1 = test_422 (0, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail.  Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_0_422);
                        rc1= test_422 (0,cable_tested);
                        rc = display(REMOVE_422_WRAP_PLUG_PORT_0);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_422_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                default:
                break;
        }
        return (rc);
}

/*----------------------------------------------------------------------------*/
/*      Function :     cable_test_port1                                       */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int     cable_test_port1(int which_menu)
{
        int     rc = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     cable_tested= TRUE;

        switch (which_menu)
        {
                case 1: /* 232 cable testing */
                rc1 = test_232 (1, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail. Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_1_232);
                        rc1= test_232 (1,cable_tested);
                        rc = display(REMOVE_232_WRAP_PLUG_PORT_1);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                        /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_232_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                case 2: /* V35 Cable testing */

                rc1 = test_V35 (1, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail.      */
                        /* Test port 1 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_1_V35);
                        rc1= test_V35 (1,cable_tested);
                        rc = display(REMOVE_V35_WRAP_PLUG_PORT_1);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_V35_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                default:
                        break;
        }
        return (rc);
}


/*----------------------------------------------------------------------------*/
/*      Function :     cable_test_port2                                       */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int     cable_test_port2(int which_menu)
{
        int     rc = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     cable_tested=TRUE;

        switch (which_menu)
        {
                case 1: /* 232 cable testing */
                rc1 = test_232 (2, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail. Test port 0 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_2_232);
                        rc1= test_232 (2,cable_tested);
                        rc = display(REMOVE_232_WRAP_PLUG_PORT_2);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_232_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                case 2: /* 422 Cable testing */
                rc1 = test_422 (2, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail          */
                        /* Test port 2 without cable    */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_2_422);
                        rc1= test_422 (2,cable_tested);
                        rc = display(REMOVE_422_WRAP_PLUG_PORT_2);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_422_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
                break;

                default:
                        break;
        }
        return (rc);
}

/*----------------------------------------------------------------------------*/
/*      Function :     cable_test_port3                                       */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int     cable_test_port3()
{
        int     rc = NO_ERROR;
        int     rc1 = NO_ERROR;
        int     cable_tested = TRUE;
        {
                rc1 = test_232 (3, cable_tested);
                if ((rc1 != QUIT) && (rc1 != NO_ERROR))
                {
                        /* The cable test fail. Test port 3 without cable */

                        cable_isolate_test = TRUE;
                        rc=display (CABLE_ISOLATE_PORT_3_232);
                        rc1= test_232 (3,cable_tested);
                        rc = display(REMOVE_232_WRAP_PLUG_PORT_3);
                        if ((rc != QUIT) &&( rc1 == NO_ERROR))
                        {
                                /* Issuing a FRU cable bad */
                                report_frub_mpqp_cable (CABLE_232_BAD);
                                rc = BAD_CABLE_RC;
                        }
                        else if ((rc != QUIT) &&( rc1 != NO_ERROR))
                        {
                                report_frub_mpqp();
                        }
                }
        }
        return (rc);
}
/*--------------------------------------------------------------------------
| NAME: report_frub_mpqp_cable
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

void report_frub_mpqp_cable (int cable_type)
{

        struct fru_bucket *frub_addr;
        switch (cable_type)
        {
                case    CABLE_V35_BAD:
                        frub_addr = &mpqp_cable_frus[0];
                        break;
                case    CABLE_X21_BAD:
                        frub_addr = &mpqp_cable_frus[1];
                        break;
                case    CABLE_232_BAD:
                        frub_addr = &mpqp_cable_frus[2];
                        break;
                case    CABLE_422_BAD:
                        frub_addr = &mpqp_cable_frus[3];
                        break;
                default:
                        break;
        }

        strcpy ( frub_addr->dname, da_input.dname);
        frub_addr->sn = 0x855;

        insert_frub(&da_input, frub_addr);
        /* the reason why there is another switch because insert_frub   */
        /* will reset frub_addr->sn                                     */

        frub_addr->sn = 0x855;
        addfrub(frub_addr);
        DA_SETRC_STATUS (DA_STATUS_BAD);
}

/*--------------------------------------------------------------------------
| NAME: report_frub_mpqp
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

void report_frub_mpqp ()
{
        struct fru_bucket portmaster_interface_box_frus[] =
        {
                {"", FRUB1, 0x855, 0x720, PORT_WRAP_TEST_FAILURE,
                        {
                                {90, " ", "", MULTIPORT_INTERFACE_BOX,
                                        NOT_IN_DB, EXEMPT},
                                {10, "", "", 0, DA_NAME, EXEMPT},

                        },
                }
        };

        struct fru_bucket *frub_addr;

        frub_addr = &portmaster_interface_box_frus[0];

        strcpy ( frub_addr->dname, da_input.dname);
        frub_addr->sn = 0x855;

        insert_frub(&da_input, frub_addr);
        /* the reason why there is another switch because insert_frub   */
        /* will reset frub_addr->sn                                     */

        frub_addr->sn = 0x855;
        addfrub(frub_addr);
        DA_SETRC_STATUS (DA_STATUS_BAD);
}

