static char sccsid[] = "@(#)72  1.3  src/bos/diag/da/artic/dx25.c, daartic, bos41J, 9511A_all 3/7/95 15:17:09";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: check_rc
 *              dtoh
 *              ela
 *              interface_test
 *              no_plug
 *              pre_clean_up
 *              set_interface
 *              tu_test
 *              x25_advanced_tests
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
#include <nl_types.h>
#include <fcntl.h>
#include <limits.h>
#include <locale.h>
#include <time.h>
#include <sys/types.h>
#include <sys/devinfo.h>
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <sys/stat.h>
#include <errno.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "dartic_msg.h"
#include "dx25_msg.h"
#include "dc1x_msg.h"
#include "artictst.h"
#include "dx25.h"

extern  int     isabus;
/*******************************************************/

extern clean_up();
FILE   *trf;


x25_advanced_tests()
{
    int             i;


      /* running under console */
        if (da_input.console == CONSOLE_TRUE)
        {
                /* open X25 message catalog       */

                if (isabus == FALSE)
                   x25_catd = diag_catopen ("dx25.cat",0);  /* C2X */
                else
                   x25_catd = diag_catopen ("dc1x.cat",0);  /* C1X */

                if (x25_catd == CATD_ERR)
                        pre_clean_up();
        }


    if (da_input.dmode != DMODE_ELA) {
        if (da_input.loopmode == LOOPMODE_NOTLM) {    /* regular mode */
                da_display ();
            if (da_input.console == CONSOLE_TRUE) {

                if (da_input.advanced == ADVANCED_TRUE) {/* Execute TU10 ? */
                    rc = diag_display(0x849003, x25_catd, plug_37, DIAG_IO,
                                      ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                      &menutype,q_plug_37);
                    check_rc(rc);
                    if (rc == DIAG_ASL_COMMIT)
                        switch (DIAG_ITEM_SELECTED(menutype)) {
                        case 1:
                            strcpy(slot,da_input.dnameloc);
                            rc = diag_msg(0x849004, x25_catd, PLUG_37_PIN,
                                          PLUG_37_PIN_TITLE, slot);
                            check_rc(rc);
                            current_plug = 37;
                            set_interface();
                            tu_test (10);
                            interface_test();
                            da_display ();
                            rc = diag_msg(0x849005, x25_catd, UNPLUG_37_PIN,
                                          UNPLUG_37_PIN_TITLE);
                            check_rc(rc);
                            da_display ();
                            break;
                        case 2:
                            set_interface();
                            break;
                        default:
                            DA_SETRC_ERROR(DA_ERROR_OTHER);
                            pre_clean_up();
                            break;
                        } /* end switch - end of TU10 */

                        switch (interface)
                        {
                        case 21:    /* x21 interface */
                            rc = diag_display(0x849008,x25_catd,intx21_or_not,
                                              DIAG_IO,
                                              ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                              &menutype, q_intx21_or_not);
                            check_rc(rc);
                            if (rc == DIAG_ASL_COMMIT)
                                switch (DIAG_ITEM_SELECTED(menutype)) {
                                case 1:
                                    rc = diag_msg(0x849009,x25_catd,
                                                  PUT_INTX21_PLUG,
                                                  PUT_INTX21_PLUG_TITLE);
                                    check_rc(rc);
                                    da_display ();
                                    current_plug = 21;
                                    tu_test(14);
                                    rc = diag_msg(0x849010, x25_catd, RESTORE,
                                                  RESTORE_TITLE);
                                    check_rc(rc);
                                    break;
                                case 2:
                                    break;
                                default:
                                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                                    pre_clean_up();
                                    break;
                                }
                            break;

                        case 24:    /* v24 interface */
                            rc = diag_display(0x849011,x25_catd,intv24_or_not,
                                              DIAG_IO,
                                              ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                              &menutype, q_intv24_or_not);
                            check_rc(rc);
                            if (rc == DIAG_ASL_COMMIT)
                                switch (DIAG_ITEM_SELECTED(menutype)) {
                                case 1:
                                    rc = diag_msg(0x849012,x25_catd,
                                                  PUT_INTV24_PLUG,
                                                  PUT_INTV24_PLUG_TITLE);
                                    check_rc(rc);
                                    da_display ();
                                    current_plug = 24;
                                    tu_test(15);
                                    rc = diag_msg(0x849010, x25_catd, RESTORE,
                                                  RESTORE_TITLE);
                                    check_rc(rc);
                                    break;
                                case 2:
                                    break;
                                default:
                                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                                    pre_clean_up();
                                    break;
                                }
                            break;

                        case 35:    /* v35 interface */
                            rc = diag_display(0x849013,x25_catd,intv35_or_not,
                                              DIAG_IO,
                                              ASL_DIAG_LIST_CANCEL_EXIT_SC,
                                              &menutype, q_intv35_or_not);
                            check_rc(rc);
                            if (rc == DIAG_ASL_COMMIT)
                                switch (DIAG_ITEM_SELECTED(menutype)) {
                                case 1:
                                    rc = diag_msg(0x849014,x25_catd,
                                                  PUT_INTV35_PLUG,
                                                  PUT_INTV35_PLUG_TITLE);
                                    check_rc(rc);
                                    da_display ();
                                    current_plug = 35;
                                    tu_test(16);
                                    rc = diag_msg(0x849010, x25_catd, RESTORE,
                                                  RESTORE_TITLE);
                                    check_rc(rc);
                                    da_display ();
                                    break;
                                case 2:
                                    break;
                                default:
                                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                                    pre_clean_up();
                                    break;
                                }
                            break;
                        } /* end switch */
                } /* end if - end of ADVANCED tests */
            }
        } else {                       /* end regular mode - start loop mode */
            switch (da_input.loopmode) {
            case LOOPMODE_ENTERLM:
                da_display ();
                /* Check if TU10 can be executed */
                rc = diag_display(0x849003, x25_catd, plug_37, DIAG_IO,
                                  ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutype,
                                  q_plug_37);
                check_rc(rc);
                if (rc == DIAG_ASL_COMMIT)
                    switch (DIAG_ITEM_SELECTED(menutype)) {
                    case 1:
                        strcpy(slot,da_input.dnameloc);
                        rc = diag_msg(0x849004, x25_catd, PLUG_37_PIN,
                                      PLUG_37_PIN_TITLE, slot);
                        check_rc(rc);
                        current_plug = 37;
                        putdavar(da_input.dname,"plug_name", DIAG_INT,
                                 &current_plug);
                        sleep(2);
                        set_interface();
                        interface_test();
                        da_display ();
                        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
                        check_rc(rc);
                        break;

                    case 2: /* no 37 pin wrap plug */
                        set_interface();
                        break;
                    default:
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        pre_clean_up();
                        break;
                    } /* end switch */
                ela();
                break;

            case LOOPMODE_INLM:
                da_display ();
                getdavar(da_input.dname, "plug_name", DIAG_INT, &current_plug);
                getdavar(da_input.dname, "which_interface", DIAG_INT,
                                &interface);
                for (i = 0; i < 9; ++i)
                        tu_test(reg_tu_seq[i]);
                if (current_plug == 37)
                {
                        tu_test (10);
                        switch (interface)
                        {
                                case 21 : tu_test (11);
                                        break;
                                case 24 : tu_test (12);
                                        break;
                                case 35 : tu_test (13);
                                        break;
                        }
                } /* end if have plug */

                check_rc (diag_asl_read
                        (ASL_DIAG_LIST_CANCEL_EXIT_SC, FALSE, NULL));

                break;

            case LOOPMODE_EXITLM:
                getdavar(da_input.dname, "plug_name", DIAG_INT, &current_plug);
                getdavar(da_input.dname, "which_interface", DIAG_INT,
                                &interface);

                if (current_plug == 37)
                {
                    rc = diag_msg(0x849005, x25_catd, UNPLUG_37_PIN,
                              UNPLUG_37_PIN_TITLE);
                    check_rc(rc);
                }
                else  if (interface != 0)
                {
                    rc = diag_msg(0x849010, x25_catd, RESTORE, RESTORE_TITLE);
                    check_rc(rc);
                }

            default:
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
                break;
            } /* end switch - loop mode */
        } /* end if-else - loop mode */
    } /* end if ! ELA */

    /* Performing error log analysis */
    if (((da_input.dmode == DMODE_PD) || (da_input.dmode == DMODE_ELA))
       && (da_input.loopmode == LOOPMODE_NOTLM))
        ela();

    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_TESTS(DA_TEST_FULL);
    pre_clean_up();

} /* end main */
/*------------------------------------------------------------*/

set_interface()
{
        int item;

        strcpy(slot,da_input.dnameloc);
        rc = diag_display(NULL,x25_catd,which_int,DIAG_MSGONLY,
                          NULL, &menutype, q_which_int);
        sprintf(temp_str, q_which_int[0].text, slot);
        free(q_which_int[0].text);
        q_which_int[0].text = temp_str;
        rc = diag_display(0x849007, x25_catd, NULL, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutype, q_which_int);
        check_rc(rc);

        if (rc == DIAG_ASL_COMMIT)
        {
                item = DIAG_ITEM_SELECTED(menutype);
                switch(item)
                {
                        case 1:  interface = 21;
                                 break;
                        case 2:  interface = 24;
                                 break;
                        case 3:  interface = 35;
                                 break;
                        case 4:  interface = 0;
                                 break;
                        default:
                                 DA_SETRC_ERROR(DA_ERROR_OTHER);
                                 pre_clean_up();
                                 break;
                }
        } /* end if */

        putdavar(da_input.dname,"which_interface", DIAG_INT,
                                 &interface);

}  /*  end set_interface */
/*_________________________________________________________*/

interface_test()
{
        switch (interface)
        {
                case 21:  tu_test (11);
                          break;
                case 24:  tu_test (12);
                          break;
                case 35:  tu_test (13);
                          break;
        }
}

/*_________________________________________________________*/

no_plug ()
{
        rc = diag_msg_nw(0x849006,x25_catd,NO_PLUG,NO_PLUG_TITLE);
        sleep(4);
        check_rc(rc);
}
/*____________________________________________________________*/

pre_clean_up ()
{
        fflush(trf);
        fclose(trf);

        if (da_input.console == CONSOLE_TRUE)
                catclose(x25_catd);

        clean_up();

}
/*_____________________________________________________________*/

/*
* NAME: tu_test
*
* FUNCTION: Executing test units and report fru(s) to the controller if a
*           failure is found.
*
* EXECUTION ENVIRONMENT:
*
*       Called by the main program to execute test units.
*       Call external routine exectu to actually execute the test units.
*       Call external routine diag_asl_read to get user's input to screen.
*       Call check_rc to check if user has entered the Esc or Cancel key.
*       Call external routines insert_frub and addfrub when a failure is found.
*       Call clean_up after a fru is reported to the controller.
*
* RETURNS: NONE
*/

tu_test (int testptr)
{
    int             turc;               /* return code from test unit */

        artic_tucb.header.tu = testptr;
        turc = exectu(filedes, &artic_tucb);

    if (IS_CONSOLE) {
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }

    if (turc != 0) {
        switch (testptr) {
        case 1:
            if (turc < 0x13) {
                rc = insert_frub(&da_input, &frub[0]);
                if (rc != 0) {
                    DA_SETRC_STATUS(DA_STATUS_BAD);
                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                    pre_clean_up();
                }
                strncpy(frub[0].dname,da_input.dname, sizeof(frub[0].dname));
                addfrub(&frub[0]);
            } else {
                frub[1].rcode = 0x211;
                rc = insert_frub(&da_input, &frub[1]);
                if (rc != 0) {
                    DA_SETRC_STATUS(DA_STATUS_BAD);
                    DA_SETRC_ERROR(DA_ERROR_OTHER);
                    pre_clean_up();
                }
                strncpy(frub[1].dname,da_input.dname, sizeof(frub[1].dname));
                addfrub(&frub[1]);
            }
            break;
        case 3:
        case 9:
        case 10:
        case 18:
        case 19:
            if (testptr < 10) {
                rcode_val = 200 + testptr * 10;
                rcode_val = dtoh(rcode_val);
            }
            else {
                rcode_val = testptr * 10;
                rcode_val = dtoh(rcode_val);
            }
            if (testptr == 10)
                rcode_val = 0x101;
            frub[2].rcode = rcode_val;
            rc = insert_frub(&da_input, &frub[2]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[2].dname,da_input.dname, sizeof(frub[2].dname));
            addfrub(&frub[2]);
            break;
        case 4:
        case 6:
        case 7:
        case 8:
            rcode_val = 200 + testptr * 10;
            rcode_val = dtoh(rcode_val);
            frub[2].rcode = rcode_val;
            rc = insert_frub(&da_input, &frub[2]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[2].dname,da_input.dname, sizeof(frub[2].dname));
            addfrub(&frub[2]);
            break;
        case 5:
            rc = insert_frub(&da_input, &frub[3]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[3].dname,da_input.dname, sizeof(frub[3].dname));
            addfrub(&frub[3]);
            break;
        case 14:
            rc = insert_frub(&da_input, &frub[4]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[4].dname,da_input.dname, sizeof(frub[4].dname));
            addfrub(&frub[4]);
            break;
        case 15:
            rc = insert_frub(&da_input, &frub[5]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[5].dname,da_input.dname, sizeof(frub[5].dname));
            addfrub(&frub[5]);
            break;
        case 16:
            rc = insert_frub(&da_input, &frub[6]);
            if (rc != 0) {
                DA_SETRC_STATUS(DA_STATUS_BAD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            strncpy(frub[6].dname,da_input.dname, sizeof(frub[6].dname));
            addfrub(&frub[6]);
            break;
        default:
            DA_SETRC_ERROR(DA_ERROR_OTHER);
            pre_clean_up();
            break;
        } /* end switch */

        DA_SETRC_STATUS(DA_STATUS_BAD);
        DA_SETRC_MORE(DA_MORE_NOCONT);
        DA_SETRC_TESTS(DA_TEST_FULL);
        pre_clean_up();
    } /* end if */
} /* end tu_test */

/*
* NAME: check_rc
*
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*
* EXECUTION ENVIRONMENT:
*       Called by main program and some other routines.
*
* RETURNS: rc, the input parameter
*/

int
check_rc(rc)
    int             rc;                         /* user's input */
{
    if (rc == DIAG_ASL_CANCEL) {
        DA_SETRC_USER(DA_USER_QUIT);
        DA_SETRC_TESTS(DA_TEST_FULL);
        pre_clean_up ();
    }
    if (rc == DIAG_ASL_EXIT) {
        DA_SETRC_USER(DA_USER_EXIT);
        DA_SETRC_TESTS(DA_TEST_FULL);
        pre_clean_up ();
    }
    return (rc);
} /* end check_rc */

/*
* NAME: dtoh
*
* FUNCTION: Converting a decimal number to hexadecimal
*
* EXECUTION ENVIRONMENT:
*       Called by main program and some other routines.
*
* RETURNS: Input number in hex.
*/

unsigned int
dtoh(n)
    unsigned int n;
{
    unsigned int        shift;
    unsigned int        h;

    shift = 0;
    h = 0;
    for ( ; n ; n /= 10, shift += 4)
    {
        h |= (n%10) << shift;
    }
    return(h);
}

/*
* NAME: ela
*
* FUNCTION: Performing error log analysis
*
* EXECUTION ENVIRONMENT:
*       Called by main program.
*
* RETURNS: None
*/

int
ela()
{
        char     crit[256];

        sprintf(crit, "-N %s %s", da_input.dname, da_input.date);
        rc = error_log_get(INIT, crit, &err_data);
        while (rc != 0) {
            if (rc == -1) {
                DA_SETRC_STATUS(DA_STATUS_GOOD);
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                pre_clean_up();
            }
            else if (rc > 0) {
                if ((err_data.err_id==0x64adc879)||
                    (err_data.err_id==0x74e0cea8)) {
                    rc = insert_frub(&da_input, &frub[7]);
                    if (rc != 0) {
                        DA_SETRC_STATUS(DA_STATUS_GOOD);
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        DA_SETRC_TESTS(DA_TEST_FULL);
                        pre_clean_up();
                    }
                    strncpy(frub[7].dname,da_input.dname,sizeof(frub[7].dname));
                    addfrub(&frub[7]);
                    DA_SETRC_STATUS(DA_STATUS_BAD);
                    pre_clean_up();
                }
            }
            rc = error_log_get(SUBSEQ, crit, &err_data);
        }
        rc = error_log_get(TERMI, crit, &err_data);
        if (rc == -1) {
            DA_SETRC_STATUS(DA_STATUS_GOOD);
            DA_SETRC_ERROR(DA_ERROR_OTHER);
            pre_clean_up();
        }
}
