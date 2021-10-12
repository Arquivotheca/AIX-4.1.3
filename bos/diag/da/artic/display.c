static char sccsid[] = "@(#)63  1.2  src/bos/diag/da/artic/display.c, daartic, bos41J, 9511A_all 3/3/95 11:13:45";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: display
 *              display_generic_wrap_plug2166
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
#include <fcntl.h>
#include <sys/signal.h>
#include <asl.h>
#include <nl_types.h>
#include <limits.h>
#include <errno.h>
#include <diag/tm_input.h>
#include <diag/tmdefs.h>
#include <diag/da.h>
#include <diag/diag_exit.h>
#include <diag/diag.h>
#include <diag/diago.h>
#include <diag/bit_def.h>
#include <locale.h>
#include "dmultp_msg.h"
#include "dartic.h"
#include "dartic_ext.h"
#include "display.h"

static ASL_SCR_TYPE     device_menutype;
extern nl_catd  catd;
extern nl_catd  mpqp_catd;
extern struct tm_input da_input;
extern short cable_tested;
extern short wrap_plug;
extern short wrap_plug_78_pin;
extern short wrap_plug_V35;
extern short wrap_plug_X21;
extern short wrap_plug_232;
extern short wrap_plug_422;

extern short port_0_V35_tested;
extern short port_0_X21_tested;
extern short port_0_232_tested;
extern short port_0_422_tested;

extern short port_1_V35_tested;
extern short port_1_232_tested;

extern short port_2_422_tested;
extern short port_2_232_tested;

extern short port_3_232_tested;

char    msgstr[512];
char    msgstr1[512];
char    msgstr2[512];
char    msgstr3[512];
char    msgstr4[512];
char    msgstr5[512];
char    msgstr6[512];
char    msgstr7[512];
char    msgstr8[512];
char    msgstr9[512];
char    msgstr10[512];

/*......................................................................*/
/*
 * NAME:        display
 *
 * FUNCTION: display the different menus.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS:
 */
/*......................................................................*/

display (msg_number)
int     msg_number;

{
        int     rc;
        int     input;
        char    buffer[80];
        int     return_code = NOT_GOOD;

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        switch (msg_number)
        {
                case DO_YOU_HAVE_78_PIN_WRAP_PLUG:

                {
                        rc = diag_display(0x855005, mpqp_catd,
                                do_you_have_78_pin_wrap_plug, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_do_you_have_78_pin_wrap_plug);
                        sprintf(msgstr,asi_do_you_have_78_pin_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_do_you_have_78_pin_wrap_plug[0].text);
                        asi_do_you_have_78_pin_wrap_plug[0].text = msgstr;


                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(0x855005, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_do_you_have_78_pin_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                return (QUIT);
                        }

                        rc = DIAG_ITEM_SELECTED (menutypes);
                        if      /* if user does not have the wrap NTF */
                                ( rc != YES)
                        {
                                rc = NO;
                        }
                        else
                                rc = YES;

                }
                break;

                case INSERT_78_PIN_WRAP_PLUG:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855006,
                                mpqp_catd, put_78_pin_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_78_pin_wrap_plug);
                        sprintf(msgstr, asi_put_78_pin_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_78_pin_wrap_plug[0].text);
                        asi_put_78_pin_wrap_plug[0].text = msgstr;
                        sprintf(msgstr1, asi_put_78_pin_wrap_plug[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_put_78_pin_wrap_plug[1].text);
                        asi_put_78_pin_wrap_plug[1].text = msgstr1;

                        rc = diag_display(0x855006, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_78_pin_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                return (QUIT);
                        }
                        wrap_plug_78_pin = TRUE;
                        putdavar(da_input.dname, "wrap_plug_78_pin",
                                DIAG_SHORT, &wrap_plug_78_pin);
                }
                break;
                case NO_PORT_TO_RUN:
                {
                        rc = diag_display(0x855007,
                                mpqp_catd, no_port_to_run,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_no_port_to_run);
                        sprintf(msgstr, asi_no_port_to_run[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_no_port_to_run[0].text);
                        asi_no_port_to_run[0].text = msgstr;
                        sprintf(msgstr1, asi_no_port_to_run[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_no_port_to_run[1].text);
                        asi_no_port_to_run[1].text = msgstr1;

                        rc = diag_display(0x855007, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_no_port_to_run);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }
                }
                break;

                case TESTING_INDIVIDUAL_PORT_CONNECTOR:
                {
                        rc = diag_display(0x855035, mpqp_catd,
                                individual_port_connector, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_individual_port_connector);
                        sprintf(msgstr,asi_individual_port_connector[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_individual_port_connector[0].text);
                        asi_individual_port_connector[0].text = msgstr;

                        sprintf(msgstr1, asi_individual_port_connector[3].text,
                                da_input.dname);
                        free (asi_individual_port_connector[3].text);
                        asi_individual_port_connector[3].text = msgstr1;



                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(0x855035, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_individual_port_connector);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                return (QUIT);
                        }

                        rc = DIAG_ITEM_SELECTED (menutypes);
                        if      /* if user does not have the wrap NTF */
                                ( rc != YES)
                        {
                                return (NO);
                        }
                        return (YES);


                }
                break;

                case REMOVE_78_PIN_WRAP_PLUG:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855007,
                                mpqp_catd, remove_78_pin_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_78_pin_wrap_plug);
                        sprintf(msgstr, asi_remove_78_pin_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_78_pin_wrap_plug[0].text);
                        asi_remove_78_pin_wrap_plug[0].text = msgstr;
                        sprintf(msgstr1, asi_remove_78_pin_wrap_plug[1].text,
                                da_input.dnameloc,da_input.dnameloc);
                        free (asi_remove_78_pin_wrap_plug[1].text);
                        asi_remove_78_pin_wrap_plug[1].text = msgstr1;

                        rc = diag_display(0x855007, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_78_pin_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                return (QUIT);
                        }
                        wrap_plug_78_pin = FALSE;
                        putdavar(da_input.dname, "wrap_plug_78_pin",
                                DIAG_SHORT, &wrap_plug_78_pin);
                }
                break;

                case INSERT_V35_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855037,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_0_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855037, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_V35_CABLE_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855011,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855011, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_V35_CABLE_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855015,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855015, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855037,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_0_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855037, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_CABLE_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855011,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855011, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_CABLE_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855015,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855015, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case INSERT_232_CABLE_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855020,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855020, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_CABLE_PORT_3:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855030,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232) );
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855030, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case INSERT_X21_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855037,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_0_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_X21),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855037, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case INSERT_X21_CABLE_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855011,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_X21));
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855011, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case INSERT_422_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855037,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_0_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855037, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case INSERT_422_CABLE_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855011,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422));
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855011, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_422_CABLE_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855020,
                                mpqp_catd, put_generic_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_cable);
                        sprintf(msgstr, asi_put_generic_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_cable[0].text);
                        asi_put_generic_cable[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_cable[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422));
                        free (asi_put_generic_cable[1].text);
                        asi_put_generic_cable[1].text = msgstr1;


                        rc = diag_display(0x855020, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_V35_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855038,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_0));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855038, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_WRAP_PLUG_FROM_CABLE_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855012,
                                mpqp_catd, remove_generic_wrap_plug_from_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);
                        sprintf(msgstr,
                                asi_remove_generic_wrap_plug_from_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug_from_cable[0].text);
                        asi_remove_generic_wrap_plug_from_cable[0].text= msgstr;

                        sprintf(msgstr1,
                             asi_remove_generic_wrap_plug_from_cable[1].text);
                        free (asi_remove_generic_wrap_plug_from_cable[1].text);
                        asi_remove_generic_wrap_plug_from_cable[1].text=msgstr1;

                        sprintf(msgstr2,
                                asi_remove_generic_wrap_plug_from_cable[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_0));

                        free (asi_remove_generic_wrap_plug_from_cable[2].text);
                        asi_remove_generic_wrap_plug_from_cable[2].text=msgstr2;

                        rc = diag_display(0x855012, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_WRAP_PLUG_FROM_CABLE_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855016,
                                mpqp_catd, remove_generic_wrap_plug_from_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);
                        sprintf(msgstr,
                                asi_remove_generic_wrap_plug_from_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug_from_cable[0].text);
                        asi_remove_generic_wrap_plug_from_cable[0].text= msgstr;

                        sprintf(msgstr1,
                             asi_remove_generic_wrap_plug_from_cable[1].text);
                        free (asi_remove_generic_wrap_plug_from_cable[1].text);
                        asi_remove_generic_wrap_plug_from_cable[1].text=msgstr1;

                        sprintf(msgstr2,
                                asi_remove_generic_wrap_plug_from_cable[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_1));

                        free (asi_remove_generic_wrap_plug_from_cable[2].text);
                        asi_remove_generic_wrap_plug_from_cable[2].text=msgstr2;

                        rc = diag_display(0x855016, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_WRAP_PLUG_FROM_CABLE_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855021,
                                mpqp_catd, remove_generic_wrap_plug_from_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);
                        sprintf(msgstr,
                                asi_remove_generic_wrap_plug_from_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug_from_cable[0].text);
                        asi_remove_generic_wrap_plug_from_cable[0].text= msgstr;

                        sprintf(msgstr1,
                             asi_remove_generic_wrap_plug_from_cable[1].text);
                        free (asi_remove_generic_wrap_plug_from_cable[1].text);
                        asi_remove_generic_wrap_plug_from_cable[1].text=msgstr1;

                        sprintf(msgstr2,
                                asi_remove_generic_wrap_plug_from_cable[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_2));

                        free (asi_remove_generic_wrap_plug_from_cable[2].text);
                        asi_remove_generic_wrap_plug_from_cable[2].text=msgstr2;

                        rc = diag_display(0x855021, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;


                case REMOVE_WRAP_PLUG_FROM_CABLE_PORT_3:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855031,
                                mpqp_catd, remove_generic_wrap_plug_from_cable,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);
                        sprintf(msgstr,
                                asi_remove_generic_wrap_plug_from_cable[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug_from_cable[0].text);
                        asi_remove_generic_wrap_plug_from_cable[0].text= msgstr;

                        sprintf(msgstr1,
                             asi_remove_generic_wrap_plug_from_cable[1].text);
                        free (asi_remove_generic_wrap_plug_from_cable[1].text);
                        asi_remove_generic_wrap_plug_from_cable[1].text=msgstr1;

                        sprintf(msgstr2,
                                asi_remove_generic_wrap_plug_from_cable[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_3));

                        free (asi_remove_generic_wrap_plug_from_cable[2].text);
                        asi_remove_generic_wrap_plug_from_cable[2].text=msgstr2;

                        rc = diag_display(0x855031, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_from_cable);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_232_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855038,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_0));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855038, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_X21_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855038,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_0));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855038, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_422_WRAP_PLUG_PORT_0:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855038,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_0));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855038, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_V35_WRAP_PLUG_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855040,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_1_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855040, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case REMOVE_V35_WRAP_PLUG_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855041,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_1));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855041, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_232_WRAP_PLUG_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855041,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_1));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855041, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_WRAP_PLUG_PORT_1:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855040,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_1_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855040, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_WRAP_PLUG_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855045,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_2_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855045, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_232_WRAP_PLUG_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855046,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_2));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855046, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_422_WRAP_PLUG_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855045,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_2_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855045, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_422_WRAP_PLUG_PORT_2:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855046,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_2));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855046, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case INSERT_232_WRAP_PLUG_PORT_3:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855050,
                                mpqp_catd, put_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_put_generic_wrap_plug);
                        sprintf(msgstr, asi_put_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_put_generic_wrap_plug[0].text);
                        asi_put_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_put_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                REMOVE_PORT_3_CABLE));
                        free (asi_put_generic_wrap_plug[1].text);
                        asi_put_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_put_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232));

                        free (asi_put_generic_wrap_plug[2].text);
                        asi_put_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855050, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_put_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case REMOVE_232_WRAP_PLUG_PORT_3:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855051,
                                mpqp_catd, remove_generic_wrap_plug,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_remove_generic_wrap_plug);
                        sprintf(msgstr, asi_remove_generic_wrap_plug[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_remove_generic_wrap_plug[0].text);
                        asi_remove_generic_wrap_plug[0].text = msgstr;

                        sprintf(msgstr1, asi_remove_generic_wrap_plug[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232));
                        free (asi_remove_generic_wrap_plug[1].text);
                        asi_remove_generic_wrap_plug[1].text = msgstr1;

                        sprintf(msgstr2, asi_remove_generic_wrap_plug[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                RECONNECT_CABLE_PORT_3));

                        free (asi_remove_generic_wrap_plug[2].text);
                        asi_remove_generic_wrap_plug[2].text = msgstr2;

                        rc = diag_display(0x855051, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_NOTE:

                {
                        rc = diag_display(0x855008, mpqp_catd,
                                cable_note, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_note);
                        sprintf(msgstr,asi_cable_note[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_note[0].text);
                        asi_cable_note[0].text = msgstr;


                        /* set the cursor points to NO */
                        menutypes.cur_index = NO;
                        rc = diag_display(0x855008, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_note);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                return (QUIT);
                        }

                        rc = DIAG_ITEM_SELECTED (menutypes);

                }
                break;

                case CABLE_ISOLATE_PORT_0_232:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855060,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_232));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855060, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case CABLE_ISOLATE_PORT_1_232:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855065,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_232));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855065, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case CABLE_ISOLATE_PORT_2_232:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855070,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_232));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855070, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case CABLE_ISOLATE_PORT_3_232:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855075,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_232),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_3_232));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855075, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;

                case CABLE_ISOLATE_PORT_0_V35:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855060,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_V35));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855060, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_ISOLATE_PORT_0_X21:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855060,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_X21),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_X21));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855060, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_ISOLATE_PORT_0_422:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855060,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_0_422));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855060, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_ISOLATE_PORT_1_V35:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855065,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_V35),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_1_V35));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855065, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_ISOLATE_PORT_2_422:
                {
                        /* ask the user to plug the wrap plug in        */
                        rc = diag_display(0x855070,
                                mpqp_catd, cable_isolate,
                                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                                &menutypes, asi_cable_isolate);
                        sprintf(msgstr, asi_cable_isolate[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_isolate[0].text);
                        asi_cable_isolate[0].text = msgstr;


                        sprintf(msgstr1, asi_cable_isolate[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                WRAP_PLUG_422),
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                PORT_2_422));

                        free (asi_cable_isolate[1].text);
                        asi_cable_isolate[1].text = msgstr1;


                        rc = diag_display(0x855070, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_cable_isolate);

                        if ((check_asl_stat (rc)) == QUIT)
                        {
                                rc = QUIT;
                        }

                }
                break;
                case CABLE_MENU_TESTING:
                {
                        rc = diag_display(0x855009, mpqp_catd,
                                cable_menu, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_menu);
                        sprintf(msgstr,asi_cable_menu[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_cable_menu[0].text);
                        asi_cable_menu[0].text = msgstr;

                        if (port_0_V35_tested)
                        {
                                sprintf(msgstr1, asi_cable_menu[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr1, asi_cable_menu[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[1].text);
                        asi_cable_menu[1].text = msgstr1;

                        if (port_0_232_tested)
                        {
                                sprintf(msgstr2, asi_cable_menu[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr2, asi_cable_menu[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[2].text);
                        asi_cable_menu[2].text = msgstr2;

                        if (port_0_X21_tested)
                        {
                                sprintf(msgstr3, asi_cable_menu[3].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr3, asi_cable_menu[3].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[3].text);
                        asi_cable_menu[3].text = msgstr3;

                        if (port_0_422_tested)
                        {
                                sprintf(msgstr4, asi_cable_menu[4].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr4, asi_cable_menu[4].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[4].text);
                        asi_cable_menu[4].text = msgstr4;

                        if (port_1_232_tested)
                        {
                                sprintf(msgstr5, asi_cable_menu[5].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr5, asi_cable_menu[5].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[5].text);
                        asi_cable_menu[5].text = msgstr5;

                        if (port_1_V35_tested)
                        {
                                sprintf(msgstr6, asi_cable_menu[6].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr6, asi_cable_menu[6].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[6].text);
                        asi_cable_menu[6].text = msgstr6;

                        if (port_2_232_tested)
                        {
                                sprintf(msgstr7, asi_cable_menu[7].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr7, asi_cable_menu[7].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[7].text);
                        asi_cable_menu[7].text = msgstr7;

                        if (port_2_422_tested)
                        {
                                sprintf(msgstr8, asi_cable_menu[8].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr8, asi_cable_menu[8].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[8].text);
                        asi_cable_menu[8].text = msgstr8;

                        if (port_3_232_tested)
                        {
                                sprintf(msgstr9, asi_cable_menu[9].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr9, asi_cable_menu[9].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_cable_menu[9].text);
                        asi_cable_menu[9].text = msgstr9;


                        if ((port_0_V35_tested) || ( port_0_X21_tested) ||
                            (port_0_232_tested) || ( port_0_422_tested) ||
                            (port_1_232_tested) || ( port_1_V35_tested) ||
                            (port_2_232_tested) || ( port_2_422_tested) ||
                            (port_3_232_tested))
                        {
                                sprintf(msgstr10, asi_cable_menu[10].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOTE_ASTERISK_CABLE));
                                free (asi_cable_menu[10].text);
                                asi_cable_menu[10].text = msgstr10;
                        }
                        else
                        {
                                sprintf(msgstr10, asi_cable_menu[10].text, " ");
                                free (asi_cable_menu[10].text);
                                asi_cable_menu[10].text = msgstr10;
                        }



                        /* set the cursor points to NO */
                        if (!port_0_V35_tested)
                                menutypes.cur_index = 1;
                        else if (!port_0_232_tested)
                                menutypes.cur_index = 2;
                        else if (!port_0_X21_tested)
                                menutypes.cur_index = 3;
                        else if (!port_0_422_tested)
                                menutypes.cur_index = 4;
                        else if (!port_1_232_tested)
                                menutypes.cur_index = 5;
                        else if (!port_1_V35_tested)
                                menutypes.cur_index = 6;
                        else if (!port_2_232_tested)
                                menutypes.cur_index = 7;
                        else if (!port_2_422_tested)
                                menutypes.cur_index = 8;
                        else if (!port_3_232_tested)
                                menutypes.cur_index = 9;

                        rc = diag_display(0x855009, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_menu);

                        if ((check_asl_stat (rc)) == QUIT)
                                return(QUIT);

                        rc = DIAG_ITEM_SELECTED (menutypes);
                }
                break;

                case PORTS_MENU_TESTING:
                {
                        rc = diag_display(0x855036, mpqp_catd,
                                ports_menu, DIAG_MSGONLY,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_ports_menu);
                        sprintf(msgstr,asi_ports_menu[0].text,
                                da_input.dname, da_input.dnameloc);
                        free (asi_ports_menu[0].text);
                        asi_ports_menu[0].text = msgstr;

                        if (port_0_V35_tested)
                        {
                                sprintf(msgstr1, asi_ports_menu[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr1, asi_ports_menu[1].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[1].text);
                        asi_ports_menu[1].text = msgstr1;

                        if (port_0_232_tested)
                        {
                                sprintf(msgstr2, asi_ports_menu[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr2, asi_ports_menu[2].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[2].text);
                        asi_ports_menu[2].text = msgstr2;

                        if (port_0_X21_tested)
                        {
                                sprintf(msgstr3, asi_ports_menu[3].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr3, asi_ports_menu[3].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[3].text);
                        asi_ports_menu[3].text = msgstr3;

                        if (port_0_422_tested)
                        {
                                sprintf(msgstr4, asi_ports_menu[4].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr4, asi_ports_menu[4].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[4].text);
                        asi_ports_menu[4].text = msgstr4;

                        if (port_1_232_tested)
                        {
                                sprintf(msgstr5, asi_ports_menu[5].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr5, asi_ports_menu[5].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[5].text);
                        asi_ports_menu[5].text = msgstr5;

                        if (port_1_V35_tested)
                        {
                                sprintf(msgstr6, asi_ports_menu[6].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr6, asi_ports_menu[6].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[6].text);
                        asi_ports_menu[6].text = msgstr6;

                        if (port_2_232_tested)
                        {
                                sprintf(msgstr7, asi_ports_menu[7].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr7, asi_ports_menu[7].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[7].text);
                        asi_ports_menu[7].text = msgstr7;

                        if (port_2_422_tested)
                        {
                                sprintf(msgstr8, asi_ports_menu[8].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr8, asi_ports_menu[8].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[8].text);
                        asi_ports_menu[8].text = msgstr8;

                        if (port_3_232_tested)
                        {
                                sprintf(msgstr9, asi_ports_menu[9].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                TESTED));
                        }
                        else
                        {
                                sprintf(msgstr9, asi_ports_menu[9].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOT_TESTED));
                        }

                        free (asi_ports_menu[9].text);
                        asi_ports_menu[9].text = msgstr9;


                        if ((port_0_V35_tested) || ( port_0_X21_tested) ||
                            (port_0_232_tested) || ( port_0_422_tested) ||
                            (port_1_232_tested) || ( port_1_V35_tested) ||
                            (port_2_232_tested) || ( port_2_422_tested) ||
                            (port_3_232_tested))
                        {
                                sprintf(msgstr10, asi_ports_menu[10].text,
                                (char *)diag_cat_gets ( mpqp_catd, 1,
                                NOTE_ASTERISK_CABLE));
                                free (asi_ports_menu[10].text);
                                asi_ports_menu[10].text = msgstr10;
                        }
                        else
                        {
                                sprintf(msgstr10, asi_ports_menu[10].text, " ");
                                free (asi_ports_menu[10].text);
                                asi_ports_menu[10].text = msgstr10;
                        }



                        /* set the cursor points to NO */
                        if (!port_0_V35_tested)
                                menutypes.cur_index = 1;
                        else if (!port_0_232_tested)
                                menutypes.cur_index = 2;
                        else if (!port_0_X21_tested)
                                menutypes.cur_index = 3;
                        else if (!port_0_422_tested)
                                menutypes.cur_index = 4;
                        else if (!port_1_232_tested)
                                menutypes.cur_index = 5;
                        else if (!port_1_V35_tested)
                                menutypes.cur_index = 6;
                        else if (!port_2_232_tested)
                                menutypes.cur_index = 7;
                        else if (!port_2_422_tested)
                                menutypes.cur_index = 8;
                        else if (!port_3_232_tested)
                                menutypes.cur_index = 9;

                        rc = diag_display(0x855036, mpqp_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_ports_menu);

                        if ((check_asl_stat (rc)) == QUIT)
                                return(QUIT);

                        rc = DIAG_ITEM_SELECTED (menutypes);
                }
                break;
        }
        return (rc);
} /* display */

int display_generic_wrap_plug (wrap_plug_type)
int     wrap_plug_type;
{
        int     rc;
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        rc = diag_display(0x855010, mpqp_catd,
                generic_wrap_plug, DIAG_MSGONLY,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug);
        sprintf(msgstr,asi_generic_wrap_plug[0].text,
                da_input.dname, da_input.dnameloc);
        free (asi_generic_wrap_plug[0].text);
        asi_generic_wrap_plug[0].text = msgstr;

        if (wrap_plug_type == WRAP_PLUG_232)
        {
                sprintf (msgstr1,asi_generic_wrap_plug[3].text,
                (char *)diag_cat_gets ( mpqp_catd, 1, WRAP_PLUG_232));

        }
        else if ( wrap_plug_type == WRAP_PLUG_V35)
        {
                sprintf (msgstr1,asi_generic_wrap_plug[3].text,
                (char *)diag_cat_gets ( mpqp_catd, 1, WRAP_PLUG_V35));
        }
        else if (wrap_plug_type == WRAP_PLUG_X21)
        {
                sprintf (msgstr1,asi_generic_wrap_plug[3].text,
                (char *)diag_cat_gets ( mpqp_catd, 1, WRAP_PLUG_X21));
        }
        else if ( wrap_plug_type == WRAP_PLUG_422)
        {
                sprintf (msgstr1,asi_generic_wrap_plug[3].text,
                (char *)diag_cat_gets ( mpqp_catd, 1, WRAP_PLUG_422));
        }

        free (asi_generic_wrap_plug[3].text);
        asi_generic_wrap_plug[3].text = msgstr1;


        /* set the cursor points to NO */
        menutypes.cur_index = NO;
        rc = diag_display(0x855010, mpqp_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug);

        if ((check_asl_stat (rc)) == QUIT)
        {
                return (QUIT);
        }

        rc = DIAG_ITEM_SELECTED (menutypes);
        if      /* if user does not have the wrap NTF */
                ( rc != YES)
        {
                cable_tested= FALSE;
                putdavar(da_input.dname, "cable_tested", DIAG_SHORT,
                        &cable_tested);
                return (NO);
        }

        /* setting wrap_plug variable in data base to true */

        wrap_plug = TRUE;
        putdavar(da_input.dname, "wrap_plug", DIAG_SHORT, &wrap_plug);
        /* setting cable_tested variable in data base to true */
        return (YES);
}
