static char sccsid[] = "@(#)71  1.2  src/bos/diag/da/artic/dt1e1j1.c, daartic, bos41J, 9511A_all 3/3/95 11:13:55";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: Having_cable_tested
 *              Having_wrap_plug
 *              t1e1j1_interactive_tests
 *              unplug_cable_and_put_wrap
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
#include "dartic_ext.h"
#include "dartic_msg.h"
#include "dmultp_msg.h"
#include "artictst.h"
#include <locale.h>

extern short    wrap_plug;
extern short    cable_tested;
extern  nl_catd diag_catopen();
extern  getdainput();           /* gets input from ODM                  */
extern  long getdamode();       /* transforms input into a mode         */
extern  int errno;              /* reason device driver won't open      */

extern  int     alarm_timeout;
extern struct  sigaction  invec;        /* interrupt handler structure  */

extern short    odm_flg;                /* return code from init_dgodm() */
extern short    asl_flg;        /* return code from diag_asl_init() */

extern TUTYPE   artic_tucb;
extern short    set_device_to_diagnose;
extern struct   tm_input da_input;
extern int      filedes;
extern nl_catd catd;
extern int      card_type;
extern int      EIB_type;
extern char     *new_out;
extern char      *new_err;
extern char     test_device[];
extern char     option[];

extern  exectu ();


int     t1e1j1_interactive_tests();
extern int      test_tu();
extern int      verify_rc();
extern int      poll_kb_for_quit();
int     Having_wrap_plug();
int     Having_cable_tested();
extern int      check_asl_stat(int);
extern void     intr_handler(int);
extern void     error_log_analysis ();
extern int      test_display();
extern void     da_display();
extern void     report_frub();
extern void     clean_up();
extern void     timeout(int);
extern void     clear_variables();
extern void    set_timer();
/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/

extern  ASL_SCR_TYPE    menutypes;
extern char *msgstr;
extern char *msgstr1;


/*--------------------------------------------------------------------------
|       NAME: t1e1j1_interactive_tests()
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
int t1e1j1_interactive_tests()
{
        int rc = NO_ERROR;

        getdavar(da_input.dname, "wrap_plug", DIAG_SHORT, &wrap_plug);
        getdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);

        /* running test 20      */
        set_timer ();
        artic_tucb.header.tu = 20;
        rc = exectu(filedes, &artic_tucb);
        if(((rc != 0) && (verify_rc ( &tu020[0],rc) == TRUE))
                        || (alarm_timeout))
        {
                /* report_frub (fru_index); */
                /* 851-110 or 851-410           */
                report_frub(FRU_120);
                DA_SETRC_STATUS (DA_STATUS_BAD);
                clean_up();
        }

        if (da_input.loopmode & LOOPMODE_INLM)

        {
                if ((wrap_plug) || ( cable_tested))
                {
                        set_timer ();
                        artic_tucb.header.tu = 21;
                        rc = exectu(filedes, &artic_tucb);

                        if(((rc != 0) && (verify_rc (&tu021[0],rc) == TRUE))
                                        || (alarm_timeout))
                        {
                                /* 100 % adapter        */
                                report_frub(FRU_121);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                                return (NOT_GOOD);
                        }
                }
                else
                {
                        /* no testing is done */
                        return(NO_ERROR);
                }
        }

        if ((da_input.loopmode & LOOPMODE_NOTLM) ||
                (da_input.loopmode & LOOPMODE_ENTERLM))
        {
                if      /* cable testing is desired     */
                   (Having_cable_tested() == YES)
                {
                        /* ask the user to put the cable in     */
                                test_display (PLUG_T1_CABLE);

                        artic_tucb.header.tu = 21;
                        da_display ();
                        set_timer ();
                        rc = exectu(filedes, &artic_tucb);
                        if(((rc != 0) && (verify_rc (&tu021[0],rc) == TRUE))
                                        || (alarm_timeout))
                        {
                                rc = unplug_cable_and_put_wrap ();

                        }
                }
                /* the user does not have the wrap plug. No wrap test   */
                /* is performed                                         */
                else if (Having_wrap_plug() != YES)
                {
                        wrap_plug = FALSE;
                        putdavar(da_input.dname, "wrap_plug",DIAG_SHORT,
                                &wrap_plug);
                        return (NO_ERROR);
                }
                else    /* the user has the wrap plug. Wrap test is run */
                {
                        /* ask the user to put the wrap plug in         */
                        test_display (PLUG_WRAP);
                        artic_tucb.header.tu = 21;
                        da_display ();
                        set_timer ();
                        rc = exectu(filedes, &artic_tucb);
                        if((alarm_timeout) || ((rc != 0) &&
                            (verify_rc ( &tu021[0],rc) == TRUE)))
                        {
                                report_frub (FRU_121);
                                DA_SETRC_STATUS (DA_STATUS_BAD);
                                return (NOT_GOOD);
                        }
                }
        }
        /* if running in LOOPMODE_EXITLM or NOTLM then  */
        /* check if the cable is hooked before running  */
        /* DA. If yes, then ask the user to put the     */
        /* cable back into the port.                    */

        getdavar(da_input.dname, "wrap_plug", DIAG_SHORT, &wrap_plug);
        getdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);


        if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
            ( da_input.loopmode & LOOPMODE_NOTLM )) && (wrap_plug)
                        && ( da_input.console == CONSOLE_TRUE))
        {
                /* Ask the user to unplug the wrap plug */
                test_display(UNPLUG_WRAP);
                da_display ();
        }
        else if ((( da_input.loopmode & LOOPMODE_EXITLM ) ||
            ( da_input.loopmode & LOOPMODE_NOTLM )) && (cable_tested)
                        && ( da_input.console == CONSOLE_TRUE))
        {
                /* Ask the user to unplug the cable*/
                test_display(UNPLUG_CABLE);
                da_display ();
        }
        return (rc);
}


/*--------------------------------------------------------------------------
 |      NAME: Having_wrap_plug
 |
 |      FUNCTION: Ensure user have the wrap plug before proceeding wrap plug
 |
 |      EXECUTION ENVIRONMENT:
 |
 |      NOTES:  Other local routines called --
 |              Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 |      RETURNS:
 |              YES --- Yes, user has wrap plug
 |              NO,QUIT --- otherwise
 ---------------------------------------------------------------------------*/

int Having_wrap_plug()
{
        int menu_rc;

        /* menu 0x851008  */
        menu_rc = test_display(HAVE_WRAP_PLUG);

    return(menu_rc);
}

/*--------------------------------------------------------------------------
 |      NAME: Having_cable_tested
 |
 |      FUNCTION: Ensure user have the wrap plug and cable before testing
 |
 |      EXECUTION ENVIRONMENT:
 |
 |      NOTES:  Other local routines called --
 |              Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 |      RETURNS:
 |              YES --- Yes, user chooses to test cable
 |              NO,QUIT --- otherwise
 ---------------------------------------------------------------------------*/

int Having_cable_tested()
{
        int rc;
        /* asking the user if he wants to test cable    */

                rc = diag_display(0x851004, catd,
                        testing_cable_t1_yes_no, DIAG_MSGONLY,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_testing_cable_t1_yes_no);
                sprintf(msgstr, asi_testing_cable_t1_yes_no[0].text,
                        da_input.dname, da_input.dnameloc);
                free (asi_testing_cable_t1_yes_no[0].text);
                asi_testing_cable_t1_yes_no[0].text = msgstr;


                /* set the cursor points to NO */
                menutypes.cur_index = NO;
                rc = diag_display(0x851004, catd, NULL, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_testing_cable_t1_yes_no);

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
        /* setting wrap_plug variable in data base to false     */
        wrap_plug = FALSE;
        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, &wrap_plug);

        /* setting cable_tested variable in data base to true   */
        cable_tested= TRUE;
        putdavar(da_input.dname, "cable_tested", DIAG_SHORT, &cable_tested);

        return (YES);
}


/*--------------------------------------------------------------------------
 |      NAME: unplug_cable_and_put_wrap
 |
 |      FUNCTION: This function will display the message asking the user
 |                to unplug the cable and put the wrap plug in.
 |                Do not need to ask the user if he does have the wrap
 |                because before this function the user is already informed
 |                about the wrap plug
 |
 |      EXECUTION ENVIRONMENT:
 |
 |      NOTES:  Other local routines called --
 |              Only executed in LOOPMODE_ENTERLM or LOOPMODE_NOTLM
 |
 |      RETURNS:
 |              NO_ERROR        0
 |              NOT_GOOD        -1
 ---------------------------------------------------------------------------*/

int unplug_cable_and_put_wrap ()
{
        int rc;

        /* ask the user to unplug cable and plug*/
        /* wrap plug in                         */
        test_display (UNPLUG_WRAP_AND_CABLE);
        da_display ();
        wrap_plug = TRUE;
        putdavar(da_input.dname, "wrap_plug",DIAG_SHORT, &wrap_plug);
        artic_tucb.header.tu = 21;
        set_timer ();
        rc = exectu(filedes, &artic_tucb);

        if(((rc != 0) && (verify_rc (&tu021[0],rc)
                                        == TRUE)) || (alarm_timeout))
        {
                /* report_frub (fru_index); */
                report_frub(FRU_121);
                DA_SETRC_STATUS (DA_STATUS_BAD);
                return (NOT_GOOD);
        }
        else if ( rc == 0)
        {
                /* Issue bad cable      */
                report_frub(FRU_112);
                DA_SETRC_STATUS (DA_STATUS_BAD);
        }
        return (rc);
} /* unplug_cable_and_put_wrap */
