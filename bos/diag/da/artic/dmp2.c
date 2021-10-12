static char sccsid[] = "@(#)65  1.5.1.2  src/bos/diag/da/artic/dmp2.c, daartic, bos41J, 9511A_all 3/3/95 11:13:47";
/*
 *   COMPONENT_NAME: DAARTIC
 *
 *   FUNCTIONS: DIAG_NUM_ENTRIES
 *              cable_menu_testing_6_ports
 *              cable_menu_testing_8_ports
 *              cable_test_port
 *              cable_testing_required
 *              display_generic_wrap_plug_multiport
 *              insert_generic_cable
 *              insert_generic_wrap_plug_multiport
 *              insert_wrap_after_bad_cable_test
 *              insert_wrap_into_port
 *              port_menu_testing_6_ports
 *              port_menu_testing_8_ports
 *              multiport_2_advanced_tests
 *              ports_testing
 *              ports_testing_required
 *              remove_generic_cable
 *              remove_generic_wrap_from_port
 *              remove_generic_wrap_plug_multiport
 *              report_frub_mp_cable
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
#include "dxm2p_msg.h"
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
extern short   wrap_plug_232;
extern short   wrap_plug_422;
struct  sigaction  invec;       /* interrupt handler structure  */


short   big_wrap_plug_mp = FALSE;
short   wrap_plug_6_port=FALSE;

char    card_name[80];
char    port_name[25];
short   port_number;

extern TUTYPE   artic_tucb;
extern struct   tm_input da_input;
extern int      filedes;
extern nl_catd  catd;
nl_catd         multiport_catd;
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

void report_frub_mp_cable ();

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

struct msglist insert_generic_wrap_plug_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_WRAP_PLUG_PM        },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_generic_wrap_plug_mp[DIAG_NUM_ENTRIES(insert_generic_wrap_plug_mp)];



struct msglist generic_wrap_plug_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, HAVE_WRAP_PLUG_PM                  },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_generic_wrap_plug_mp[DIAG_NUM_ENTRIES(generic_wrap_plug_mp)];

struct msglist remove_generic_wrap_plug_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_GENERIC_WRAP_PLUG_PM        },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_remove_generic_wrap_plug_mp[DIAG_NUM_ENTRIES(remove_generic_wrap_plug_mp)];

struct msglist cable_note_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, CABLE_NOTE_PM                      },
                {       0, 0                    }

        };

ASL_SCR_INFO    asi_cable_note_mp[DIAG_NUM_ENTRIES(cable_note_mp)];

struct msglist insert_generic_cable_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_CABLE_PM            },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_generic_cable_mp[DIAG_NUM_ENTRIES(insert_generic_cable_mp)];

struct msglist insert_wrap_after_bad_cable_test_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_WRAP_AFTER_BAD_CABLE_TEST   },
                {       0, 0                            }

        };

ASL_SCR_INFO asi_insert_wrap_after_bad_cable_test_mp[DIAG_NUM_ENTRIES(insert_wrap_after_bad_cable_test_mp)];

/* ------------------------------------------------------------ */
/**      FRU part for Multiport adapter                 */
/* ------------------------------------------------------------ */

struct fru_bucket multiport_interface_box_frus[] =
{
        {"", FRUB1, 0x849, 0x720, CABLE_WRAP_TEST_FAILURE,
                {
                        {90, " ", "", MULTIPORT_INTERFACE_BOX,
                                 NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},

                },
        }
};
struct fru_bucket multiport_port_frus[] =
{
        {"", FRUB1, 0x849, 0x721, PORT_WRAP_TEST_FAILURE,
                {
                        {90, " ", "", MULTIPORT_INTERFACE_BOX,
                                 NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},

                },
        }
};

struct fru_bucket multiport_cable_frus[] =
{
        {"", FRUB1, 0x849, 0x722, CABLE_WRAP_TEST_FAILURE,
                {
                        {90, " ", "", MDB,
                                 NOT_IN_DB, EXEMPT},
                        {10, "", "", 0, DA_NAME, EXEMPT},

                },
        }
};
/*----------------------------------------------------------------------*/
/*      Function :      multiport_2_advanced_tests              */
/*      Description     advanced tests for multiport adapters           */
/*                      There are 5 different adapters for multiport    */
/*                                                                      */
/*      EIB_type        1. 4 port RS 232                                */
/*      EIB_type        2. 6 port RS 232                                */
/*      EIB_type        3. 8 port RS 232                                */
/*      EIB_type        4. 8 port RS 422                                */
/*      EIB_type        5. 4 port RS232/RS422                           */
/*                                                                      */
/*----------------------------------------------------------------------*/

char   message_text[80];
int   wrap_part_number[64];


multiport_2_advanced_tests ()
{
        int     rc = NO_ERROR;
        int     rc1;

        artic_tucb.header.mfg = 0;
        artic_tucb.header.loop = 1;

        multiport_catd = diag_catopen ("dxm2p.cat",0);
        switch (EIB_type)
        {
                case 1:         /* 4 Port RS 232        */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                MP2_4PORT_232));
                        sprintf (message_text,
                                (char *)diag_cat_gets( multiport_catd, 1,
                                BIG_WRAP_PLUG_8_4PORT));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( multiport_catd, 1, RS_232));
                break;
                case 2:         /* 6 Port RS-232        */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                MP2_6PORT_232));
                        sprintf (message_text,
                                (char *)diag_cat_gets( multiport_catd, 1,
                                BIG_WRAP_PLUG_6_PORT));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( multiport_catd, 1, RS_232));
                break;
                case 3:         /* 8 Port  RS 232       */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                MP2_8PORT_232));
                        sprintf (message_text,
                                (char *)diag_cat_gets( multiport_catd, 1,
                                BIG_WRAP_PLUG_8_4PORT));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( multiport_catd, 1, RS_232));
                break;
                case 4:         /* 8 Port  RS 422       */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                MP2_8PORT_422));
                        sprintf (message_text,
                                (char *)diag_cat_gets( multiport_catd, 1,
                                BIG_WRAP_PLUG_8_4PORT));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( multiport_catd, 1, RS_422));
                break;
                case 5:         /* 4 Port RS 232/RS 422         */
                        sprintf (card_name, (char *)diag_cat_gets( catd, 1,
                                MP2_4PORT_232_C4));
                        sprintf (message_text,
                                (char *)diag_cat_gets( multiport_catd, 1,
                                BIG_WRAP_PLUG_8_4PORT));
                        sprintf (port_name, (char *)diag_cat_gets
                                ( multiport_catd, 1, RS_232));
                break;
        }

        if (da_input.loopmode == LOOPMODE_INLM)
        {
                getdavar(da_input.dname, "big_wrap_plug_mp", DIAG_SHORT,
                        &big_wrap_plug_mp);
                if (big_wrap_plug_mp)
                {
                        rc = running_big_wrap_plug_tests ();
                }
        }
        else if ((da_input.loopmode == LOOPMODE_NOTLM) ||
                (da_input.loopmode == LOOPMODE_ENTERLM))
        {

                rc = display_generic_wrap_plug_multiport (message_text);
                da_display ();
                if (rc != NO)
                {
                        rc = insert_generic_wrap_plug_multiport(message_text);
                        if (rc != QUIT)
                        {
                                /* running 78 pin wrap plug test */
                                rc = running_big_wrap_plug_tests ();
                                if (!(da_input.loopmode & LOOPMODE_ENTERLM))
                                   rc1 = remove_generic_wrap_plug_multiport();
                                if ((!(da_input.loopmode & LOOPMODE_ENTERLM)) &&
                                        ( rc == NO_ERROR))
                                {
                                        rc = ports_testing ();
                                        da_display();
                                }
                        }
                }
        }
        else if (da_input.loopmode == LOOPMODE_EXITLM)
        {
                getdavar(da_input.dname, "big_wrap_plug_mp", DIAG_SHORT,
                        &big_wrap_plug_mp);
                if (big_wrap_plug_mp)
                {
                        remove_generic_wrap_plug_multiport();
                }
        }

        close (multiport_catd);
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
        struct  mp2_internal_fru_pair *start;


        start = mp2_internal_tests;
        da_display ();

        which_type = MP_2;

        /* Running test 22, 23, 24, 25, 26, 27, 28, 29, 30 */
        for (; start->tu != -1; ++start, ++fru_index)
        {
                if ( (EIB_type == 1) && (start->tu == 26) )
                        start+=4;
                if ((EIB_type == 2) && (start->tu == 28))
                        start+=2;

                rc = test_tu(start->tu,fru_index, which_type);
                if ( rc != NO_ERROR)
                {
                        break;
                }
                 if (da_input.console == TRUE && poll_kb_for_quit()==QUIT)
                        return(QUIT);
        }
        if (rc != 0) return (rc);


        da_display ();
        return (0);

}

/*----------------------------------------------------------------------------*/
/*      Function :     set_wrap_plug                                          */
/*      Description:                                                          */
/*                      0    : GOOD                                           */
/*                      9999 : QUIT                                           */
/*                      -1   : BAD                                            */
/*----------------------------------------------------------------------------*/
int  set_wrap_plug ()
{
        int   rc;
        int   ask;

        switch (EIB_type)
                {
                        case 1: /* 4 Port RS 232 */
                        case 3: /* 8 Port RS 232 */
                        case 4: /* 8 Port RS 422 */
                        case 5: /* 4 Port RS 232 / 4 Port RS 422 */
                                if ((port_number == 0 ) ||
                                   (port_number == 1))
                                {
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets
                                          ( multiport_catd, 1, wp_6));
                                }
                                else
                                {
                                        sprintf (wrap_part_number,
                                        (char *)diag_cat_gets
                                          ( multiport_catd, 1, wp_7));
                                }
                                break;
                        case 2: /* 6 port RS-232 */
                                sprintf (wrap_part_number,
                                (char *)diag_cat_gets( multiport_catd,
                                        1, wp_8));
                                break;
                }
                rc = YES;

                ask = FALSE;
                switch (EIB_type)
                {
                case 1:
                case 3:
                case 4:
                case 5:
                        if ((!wrap_plug_232 ) && ((port_number == 0) ||
                                                  (port_number == 1)))
                        {
                                ask = TRUE;
                        }
                        else if ((!wrap_plug_422)  && ((port_number == 2) ||
                                (port_number == 3)|| (port_number == 4)||
                                (port_number == 5)|| (port_number == 6)||
                                (port_number == 7)))
                        {
                                ask = TRUE;
                        }
                        break;
                case 2:
                        if (!wrap_plug_6_port)
                                ask = TRUE;
                        break;
                }

                if (ask == TRUE )
                {
                     /* ask the user if wrap plug is available */
                     rc = display_generic_wrap_plug_multiport
                                (wrap_part_number);
                     da_display ();

                     if (rc == YES)
                        switch (EIB_type)
                        {
                                case 1: /* 4 port RS 232 */
                                case 3:
                                case 4:
                                case 5:
                                        if ((port_number == 0 ) ||
                                           (port_number == 1))
                                                wrap_plug_232 = TRUE;
                                        else
                                                wrap_plug_422 = TRUE;
                                        break;
                                case 2:
                                        wrap_plug_6_port = TRUE;
                                        break;

                        }
                }
        return (rc);
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
        rc1 = cable_testing_required();
        if (rc1 == YES)
        {
                while  ((rc != QUIT )  && ( rc == NO_ERROR))
                {
                        cable_tested = TRUE;
                        switch (EIB_type)
                        {
                        case 1:
                                rc = cable_menu_testing_4_ports ();
                                port_4_tested = TRUE;
                                port_5_tested = TRUE;
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                                break;
                        case 2:
                                rc = cable_menu_testing_6_ports ();
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                                break;
                        case 3:
                        case 4:
                        case 5:
                                rc = cable_menu_testing_8_ports ();
                                break;
                        }

                        if (rc == QUIT)
                                break;
                        port_number = rc -1;
                        rc = set_wrap_plug ();

                        if (rc != YES)
                                break;
                        else
                        {
                                rc = cable_test_port ();
                                if (rc == QUIT)
                                        break;
                        }

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

                        switch (EIB_type)
                        {
                        case 1:
                                rc = port_menu_testing_4_ports ();
                                port_4_tested = TRUE;
                                port_5_tested = TRUE;
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                                break;
                        case 2:
                                rc = port_menu_testing_6_ports ();
                                port_6_tested = TRUE;
                                port_7_tested = TRUE;
                                break;
                        default:
                                rc = port_menu_testing_8_ports ();
                                break;
                        }

                        if (rc == QUIT)
                                break;
                        port_number = rc -1;

                        rc = set_wrap_plug ();

                        if (rc != YES)
                                break;
                        else
                        {
                                rc = test_ports();
                                if (rc == QUIT)
                                        break;
                        }
                        if ((port_0_tested) && ( port_1_tested) &&
                                (port_2_tested) && ( port_3_tested) &&
                                (port_4_tested) && ( port_5_tested) &&
                                (port_6_tested) && (port_7_tested))
                        {
                                return(0);
                        }
                }
        }
        return (rc);
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

                artic_tucb.header.tu = 22 + port_number;
                rc = exectu(filedes, &artic_tucb);

                if ((rc != 0) || (alarm_timeout))
                {
                        /* ask the user to unplug cable and put wrap    */
                        /* into port to test                            */
                        rc = insert_wrap_after_bad_cable_test();
                        da_display();
                        set_timer ();
                        artic_tucb.header.tu = 22 + port_number;
                        rc = exectu(filedes, &artic_tucb);

                        if ((rc != 0) || (alarm_timeout))
                        {
                                        generic_report_frub(
                                           &multiport_interface_box_frus);
                        }
                        else if (rc == NO_ERROR)
                                  report_frub_mp_cable();

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

int display_generic_wrap_plug_multiport (wrap_plug_name)
char *wrap_plug_name;
{
        int     rc;

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        rc = diag_display(0x849005, multiport_catd,
                generic_wrap_plug_mp, DIAG_MSGONLY,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug_mp);
        sprintf(msgstr,asi_generic_wrap_plug_mp[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_generic_wrap_plug_mp[0].text);
        asi_generic_wrap_plug_mp[0].text = msgstr;

        sprintf (msgstr1,asi_generic_wrap_plug_mp[3].text, wrap_plug_name);


        free (asi_generic_wrap_plug_mp[3].text);
        asi_generic_wrap_plug_mp[3].text = msgstr1;


        /* set the cursor points to NO */
        menutypes.cur_index = NO;
        rc = diag_display(0x849005, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                asi_generic_wrap_plug_mp);

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
int insert_generic_wrap_plug_multiport (wrap_plug_name)
char *wrap_plug_name;
{
        int rc;
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


                /* ask the user to plug the wrap plug in        */
                rc = diag_display(0x849006,
                        multiport_catd, insert_generic_wrap_plug_mp,
                        DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_insert_generic_wrap_plug_mp);
                sprintf(msgstr, asi_insert_generic_wrap_plug_mp[0].text,
                                card_name, da_input.dname, da_input.dnameloc);
                free (asi_insert_generic_wrap_plug_mp[0].text);
                asi_insert_generic_wrap_plug_mp[0].text = msgstr;

                sprintf(msgstr1, asi_insert_generic_wrap_plug_mp[1].text,
                        card_name, wrap_plug_name, card_name);
                free (asi_insert_generic_wrap_plug_mp[1].text);
                asi_insert_generic_wrap_plug_mp[1].text = msgstr1;

                rc = diag_display(0x849006, multiport_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_insert_generic_wrap_plug_mp);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }
                big_wrap_plug_mp = TRUE;
                putdavar(da_input.dname, "big_wrap_plug_mp",
                DIAG_SHORT, &big_wrap_plug_mp);
                return (0);
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
int remove_generic_wrap_plug_multiport ()
{
        int rc;
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


                /* ask the user to plug the wrap plug in        */
                rc = diag_display(0x849007,
                        multiport_catd, remove_generic_wrap_plug_mp,
                        DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                        &menutypes, asi_remove_generic_wrap_plug_mp);
                sprintf(msgstr, asi_remove_generic_wrap_plug_mp[0].text,
                                card_name, da_input.dname, da_input.dnameloc);
                free (asi_remove_generic_wrap_plug_mp[0].text);
                asi_remove_generic_wrap_plug_mp[0].text = msgstr;

                sprintf(msgstr1, asi_remove_generic_wrap_plug_mp[1].text,
                        card_name, card_name);
                free (asi_remove_generic_wrap_plug_mp[1].text);
                asi_remove_generic_wrap_plug_mp[1].text = msgstr1;

                rc = diag_display(0x849007, multiport_catd, NULL, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                                asi_remove_generic_wrap_plug_mp);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }
                big_wrap_plug_mp = FALSE;
                putdavar(da_input.dname, "big_wrap_plug_mp",
                DIAG_SHORT, &big_wrap_plug_mp);
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

                rc = diag_display(0x849008, multiport_catd,
                        cable_note_mp, DIAG_MSGONLY,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_cable_note_mp);
                sprintf(msgstr,asi_cable_note_mp[0].text,
                        card_name, da_input.dname, da_input.dnameloc);
                free (asi_cable_note_mp[0].text);
                asi_cable_note_mp[0].text = msgstr;

                /* set the cursor points to NO */
                menutypes.cur_index = NO;
                rc = diag_display(0x849008, multiport_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_cable_note_mp);

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
int cable_menu_testing_4_ports ()
{
        struct msglist cable_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CABLE_PORT_0},
                { 1, CABLE_PORT_1},
                { 1, CABLE_PORT_2},
                { 1, CABLE_PORT_3},
                { 1, CABLE_SELECTION_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_cable_menu[DIAG_NUM_ENTRIES(cable_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;


        int rc;


        rc = diag_display(0x849009, multiport_catd,
                cable_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_cable_menu);
        sprintf(msgstr,asi_cable_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_cable_menu[0].text);
        asi_cable_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[1].text);
        asi_cable_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_cable_menu[2].text);
        asi_cable_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[3].text);
        asi_cable_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[4].text);
        asi_cable_menu[4].text = msgstr4;

        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested))
        {
                sprintf(msgstr10, asi_cable_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_cable_menu[5].text);
                asi_cable_menu[5].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_cable_menu[5].text, " ");
                free (asi_cable_menu[5].text);
                asi_cable_menu[5].text = msgstr10;
        }


        rc = diag_display(0x849009, multiport_catd, NULL, DIAG_IO,
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


        rc = diag_display(0x849009, multiport_catd,
                cable_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_cable_menu);
        sprintf(msgstr,asi_cable_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_cable_menu[0].text);
        asi_cable_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[1].text);
        asi_cable_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_cable_menu[2].text);
        asi_cable_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[3].text);
        asi_cable_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[4].text);
        asi_cable_menu[4].text = msgstr4;

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[5].text);
        asi_cable_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
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
                        (char *)diag_cat_gets ( multiport_catd, 1,
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


        rc = diag_display(0x849009, multiport_catd, NULL, DIAG_IO,
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

        rc = diag_display(0x849009, multiport_catd,
                cable_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_cable_menu);
        sprintf(msgstr,asi_cable_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_cable_menu[0].text);
        asi_cable_menu[0].text = msgstr;

if (EIB_type == 5)
sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_232));

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_cable_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[1].text);
        asi_cable_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_cable_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_cable_menu[2].text);
        asi_cable_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_cable_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[3].text);
        asi_cable_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_cable_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[4].text);
        asi_cable_menu[4].text = msgstr4;

if (EIB_type == 5)
sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_422));

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_cable_menu[5].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_cable_menu[5].text);
        asi_cable_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_cable_menu[6].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[6].text);
        asi_cable_menu[6].text = msgstr6;

        if (port_6_tested)
        {
                sprintf(msgstr7, asi_cable_menu[7].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr7, asi_cable_menu[7].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_cable_menu[7].text);
        asi_cable_menu[7].text = msgstr7;

        if (port_7_tested)
        {
                sprintf(msgstr8, asi_cable_menu[8].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr8, asi_cable_menu[8].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
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
                        (char *)diag_cat_gets ( multiport_catd, 1,
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

        rc = diag_display(0x849009, multiport_catd, NULL, DIAG_IO,
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
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x849011, multiport_catd, insert_generic_cable_mp,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_generic_cable_mp);
        sprintf(msgstr, asi_insert_generic_cable_mp[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_generic_cable_mp[0].text);
        asi_insert_generic_cable_mp[0].text = msgstr;

if (EIB_type == 5)
{
        if (port_number < 4)
        sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_232));
        else
        sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_422));
}

        sprintf(msgstr1, asi_insert_generic_cable_mp[1].text,port_number,
                port_name, port_number,wrap_part_number);
        free (asi_insert_generic_cable_mp[1].text);
        asi_insert_generic_cable_mp[1].text = msgstr1;


        rc = diag_display(0x849011, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_insert_generic_cable_mp);

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
        struct msglist remove_generic_cable_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_WRAP_PLUG_FROM_CABLE_PM             },
                {       0, 0                            }

        };

        static ASL_SCR_INFO
         asi_remove_generic_cable_mp[DIAG_NUM_ENTRIES(remove_generic_cable_mp)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x849012, multiport_catd, remove_generic_cable_mp,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_remove_generic_cable_mp);
        sprintf(msgstr, asi_remove_generic_cable_mp[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_remove_generic_cable_mp[0].text);
        asi_remove_generic_cable_mp[0].text = msgstr;

        sprintf(msgstr1, asi_remove_generic_cable_mp[1].text,port_number);
        free (asi_remove_generic_cable_mp[1].text);
        asi_remove_generic_cable_mp[1].text = msgstr1;


        rc = diag_display(0x849012, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_remove_generic_cable_mp);

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
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x849017, multiport_catd, insert_wrap_after_bad_cable_test_mp,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_wrap_after_bad_cable_test_mp);
        sprintf(msgstr, asi_insert_wrap_after_bad_cable_test_mp[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_wrap_after_bad_cable_test_mp[0].text);
        asi_insert_wrap_after_bad_cable_test_mp[0].text = msgstr;

if (EIB_type == 5)
{
        if (port_number < 4)
        sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_232));
        else
        sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_422));
}

        sprintf(msgstr1, asi_insert_wrap_after_bad_cable_test_mp[1].text,
                port_name, port_number, wrap_part_number, port_number);
        free (asi_insert_wrap_after_bad_cable_test_mp[1].text);
        asi_insert_wrap_after_bad_cable_test_mp[1].text = msgstr1;


        rc = diag_display(0x849017, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_insert_wrap_after_bad_cable_test_mp);

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
        struct msglist ports_note_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, YES_OPTION_PM                      },
                { 1, NO_OPTION_PM                       },
                { 1, PORTS_NOTE_PM                      },
                {       0, 0                    }

        };

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;
        static ASL_SCR_INFO     asi_ports_note_mp[DIAG_NUM_ENTRIES(ports_note_mp)];

                rc = diag_display(0x849013, multiport_catd,
                        ports_note_mp, DIAG_MSGONLY,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                        asi_ports_note_mp);
                sprintf(msgstr,asi_ports_note_mp[0].text,
                        card_name, da_input.dname, da_input.dnameloc);
                free (asi_ports_note_mp[0].text);
                asi_ports_note_mp[0].text = msgstr;

                /* set the cursor points to NO */
                menutypes.cur_index = NO;
                rc = diag_display(0x849013, multiport_catd, NULL, DIAG_IO,
                                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes,
                                asi_ports_note_mp);

                if ((check_asl_stat (rc)) == QUIT)
                {
                        return (QUIT);
                }

                rc = DIAG_ITEM_SELECTED (menutypes);
                return (rc);

}
/******************************************************************/
int     test_ports()
{
        int rc;
        rc = insert_wrap_into_port();
        if (rc != QUIT)
        {
                da_display();
                set_timer ();
                artic_tucb.header.tu = 22 + port_number;
                rc = exectu(filedes, &artic_tucb);
                alarm (0);
                if ((rc != 0) || (alarm_timeout))
                {
                        generic_report_frub(&multiport_port_frus);
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
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        struct msglist insert_wrap_into_port_msg[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, INSERT_GENERIC_WRAP_INTO_PORT      },
                {       0, 0                            }

        };

static ASL_SCR_INFO asi_insert_wrap_into_port_msg[DIAG_NUM_ENTRIES(insert_wrap_into_port_msg)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x849015, multiport_catd, insert_wrap_into_port_msg,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_insert_wrap_into_port_msg);
        sprintf(msgstr, asi_insert_wrap_into_port_msg[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_insert_wrap_into_port_msg[0].text);
        asi_insert_wrap_into_port_msg[0].text = msgstr;


        sprintf(msgstr1, asi_insert_wrap_into_port_msg[1].text,port_number,
                wrap_part_number, port_number);
        free (asi_insert_wrap_into_port_msg[1].text);
        asi_insert_wrap_into_port_msg[1].text = msgstr1;


        rc = diag_display(0x849015, multiport_catd, NULL, DIAG_IO,
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
int port_menu_testing_4_ports ()
{
        struct msglist port_menu[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, CONNECTOR_PORT_0},
                { 1, CONNECTOR_PORT_1},
                { 1, CONNECTOR_PORT_2},
                { 1, CONNECTOR_PORT_3},
                { 1, INTERFACE_CABLE_MESSAGE_PM},
                {       0, 0            }

        };
        static ASL_SCR_INFO asi_port_menu[DIAG_NUM_ENTRIES(port_menu)];
        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;

        int rc;

        rc = diag_display(0x849014, multiport_catd,
                port_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_port_menu);
        sprintf(msgstr,asi_port_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_port_menu[0].text);
        asi_port_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[1].text);
        asi_port_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_port_menu[2].text);
        asi_port_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[3].text);
        asi_port_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[4].text);
        asi_port_menu[4].text = msgstr4;


        /* set the cursor points to NO */
        if (!port_0_tested)
                menutypes.cur_index = 1;
        else if (!port_1_tested)
                menutypes.cur_index = 2;
        else if (!port_2_tested)
                menutypes.cur_index = 3;
        else if (!port_3_tested)
                menutypes.cur_index = 4;

        if ((port_0_tested) || ( port_1_tested) ||
                (port_2_tested) || ( port_3_tested))
        {
                sprintf(msgstr10, asi_port_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1,
                        NOTE_ASTERISK_CABLE_PM));
                free (asi_port_menu[5].text);
                asi_port_menu[5].text = msgstr10;
        }
        else
        {
                sprintf(msgstr10, asi_port_menu[5].text, " ");
                free (asi_port_menu[5].text);
                asi_port_menu[5].text = msgstr10;
        }


        rc = diag_display(0x849014, multiport_catd, NULL, DIAG_IO,
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

        rc = diag_display(0x849014, multiport_catd,
                port_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_port_menu);
        sprintf(msgstr,asi_port_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_port_menu[0].text);
        asi_port_menu[0].text = msgstr;

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[1].text);
        asi_port_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_port_menu[2].text);
        asi_port_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[3].text);
        asi_port_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[4].text);
        asi_port_menu[4].text = msgstr4;


        if (port_4_tested)
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[5].text);
        asi_port_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
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
                        (char *)diag_cat_gets ( multiport_catd, 1,
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


        rc = diag_display(0x849014, multiport_catd, NULL, DIAG_IO,
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
        struct msglist remove_generic_wrap_from_port_mp[]=
        {
                { 1, TESTING_ADVANCED_MESSAGE_PM        },
                { 1, REMOVE_GENERIC_WRAP_FROM_PORT      },
                {       0, 0                            }

        };

        static ASL_SCR_TYPE    menutypes=DM_TYPE_DEFAULTS;
        static ASL_SCR_INFO
         asi_remove_generic_wrap_from_port_mp[DIAG_NUM_ENTRIES(remove_generic_wrap_from_port_mp)];


        /* ask the user to plug the wrap plug in        */
        rc = diag_display(0x849016, multiport_catd, remove_generic_wrap_from_port_mp,
                DIAG_MSGONLY, ASL_DIAG_KEYS_ENTER_SC,
                &menutypes, asi_remove_generic_wrap_from_port_mp);
        sprintf(msgstr, asi_remove_generic_wrap_from_port_mp[0].text,
                card_name, da_input.dname, da_input.dnameloc);
        free (asi_remove_generic_wrap_from_port_mp[0].text);
        asi_remove_generic_wrap_from_port_mp[0].text = msgstr;

        sprintf(msgstr1, asi_remove_generic_wrap_from_port_mp[1].text,port_number);
        free (asi_remove_generic_wrap_from_port_mp[1].text);
        asi_remove_generic_wrap_from_port_mp[1].text = msgstr1;


        rc = diag_display(0x849016, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_KEYS_ENTER_SC, &menutypes,
                asi_remove_generic_wrap_from_port_mp);

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

        rc = diag_display(0x849014, multiport_catd,
                port_menu, DIAG_MSGONLY, ASL_DIAG_LIST_CANCEL_EXIT_SC,
                &menutypes, asi_port_menu);
        sprintf(msgstr,asi_port_menu[0].text, card_name, da_input.dname,
                da_input.dnameloc);
        free (asi_port_menu[0].text);
        asi_port_menu[0].text = msgstr;

if (EIB_type == 5)
sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_232));

        if (port_0_tested)
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                 port_name);
        }
        else
        {
                sprintf(msgstr1, asi_port_menu[1].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[1].text);
        asi_port_menu[1].text = msgstr1;

        if (port_1_tested)
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr2, asi_port_menu[2].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);

        }

        free (asi_port_menu[2].text);
        asi_port_menu[2].text = msgstr2;

        if (port_2_tested)
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr3, asi_port_menu[3].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[3].text);
        asi_port_menu[3].text = msgstr3;

        if (port_3_tested)
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr4, asi_port_menu[4].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[4].text);
        asi_port_menu[4].text = msgstr4;

if (EIB_type == 5)
sprintf (port_name, (char *)diag_cat_gets ( multiport_catd, 1, RS_422));

        if (port_4_tested)
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr5, asi_port_menu[5].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }

        free (asi_port_menu[5].text);
        asi_port_menu[5].text = msgstr5;

        if (port_5_tested)
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr6, asi_port_menu[6].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[6].text);
        asi_port_menu[6].text = msgstr6;

        if (port_6_tested)
        {
                sprintf(msgstr7, asi_port_menu[7].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr7, asi_port_menu[7].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
                        port_name);
        }
        free (asi_port_menu[7].text);
        asi_port_menu[7].text = msgstr7;

        if (port_7_tested)
        {
                sprintf(msgstr8, asi_port_menu[8].text,
                        (char *)diag_cat_gets ( multiport_catd, 1, TESTED_PM),
                        port_name);
        }
        else
        {
                sprintf(msgstr8, asi_port_menu[8].text,
                  (char *)diag_cat_gets ( multiport_catd, 1, NOT_TESTED_PM),
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
                        (char *)diag_cat_gets ( multiport_catd, 1,
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


        rc = diag_display(0x849014, multiport_catd, NULL, DIAG_IO,
                ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, asi_port_menu);

        if ((check_asl_stat (rc)) == QUIT)
                return(QUIT);

        rc = DIAG_ITEM_SELECTED (menutypes);
        return (rc);
}

/*--------------------------------------------------------------------------
| NAME: report_frub_mp_cable
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

void report_frub_mp_cable ()
{

        struct fru_bucket *frub_addr;

        frub_addr = &multiport_cable_frus[0];

        strcpy ( frub_addr->dname, da_input.dname);
        frub_addr->sn = 0x849;

        insert_frub(&da_input, frub_addr);
        /* the reason why there is another switch because insert_frub   */
        /* will reset frub_addr->sn                                     */

        frub_addr->sn = 0x849;
        addfrub(frub_addr);
        DA_SETRC_STATUS (DA_STATUS_BAD);
}
