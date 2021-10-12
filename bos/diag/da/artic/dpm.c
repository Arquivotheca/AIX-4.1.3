static char sccsid[] = "@(#)69  1.2  src/bos/diag/da/artic/dpm.c, daartic, bos41J, 9511A_all 3/3/95 11:13:53";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *              cable_menu_testing_6_ports
 *              cable_menu_testing_8_ports
 *              cable_test_port
 *              cable_testing_required
 *              display_generic_wrap_plug_portmaster
 *              insert_generic_cable
 *              insert_generic_wrap_plug_portmaster
 *              insert_wrap_after_bad_cable_test
 *              insert_wrap_into_port
 *              port_menu_testing_6_ports
 *              port_menu_testing_8_ports
 *              portmaster_advanced_tests
 *              ports_testing
 *              ports_testing_required
 *              remove_generic_cable
 *              remove_generic_wrap_from_port
 *              remove_generic_wrap_plug_portmaster
 *              report_frub_pm_cable
 *              running_big_wrap_plug_tests
 *              test_ports
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
#include "dpm_msg.h"
#include "artictst.h"
#include <locale.h>

extern short    wrap_plug;
extern short    cable_tested;
extern  nl_catd diag_catopen();
extern  getdainput();           /* gets input from ODM                  */
extern  long getdamode();       /* transforms input into a mode         */
extern  int errno;              /* reason device driver won't open      */
extern  int which_type;
extern  EIB_type;

extern volatile int alarm_timeout;
extern short   wrap_plug_V35;
extern short   wrap_plug_232;
extern short   wrap_plug_422;
struct  sigaction  invec;       /* interrupt handler structure  */


short   big_wrap_plug_pm = FALSE;
short   wrap_plug_X21=FALSE;

char    card_name[80];
char    port_name[25];
short   port_number;

extern TUTYPE   artic_tucb;
extern struct   tm_input da_input;
extern int      filedes;
extern nl_catd catd;
nl_catd portmaster_catd;
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
extern void     set_timer();
extern void     generic_report_frub();

static short   port_0_tested=FALSE;
static short   port_1_tested=FALSE;
static short   port_2_tested=FALSE;
static short   port_3_tested=FALSE;
static short   port_4_tested=FALSE;
static short   port_5_tested=FALSE;
static short   port_6_tested=FALSE;
static short   port_7_tested=FALSE;

void report_frub_pm_cable ();

/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/
/** PORT MASTER MENU SECTION            */
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


struct msglist generic_wrap_plug_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, HAVE_WRAP_PLUG_PM                  },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_generic_wrap_plug_pm[DIAG_NUM_ENTRIES(generic_wrap_plug_pm)];

struct msglist insert_generic_wrap_plug_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_WRAP_PLUG_PM        },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_generic_wrap_plug_pm[DIAG_NUM_ENTRIES(insert_generic_wrap_plug_pm)];

struct msglist remove_generic_wrap_plug_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_GENERIC_WRAP_PLUG_PM        },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_remove_generic_wrap_plug_pm[DIAG_NUM_ENTRIES(remove_generic_wrap_plug_pm)];

struct msglist cable_note_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, CABLE_NOTE_PM                      },
                {       0, 0                    }

        };

ASL_SCR_INFO    asi_cable_note_pm[DIAG_NUM_ENTRIES(cable_note_pm)];

struct msglist insert_generic_cable_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_CABLE_PM            },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_generic_cable_pm[DIAG_NUM_ENTRIES(insert_generic_cable_pm)];

struct msglist insert_wrap_after_bad_cable_test_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_WRAP_AFTER_BAD_CABLE_TEST   },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_wrap_after_bad_cable_test_pm[DIAG_NUM_ENTRIES(insert_wrap_after_bad_cable_test_pm)];

/* ------------------------------------------------------------ */
/**      FRU part for Portmaster adapter                        */
/* ------------------------------------------------------------ */

/* FRU for cable of port_master */
struct fru_bucket portmaster_cable_frus[] =
{
        /* V.35 Cable is BAD            */
        {"", FRUB1, 0x855, 0x118, MPQP_V_35_CABLE_FAILURE,
                {
                        {100, " ", "", V35_cable,
                                 NOT_IN_DB, EXEMPT},

                },
        },
        {"", FRUB1, 0x855, 0x119, MPQP_X_21_CABLE_FAILURE,
                {
                        {100, " ", "", X21_cable,
                                 NOT_IN_DB, EXEMPT},

                },
        },
        {"", FRUB1, 0x855, 0x116, MPQP_RS_232_CABLE_FAILURE,
                {
                        {100, " ", "", cable_232,
                                 NOT_IN_DB, EXEMPT},

                },
        },

        {"", FRUB1, 0x855, 0x117, MPQP_RS_422_CABLE_FAILURE,
                {
                        {100, " ", "", cable_422,
                                 NOT_IN_DB, EXEMPT},

                },
        }
};

struct fru_bucket portmaster_interface_box_frus[] =
{
        {"", FRUB1, 0x855, 0x720, CABLE_WRAP_TEST_FAILURE,
                {
                        {90, " ", "", MULTIPORT_INTERFACE_BOX,
                                 NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},

                },
        }
};
struct fru_bucket port_frus[] =
{
        {"", FRUB1, 0x855, 0x721, PORT_WRAP_TEST_FAILURE,
                {
                        {90, " ", "", MULTIPORT_INTERFACE_BOX,
                                 NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},

                },
        }
};

/*----------------------------------------------------------------------*/
/*      Function :      portmaster_advanced_tests                       */
/*      Description     advanced tests for portmaster adapters          */
/*                      There are 4 different adapters for portmaster   */
/*                      1. 6 port V.35                                  */
/*                      2. 8 port RS 232                                */
/*                      3. 8 port RS 422                                */
/*                      4. 6 port X.21                                  */
/*                                                                      */
/*      EIB_type        101 : 6 port V.35                               */
/*      EIB_type        102 : 6 port X.21                               */
/*      EIB_type        103 : 8 port 232                                */
/*      EIB_type        104 : 8 port 422                                */
/*----------------------------------------------------------------------*/
portmaster_advanced_tests ()
{
        int     rc = NO_ERROR;
        int     rc1;
        char    message_text[126];

        artic_tucb.header.mfg = 0;
        artic_tucb.header.loop = 1;

        portmaster_catd = diag_catopen (MF_DPM,0);
        switch (EIB_type)
        {
                case 101:               /* 6 Port V.35 Portmaster       */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                PM_6PORT_V35));
                        sprintf (message_text,
                                (char *)diag_cat_gets( portmaster_catd, 1,
                                BIG_WRAP_PLUG_6PORT_V35));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( portmaster_catd, 1, V_35));
                break;
                case 102:       /* 6 Port X.21 */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                PM_6PORT_X21));
                        sprintf (message_text,
                                (char *)diag_cat_gets( portmaster_catd, 1,
                                BIG_WRAP_PLUG_6PORT_X21));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( portmaster_catd, 1, X_21));
                break;

                case 103:       /* 8 Port 232  */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                PM_8PORT_232));
                        sprintf (message_text,
                                (char *)diag_cat_gets( portmaster_catd, 1,
                                BIG_WRAP_PLUG_8PORT_232));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( portmaster_catd, 1, RS_232));
                break;
                case 104:       /* 8 Port 422  */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                PM_8PORT_422));
                        sprintf (message_text,
                                (char *)diag_cat_gets( portmaster_catd, 1,
                                BIG_WRAP_PLUG_8PORT_422));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( portmaster_catd, 1, RS_422));
                break;
        }

        if (da_input.loopmode == LOOPMODE_INLM)
        {
                getdavar(da_input.dname, "big_wrap_plug_pm", DIAG_SHORT,
                        &big_wrap_plug_pm);
                if (big_wrap_plug_pm)
                {
                        rc = running_big_wrap_plug_tests ();
                }
        }
        else if ((da_input.loopmode == LOOPMODE_NOTLM) ||
                (da_input.loopmode == LOOPMODE_ENTERLM))
        {

                rc = display_generic_wrap_plug_portmaster (message_text);
                da_display ();
                if (rc != NO)
                {
                        rc = insert_generic_wrap_plug_portmaster(message_text);
                        if (rc != QUIT)
                        {
                                /* running 78 pin wrap plug test */
                                rc = running_big_wrap_plug_tests ();
                                if (!(da_input.loopmode & LOOPMODE_ENTERLM))
                                   rc1 = remove_generic_wrap_plug_portmaster();
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
                getdavar(da_input.dname, "big_wrap_plug_pm", DIAG_SHORT,
                        &big_wrap_plug_pm);
                if (big_wrap_plug_pm)
                {
                        remove_generic_wrap_plug_portmaster();
                }
        }
        close (portmaster_catd);
        return (rc);
}
/*----------------------------------------------------------------------------*/
/*      Function :      running_big_wrap_plug_tests ()                        */
/*      Description:    This funtion will  run all the test that require      */
/*                                                                            */
/*      Return          0 : good                                              */
/*                      -1 : bad                                              */
/*----------------------------------------------------------------------------*/

int running_big_wrap_plug_tests ()
{
        int rc;
        int     fru_index=0;
        struct  portmaster_internal_fru_pair *start;

        start = portmaster_internal_tests;
        da_display ();

        which_type = PORTMASTER_ADVANCED;

        /* Running test 53, 54, 55, 56, 57, 58, 59, 60*/
        for (; start->tu != -1; ++start, ++fru_index)
        {
                if (((EIB_type == 101) || (EIB_type == 102)) &&
                        start->tu == 59)
                        break;
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
        int     wrap_part_number[64];

        /* ask if testing cable is required     */
        rc1 = cable_testing_required();
        if (rc1 == YES)
        {
                while  ((rc != QUIT )  && ( rc == NO_ERROR))
                {
                        cable_tested = TRUE;
                        if ((EIB_type == 101) || (EIB_type == 102))
                        {
                                rc = cable_menu_testing_6_ports ();
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                        }
                        else
                                rc = cable_menu_testing_8_ports ();
                        if (rc == QUIT)
                                break;
                        port_number = rc -1;

                        switch (EIB_type)
                        {
                                case 101:       /* 6 port V.35 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, V35_WRAP_PLUG));
                                break;
                                case 102:
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, X21_WRAP_PLUG));
                                break;
                                case 103:       /* 8 port 232 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, RS_232_WRAP_PLUG));
                                break;
                                case 104:       /* 8 port 422 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, RS_422_WRAP_PLUG));
                                break;
                        }
                        if ((!wrap_plug_V35 ) && (!wrap_plug_X21) &&
                           (!wrap_plug_232) && (!wrap_plug_422))
                        {
                                /* ask the user if wrap plug is available */
                                rc = display_generic_wrap_plug_portmaster
                                        (wrap_part_number);
                                da_display ();
                                if (rc == NO)
                                        break;
                        }
                        switch (EIB_type)
                        {
                                case 101:
                                        wrap_plug_V35= TRUE;
                                        break;
                                case 102:
                                        wrap_plug_X21= TRUE;
                                        break;
                                case 103:
                                        wrap_plug_232 = TRUE;
                                        break;
                                case 104:
                                        wrap_plug_422 = TRUE;
                                        break;
                        }
                        rc = cable_test_port ();
                        if (rc == QUIT)
                                break;

                        if ((port_0_tested) && ( port_1_tested) &&
                                (port_2_tested) && ( port_3_tested) &&
                                (port_4_tested) && ( port_5_tested) &&
                                (port_6_tested) && (port_7_tested))
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

        rc1 = ports_testing_required();
        da_display();
        if ((rc1 != NO ) && (rc1 != QUIT))
        {
                rc = NO_ERROR;
                while  ((rc != QUIT )  && ( rc == NO_ERROR))
                {
                        cable_tested = FALSE;
                        if ((EIB_type == 101) || (EIB_type == 102))
                        {
                                /* Six ports for these adapters */
                                rc = port_menu_testing_6_ports ();
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                        }
                        else
                                rc = port_menu_testing_8_ports ();
                        if (rc == QUIT)
                                break;
                        port_number = rc -1;

                        switch (EIB_type)
                        {
                                case 101:       /* 6 port V.35 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, V35_WRAP_PLUG));
                                break;
                                case 102:
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, X21_WRAP_PLUG));
                                break;
                                case 103:       /* 8 port 232 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, RS_232_WRAP_PLUG));
                                break;
                                case 104:       /* 8 port 422 */
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets( portmaster_catd,
                                         1, RS_422_WRAP_PLUG));
                                break;
                        }
                        if ((!wrap_plug_V35 ) && (!wrap_plug_X21) &&
                           (!wrap_plug_232) && (!wrap_plug_422))
                        {
                                /* ask the user if wrap plug is available */
                                rc = display_generic_wrap_plug_portmaster
                                        (wrap_part_number);
                                da_display ();
                                if (rc == NO)
                                        break;
                        }
                        switch (EIB_type)
                        {
                                case 101:
                                        wrap_plug_V35= TRUE;
                                break;
                                case 102:
                                        wrap_plug_X21= TRUE;
                                break;
                                case 103:
                                        wrap_plug_232= TRUE;
                                break;
                                case 104:
                                        wrap_plug_422= TRUE;
                                break;
                        }
                        rc = test_ports();
                        if (rc == QUIT)
                                break;
                        if ((port_0_tested) && ( port_1_tested) &&
                                (port_2_tested) && ( port_3_tested) &&
                                (port_4_tested) && ( port_5_tested) &&
                                (port_6_tested) && (port_7_tested))
                        {
                                break;
                        }
                }
        }
}

/*----------------------------------------------------------------------------*/
/*      Function :     cable_test_port                                        */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int     cable_test_port()
{
        int rc;
        rc = insert_generic_cable();
        if (rc != QUIT)
        {
                da_display();
                set_timer ();
                artic_tucb.header.tu = 53 + port_number;
                rc = exectu(filedes, &artic_tucb);
                if ((rc != 0) || (alarm_timeout))
                {
                        /* ask the user to unplug cable and put wrap    */
                        /* into port to test                            */
                        rc = insert_wrap_after_bad_cable_test();
                        da_display();
                        set_timer ();
                        artic_tucb.header.tu = 53 + port_number;
                        rc = exectu(filedes, &artic_tucb);
                        if ((rc != 0) || (alarm_timeout))
                        {
                                        generic_report_frub(
                                           &portmaster_interface_box_frus);
                        }
                        else if (rc == NO_ERROR)
                        {
                                switch (EIB_type)
                                {
                                        case 101:       /* V.35 6 ports */
                                          report_frub_pm_cable(CABLE_V35_PM);
                                          break;
                                        case 102:       /* X.21 6 ports */
                                          report_frub_pm_cable(CABLE_X21_PM);
                                          break;
                                        case 103:       /* 8 ports 232 */
                                          report_frub_pm_cable(CABLE_232_PM);
                                          break;
                                        case 104:       /* 8 ports 422 */
                                          report_frub_pm_cable(CABLE_422_PM);
                                          break;
                                }
                        }
                        rc = -1 ;       /* to exit test */
                        remove_generic_wrap_from_port();
                }
                else
                {
                        switch (port_number)
                        {
                                case 0:
                                        port_0_tested = TRUE;
                                        break;
                                case 1:
                                        port_1_tested = TRUE;
                                        break;
                                case 2:
                                        port_2_tested = TRUE;
                                        break;
                                case 3:
                                        port_3_tested = TRUE;
                                        break;
                                case 4:
                                        port_4_tested = TRUE;
                                        break;
                                case 5:
                                        port_5_tested = TRUE;
                                        break;
                                case 6:
                                        port_6_tested = TRUE;
                                        break;
                                case 7:
                                        port_7_tested = TRUE;
                                        break;
                        }
                        rc = 0;
                        remove_generic_cable();
                }
        }
        da_display();
        return (rc);
}
/*--------------------------------------------------------------*/
/*      Messages and menus                                      */
/*--------------------------------------------------------------*/

int display_generic_wrap_plug_portmaster (wrap_plug_name)
char *wrap_plug_name;
{
        int     rc;

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        rc = diag_display(0x855005, portmaster_catd,
                generic_wrap_plug_pm, DIAG_MSGONLY,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug_pm);
        sprintf(msgstr,asi_generic_wrap_plug_pm[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_generic_wrap_plug_pm[0].text);
        asi_generic_wrap_plug_pm[0].text = msgstr;

        sprintf (msgstr1,asi_generic_wrap_plug_pm[3].text, wrap_plug_name);


        free (asi_generic_wrap_plug_pm[3].text);
        asi_generic_wrap_plug_pm[3].text = msgstr1;


        /* set the cursor points to NO */
        menutypes.cur_index = NO;
        rc = diag_display(0x855005, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug_pm);

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
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int insert_generic_wrap_plug_portmaster (wrap_plug_name)
char *wrap_plug_name;
{
        int rc;
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

                /* ask the user to plug the wrap plug in        */
                rc = diag_display(0x855006,
                        portmaster_catd, insert_generic_wrap_plug_pm,
                        DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_insert_generic_wrap_plug_pm);
                sprintf(msgstr, asi_insert_generic_wrap_plug_pm[0].text,
                                card_name, da_input.dname, da_input.dnameloc);
                free (asi_insert_generic_wrap_plug_pm[0].text);
                asi_insert_generic_wrap_plug_pm[0].text = msgstr;

                sprintf(msgstr1, asi_insert_generic_wrap_plug_pm[1].text,
                        card_name, wrap_plug_name, card_name);
                free (asi_insert_generic_wrap_plug_pm[1].text);
                asi_insert_generic_wrap_plug_pm[1].text = msgstr1;

                rc = diag_display(0x855006, portmaster_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_insert_generic_wrap_plug_pm);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }
                big_wrap_plug_pm = TRUE;
                putdavar(da_input.dname, "big_wrap_plug_pm",
                DIAG_SHORT, &big_wrap_plug_pm);
}

/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int remove_generic_wrap_plug_portmaster ()
{
        int rc;
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

                /* ask the user to plug the wrap plug in        */
                rc = diag_display(0x855007,
                        portmaster_catd, remove_generic_wrap_plug_pm,
                        DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_remove_generic_wrap_plug_pm);
                sprintf(msgstr, asi_remove_generic_wrap_plug_pm[0].text,
                                card_name, da_input.dname, da_input.dnameloc);
                free (asi_remove_generic_wrap_plug_pm[0].text);
                asi_remove_generic_wrap_plug_pm[0].text = msgstr;

                sprintf(msgstr1, asi_remove_generic_wrap_plug_pm[1].text,
                        card_name, card_name);
                free (asi_remove_generic_wrap_plug_pm[1].text);
                asi_remove_generic_wrap_plug_pm[1].text = msgstr1;

                rc = diag_display(0x855007, portmaster_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_pm);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }
                big_wrap_plug_pm = FALSE;
                putdavar(da_input.dname, "big_wrap_plug_pm",
                DIAG_SHORT, &big_wrap_plug_pm);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int     cable_testing_required()
{
                int rc;
                static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

                rc = diag_display(0x855008, portmaster_catd,
                        cable_note_pm, DIAG_MSGONLY,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_cable_note_pm);
                sprintf(msgstr,asi_cable_note_pm[0].text,
                        card_name, da_input.dname, da_input.dnameloc);
                free (asi_cable_note_pm[0].text);
                asi_cable_note_pm[0].text = msgstr;

                /* set the cursor points to NO */
                menutypes.cur_index = NO;
                rc = diag_display(0x855008, portmaster_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_note_pm);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }

                rc = DIAG_ITEM_SELECTED (menutypes);
                return (rc);

}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int cable_menu_testing_6_ports ()
{
        struct msglist cable_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CABLE_PORT_0},
                { 1, CABLE_PORT_1},
                { 1, CABLE_PORT_2},
                { 1, CABLE_PORT_3},
                { 1, CABLE_PORT_4},
                { 1, CABLE_PORT_5},
                { 1, CABLE_SELECTION_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_cable_menu[DIAG_NUM_ENTRIES(cable_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        int rc;

        rc = diag_display(0x855009, portmaster_catd,
                cable_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_cable_menu);
        sprintf(msgstr,asi_cable_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_cable_menu[0].text);
        asi_cable_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[1].text);
        asi_cable_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_cable_menu[2].text);
        asi_cable_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[3].text);
        asi_cable_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[4].text);
        asi_cable_menu[4].text = msgstr4;

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[5].text);
        asi_cable_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[6].text);
        asi_cable_menu[6].text = msgstr6;

        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;
        else if (!port_4_tested)
                menutypes.cur_index = 5;
        else if (!port_5_tested)
                menutypes.cur_index = 6;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested) ||
                (port_4_tested) || ( port_5_tested))
        {
                sprintf(msgstr10, asi_cable_menu[7].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_cable_menu[7].text);
                asi_cable_menu[7].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_cable_menu[7].text, " ");
                free (asi_cable_menu[7].text);
                asi_cable_menu[7].text = msgstr10;
        }


        rc = diag_display(0x855009, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, asi_cable_menu);

        if ((check_asl_stat (rc)) == QUIT)
                return(QUIT);

        rc = DIAG_ITEM_SELECTED (menutypes);
        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int cable_menu_testing_8_ports ()
{
        struct msglist cable_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CABLE_PORT_0},
                { 1, CABLE_PORT_1},
                { 1, CABLE_PORT_2},
                { 1, CABLE_PORT_3},
                { 1, CABLE_PORT_4},
                { 1, CABLE_PORT_5},
                { 1, CABLE_PORT_6},
                { 1, CABLE_PORT_7},
                { 1, CABLE_SELECTION_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_cable_menu[DIAG_NUM_ENTRIES(cable_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        int rc;
        char    which_type_cable[26];

        rc = diag_display(0x855009, portmaster_catd,
                cable_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_cable_menu);
        sprintf(msgstr,asi_cable_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_cable_menu[0].text);
        asi_cable_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[1].text);
        asi_cable_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_cable_menu[2].text);
        asi_cable_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[3].text);
        asi_cable_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[4].text);
        asi_cable_menu[4].text = msgstr4;

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[5].text);
        asi_cable_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[6].text);
        asi_cable_menu[6].text = msgstr6;

        if (port_6_tested)
        {
                sprintf(msgstr7, asi_cable_menu[7].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr7, asi_cable_menu[7].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[7].text);
        asi_cable_menu[7].text = msgstr7;

        if (port_7_tested)
        {
                sprintf(msgstr8, asi_cable_menu[8].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr8, asi_cable_menu[8].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[8].text);
        asi_cable_menu[8].text = msgstr8;


        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;
        else if (!port_4_tested)
                menutypes.cur_index = 5;
        else if (!port_5_tested)
                menutypes.cur_index = 6;
        else if (!port_6_tested)
                menutypes.cur_index = 7;
        else if (!port_7_tested)
                menutypes.cur_index = 8;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested) ||
                (port_4_tested) || ( port_5_tested) ||
                 (port_6_tested) || (port_7_tested))
        {
                sprintf(msgstr10, asi_cable_menu[9].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_cable_menu[9].text);
                asi_cable_menu[9].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_cable_menu[9].text, " ");
                free (asi_cable_menu[9].text);
                asi_cable_menu[9].text = msgstr10;
        }



        rc = diag_display(0x855009, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, asi_cable_menu);

        if ((check_asl_stat (rc)) == QUIT)
                return(QUIT);

        rc = DIAG_ITEM_SELECTED (menutypes);
        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int     insert_generic_cable()
{
        int     rc;
        char    part_number[25];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x855011, portmaster_catd, insert_generic_cable_pm,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_generic_cable_pm);
        sprintf(msgstr, asi_insert_generic_cable_pm[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_generic_cable_pm[0].text);
        asi_insert_generic_cable_pm[0].text = msgstr;

        if (EIB_type == 101)            /* V.35 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,V35_WRAP_PLUG));
        else if (EIB_type == 102)       /* X.21 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,X21_WRAP_PLUG));
        else if (EIB_type == 103)       /* 8 ports 232 */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_232_WRAP_PLUG));
        else if (EIB_type == 104)       /* 8 ports 422 */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_422_WRAP_PLUG));

        sprintf(msgstr1, asi_insert_generic_cable_pm[1].text,port_number,
                port_name, port_number,part_number);
        free (asi_insert_generic_cable_pm[1].text);
        asi_insert_generic_cable_pm[1].text = msgstr1;


        rc = diag_display(0x855011, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_insert_generic_cable_pm);

        if ((check_asl_stat (rc)) == QUIT)
        {
                rc = QUIT;
        }
        else
                rc =0;

        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/

int     remove_generic_cable()
{
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;
        int     rc;
        char    part_number[25];
        struct msglist remove_generic_cable_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_WRAP_PLUG_FROM_CABLE_PM             },
                {       0, 0                            }

        };

        static ASL_SCR_INFO
         asi_remove_generic_cable_pm[DIAG_NUM_ENTRIES(remove_generic_cable_pm)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x855012, portmaster_catd, remove_generic_cable_pm,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_remove_generic_cable_pm);
        sprintf(msgstr, asi_remove_generic_cable_pm[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_remove_generic_cable_pm[0].text);
        asi_remove_generic_cable_pm[0].text = msgstr;

        sprintf(msgstr1, asi_remove_generic_cable_pm[1].text,port_number);
        free (asi_remove_generic_cable_pm[1].text);
        asi_remove_generic_cable_pm[1].text = msgstr1;


        rc = diag_display(0x855012, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_remove_generic_cable_pm);

        if ((check_asl_stat (rc)) == QUIT)
        {
                rc = QUIT;
        }
        else
                rc =0;

        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/

int     insert_wrap_after_bad_cable_test()
{
        int     rc;
        char    part_number[25];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x855017, portmaster_catd, insert_wrap_after_bad_cable_test_pm,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_wrap_after_bad_cable_test_pm);
        sprintf(msgstr, asi_insert_wrap_after_bad_cable_test_pm[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_wrap_after_bad_cable_test_pm[0].text);
        asi_insert_wrap_after_bad_cable_test_pm[0].text = msgstr;

        if (EIB_type == 101)            /* V.35 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,V35_WRAP_PLUG));
        else if (EIB_type == 102)       /* X.21 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,V35_WRAP_PLUG));
        else if (EIB_type == 103)       /* 8 port 232   */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_232_WRAP_PLUG));
        else if (EIB_type == 104)       /* 8 port 422   */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_422_WRAP_PLUG));

        sprintf(msgstr1, asi_insert_wrap_after_bad_cable_test_pm[1].text,
                port_name, port_number,part_number, port_number);
        free (asi_insert_wrap_after_bad_cable_test_pm[1].text);
        asi_insert_wrap_after_bad_cable_test_pm[1].text = msgstr1;


        rc = diag_display(0x855017, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_insert_wrap_after_bad_cable_test_pm);

        if ((check_asl_stat (rc)) == QUIT)
        {
                rc = QUIT;
        }
        else
                rc =0;

        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int     ports_testing_required()
{
        int rc;
        struct msglist ports_note_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, PORTS_NOTE_PM                      },
                {       0, 0                    }

        };

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;
        static ASL_SCR_INFO     asi_ports_note_pm[DIAG_NUM_ENTRIES(ports_note_pm)];

                rc = diag_display(0x855013, portmaster_catd,
                        ports_note_pm, DIAG_MSGONLY,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_ports_note_pm);
                sprintf(msgstr,asi_ports_note_pm[0].text,
                        card_name, da_input.dname, da_input.dnameloc);
                free (asi_ports_note_pm[0].text);
                asi_ports_note_pm[0].text = msgstr;

                /* set the cursor points to NO */
                menutypes.cur_index = NO;
                rc = diag_display(0x855013, portmaster_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_ports_note_pm);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }

                rc = DIAG_ITEM_SELECTED (menutypes);
                return (rc);

}
int     test_ports()
{
        int rc;
        rc = insert_wrap_into_port();
        if (rc != QUIT)
        {
                da_display();
                set_timer ();
                artic_tucb.header.tu = 53 + port_number;
                rc = exectu(filedes, &artic_tucb);
                alarm (0);
                if ((rc != 0) || (alarm_timeout))
                {
                        generic_report_frub(&port_frus);
                }
                else
                {
                        switch (port_number)
                        {
                                case 0:
                                        port_0_tested = TRUE;
                                        break;
                                case 1:
                                        port_1_tested = TRUE;
                                        break;
                                case 2:
                                        port_2_tested = TRUE;
                                        break;
                                case 3:
                                        port_3_tested = TRUE;
                                        break;
                                case 4:
                                        port_4_tested = TRUE;
                                        break;
                                case 5:
                                        port_5_tested = TRUE;
                                        break;
                                case 6:
                                        port_6_tested = TRUE;
                                        break;
                                case 7:
                                        port_7_tested = TRUE;
                                        break;
                        }
                        rc = 0;
                }
                remove_generic_wrap_from_port();
        }
        da_display();
        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int     insert_wrap_into_port()
{
        int     rc;
        char    part_number[25];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        struct msglist insert_wrap_into_port_msg[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_WRAP_INTO_PORT      },
                {       0, 0                            }

        };

static ASL_SCR_INFO asi_insert_wrap_into_port_msg[DIAG_NUM_ENTRIES(insert_wrap_into_port_msg)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x855015, portmaster_catd, insert_wrap_into_port_msg,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_wrap_into_port_msg);
        sprintf(msgstr, asi_insert_wrap_into_port_msg[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_wrap_into_port_msg[0].text);
        asi_insert_wrap_into_port_msg[0].text = msgstr;

        if (EIB_type == 101)            /* V.35 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,V35_WRAP_PLUG));
        else if (EIB_type == 102)       /* X.21 6 ports */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,X21_WRAP_PLUG));
        else if (EIB_type == 103)       /* 8 ports 232 */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_232_WRAP_PLUG));
        else if (EIB_type == 104)       /* 8 ports 232 */
                sprintf(part_number,
                   (char *)diag_cat_gets(portmaster_catd, 1,RS_422_WRAP_PLUG));

        sprintf(msgstr1, asi_insert_wrap_into_port_msg[1].text,port_number,
                part_number, port_number);
        free (asi_insert_wrap_into_port_msg[1].text);
        asi_insert_wrap_into_port_msg[1].text = msgstr1;


        rc = diag_display(0x855015, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_insert_wrap_into_port_msg);

        if ((check_asl_stat (rc)) == QUIT)
        {
                rc = QUIT;
        }
        else
                rc =0;

        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int port_menu_testing_6_ports ()
{
        struct msglist port_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CONNECTOR_PORT_0},
                { 1, CONNECTOR_PORT_1},
                { 1, CONNECTOR_PORT_2},
                { 1, CONNECTOR_PORT_3},
                { 1, CONNECTOR_PORT_4},
                { 1, CONNECTOR_PORT_5},
                { 1, INTERFACE_CABLE_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_port_menu[DIAG_NUM_ENTRIES(port_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        int rc;

        rc = diag_display(0x855014, portmaster_catd,
                port_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_port_menu);
        sprintf(msgstr,asi_port_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_port_menu[0].text);
        asi_port_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[1].text);
        asi_port_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_port_menu[2].text);
        asi_port_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[3].text);
        asi_port_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[4].text);
        asi_port_menu[4].text = msgstr4;

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[5].text);
        asi_port_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[6].text);
        asi_port_menu[6].text = msgstr6;

        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;
        else if (!port_4_tested)
                menutypes.cur_index = 5;
        else if (!port_5_tested)
                menutypes.cur_index = 6;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested) ||
                (port_4_tested) || ( port_5_tested))
        {
                sprintf(msgstr10, asi_port_menu[7].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_port_menu[7].text);
                asi_port_menu[7].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_port_menu[7].text, " ");
                free (asi_port_menu[7].text);
                asi_port_menu[7].text = msgstr10;
        }


        rc = diag_display(0x855014, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, asi_port_menu);

        if ((check_asl_stat (rc)) == QUIT)
                return(QUIT);

        rc = DIAG_ITEM_SELECTED (menutypes);
        return (rc);
}
/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/

int     remove_generic_wrap_from_port()
{
        int     rc;
        char    part_number[25];
        struct msglist remove_generic_wrap_from_port_pm[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_GENERIC_WRAP_FROM_PORT      },
                {       0, 0                            }

        };

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;
        static ASL_SCR_INFO
         asi_remove_generic_wrap_from_port_pm[DIAG_NUM_ENTRIES(remove_generic_wrap_from_port_pm)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x855016, portmaster_catd, remove_generic_wrap_from_port_pm,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_remove_generic_wrap_from_port_pm);
        sprintf(msgstr, asi_remove_generic_wrap_from_port_pm[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_remove_generic_wrap_from_port_pm[0].text);
        asi_remove_generic_wrap_from_port_pm[0].text = msgstr;

        sprintf(msgstr1, asi_remove_generic_wrap_from_port_pm[1].text,port_number);
        free (asi_remove_generic_wrap_from_port_pm[1].text);
        asi_remove_generic_wrap_from_port_pm[1].text = msgstr1;


        rc = diag_display(0x855016, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_remove_generic_wrap_from_port_pm);

        if ((check_asl_stat (rc)) == QUIT)
        {
                rc = QUIT;
        }
        else
                rc =0;

        return (rc);
}

/*--------------------------------------------------------------------------
| NAME:
|
| FUNCTION:
|
| EXECUTION ENVIRONMENT:
|
| NOTES:  Other local routines called --
|
| RETURNS:
----------------------------------------------------------------------------*/
int port_menu_testing_8_ports ()
{
        struct msglist port_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CONNECTOR_PORT_0},
                { 1, CONNECTOR_PORT_1},
                { 1, CONNECTOR_PORT_2},
                { 1, CONNECTOR_PORT_3},
                { 1, CONNECTOR_PORT_4},
                { 1, CONNECTOR_PORT_5},
                { 1, CONNECTOR_PORT_6},
                { 1, CONNECTOR_PORT_7},
                { 1, INTERFACE_CABLE_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_port_menu[DIAG_NUM_ENTRIES(port_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        int rc;

        rc = diag_display(0x855014, portmaster_catd,
                port_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_port_menu);
        sprintf(msgstr,asi_port_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_port_menu[0].text);
        asi_port_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[1].text);
        asi_port_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_port_menu[2].text);
        asi_port_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[3].text);
        asi_port_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[4].text);
        asi_port_menu[4].text = msgstr4;

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[5].text);
        asi_port_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[6].text);
        asi_port_menu[6].text = msgstr6;

        if (port_6_tested)
        {
                sprintf(msgstr7, asi_port_menu[7].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr7, asi_port_menu[7].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[7].text);
        asi_port_menu[7].text = msgstr7;

        if (port_7_tested)
        {
                sprintf(msgstr8, asi_port_menu[8].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr8, asi_port_menu[8].text,
                  (char *)diag_cat_gets ( portmaster_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[8].text);
        asi_port_menu[8].text = msgstr8;

        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;
        else if (!port_4_tested)
                menutypes.cur_index = 5;
        else if (!port_5_tested)
                menutypes.cur_index = 6;
        else if (!port_6_tested)
                menutypes.cur_index = 7;
        else if (!port_7_tested)
                menutypes.cur_index = 8;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested) ||
                (port_6_tested) || ( port_7_tested) ||
                (port_4_tested) || ( port_5_tested))
        {
                sprintf(msgstr10, asi_port_menu[9].text,
                        (char *)diag_cat_gets ( portmaster_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_port_menu[9].text);
                asi_port_menu[9].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_port_menu[9].text, " ");
                free (asi_port_menu[9].text);
                asi_port_menu[9].text = msgstr10;
        }


        rc = diag_display(0x855014, portmaster_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, asi_port_menu);

        if ((check_asl_stat (rc)) == QUIT)
                return(QUIT);

        rc = DIAG_ITEM_SELECTED (menutypes);
        return (rc);
}

/*--------------------------------------------------------------------------
| NAME: report_frub_pm_cable
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

void report_frub_pm_cable (int cable_type)
{

        struct fru_bucket *frub_addr;
        switch (cable_type)
        {
                case    CABLE_V35_PM:
                        frub_addr = &mpqp_cable_frus[0];
                        break;
                case    CABLE_X21_PM:
                        frub_addr = &mpqp_cable_frus[1];
                        break;
                case    CABLE_232_PM:
                        frub_addr = &mpqp_cable_frus[2];
                        break;
                case    CABLE_422_PM:
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
