static char sccsid[] = "@(#)34  1.40  src/bos/diag/da/dial/ddials.c, dadial, bos41J, 9525E_all 6/21/95 09:23:58";
/*
 *   COMPONENT_NAME: DADIAL
 *
 *   FUNCTIONS: CHAR_TO_INT
 *              DISPLAY_MENU
 *              ERROR_RANGE
 *              EXECUTE_TU_WITHOUT_MENU
 *              EXECUTE_TU_WITH_MENU
 *              IS_TM
 *              PUT_EXIT_MENU
 *              PUT_INSTRUCTION
 *              PUT_QUEST_MENU
 *              chg_mn_adv
 *              chk_all_dial
 *              chk_asl_stat
 *              chk_ela
 *              chk_terminate_key
 *              clean_up
 *              connect_to_gio
 *              dial_position
 *              dial_screen
 *              dial_screen2
 *              dial_sub_screen2
 *              error_other
 *              initialize_all
 *              insert_wrap_menu
 *              instruction_menu
 *              int_handler
 *              main
 *              put_title
 *              question_menu
 *              rand_dial_chk
 *              remove_wrap_menu
 *              report_fru
 *              retry_time
 *              screen_update
 *              set_timer
 *              timeout
 *              tu_test
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include        <stdio.h>
#include        <fcntl.h>
#include        <nl_types.h>
#include        <memory.h>
#include        <sys/types.h>
#include        <sys/cfgodm.h>
#include        <cur00.h>
#include        <cur02.h>
#include        <signal.h>
#include        <setjmp.h>
#include        <errno.h>

#include        "diag/diago.h"
#include        "diag/tm_input.h"
#include        "diag/tmdefs.h"
#include        "diag/da.h"
#include        "diag/diag_exit.h"
#include        "diag/atu.h"
#include        "ddials_msg.h"
#include        "dial.h"
#include        "diag/diag.h"
#include        <locale.h>

char    device_name[64];


/*------------
  macro to convert a 4-byte character to an integer
  ------------*/
#define CHAR_TO_INT(x)  x[0]<<24 | x[1]<<16 | x[2]<<8 | x[3]

/**************************************************************************/
/*                                                                        */
/* IS_TM is a macro takes two input variables VAR1 & VAR2.                */
/* VAR1 is an object of the sturcture tm_input.                           */
/* VAR2 is a variable or a defined value will be compared to the tm_input */
/*      object class.                                                     */
/*                                                                        */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/*                                                                        */
/* WRP_CBL_NOT_EXEC: macro define all the conditions where device cable   */
/*                   wrap test unit will not excute.                      */
/*                                                                        */
/**************************************************************************/

#define WRP_CBL_NOT_EXEC (              IS_TM( console, CONSOLE_FALSE   ) \
                                ||      IS_TM( system, SYSTEM_TRUE      ) \
                         )

#define EXECUTE_CBL_WRP ( ! ( WRP_CBL_NOT_EXEC ) )


/**************************************************************************/
/*                                                                        */
/* D_ORDER_COMP_NOT_EXEC: macro define all the conditions where testing   */
/*                        DIALs ordered compare dial will not execute.    */
/*                                                                        */
/**************************************************************************/

#define D_ORDER_COMP_NOT_EXEC (         IS_TM( console, CONSOLE_FALSE   ) \
                                ||      IS_TM( system, SYSTEM_TRUE      ) \
                              )

#define EXECUTE_ORDER_COMP ( ! ( D_ORDER_COMP_NOT_EXEC ) )

/**************************************************************************/
/*                                                                        */
/* RAND_DIAL_COMP_NOT_EXEC: macro define all the conditions where testing */
/*                          DIALs random compare dial will not execute    */
/*                                                                        */
/**************************************************************************/

#define RAND_DIAL_COMP_NOT_EXEC (       IS_TM( console, CONSOLE_FALSE   ) \
                                   ||   IS_TM( system, SYSTEM_TRUE      ) \
                                )

#define EXECUTE_RANDOM_COMP ( ! (RAND_DIAL_COMP_NOT_EXEC ) )

/**************************************************************************/
/*                                                                        */
/* MORE_RESOURCE is a macro return TRUE if one of the test units did not  */
/*               have the privlige to execute.                            */
/*                                                                        */
/**************************************************************************/

#define MORE_RESOURCE    (!( IS_TM(loopmode, LOOPMODE_EXITLM) ))

/**************************************************************************/
/*                                                                        */
/* EXECUTE_TU_WITH_MENU is a macro defines conditions for the Test Units  */
/*                      who will have a menu to display. The macro takes  */
/*                      one parameter TU_NUM is the test unit number.     */
/*                                                                        */
/**************************************************************************/

#define EXECUTE_TU_WITH_MENU(TU_NUM)                                       \
        (                                                                  \
                (                                                          \
                        IS_TM( loopmode, LOOPMODE_NOTLM     )              \
                   ||   IS_TM( loopmode, LOOPMODE_ENTERLM   )              \
                )                                                          \
                &&                                                         \
                    (   ( tmode[/**/TU_NUM].wrapflg == TRUE       )        \
                      ||( tmode[/**/TU_NUM].wrapflg == FALSE      )        \
                      ||( tmode[/**/TU_NUM].mflg & INST )                  \
                      ||( tmode[/**/TU_NUM].mflg & INST_QUEST )            \
                    )                                                      \
                &&      ( IS_TM( console, CONSOLE_TRUE      )   )          \
        )

/**************************************************************************/
/*                                                                        */
/* EXECUTE_TU_WITHOUT_MENU is a macro define execute conditions for TUs   */
/*                         without menus. The macro takes one paramater   */
/*                         TU_NUM is the test unit number.                */
/*                                                                        */
/**************************************************************************/

#define EXECUTE_TU_WITHOUT_MENU( TU_NUM )                                  \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM  )         \
                           ||   IS_TM( loopmode, LOOPMODE_ENTERLM)         \
                           ||   IS_TM( loopmode, LOOPMODE_INLM   )         \
                        )                                                  \
                        &&      ( tmode[/**/TU_NUM].mflg == FALSE )        \
                )

/**************************************************************************/
/*                                                                        */
/* DISPLAY_MENU is a macro define when a menu should be dispalyed. The    */
/*              macro takes one paramater TU_NUM is the test unit number  */
/*                                                                        */
/**************************************************************************/

#define DISPLAY_MENU( TU_NUM )                                             \
        (                                                                  \
                (                                                          \
                        IS_TM( loopmode, LOOPMODE_NOTLM   )                \
                   ||   IS_TM( loopmode, LOOPMODE_ENTERLM )                \
                )                                                          \
                &&      ( tmode[/**/TU_NUM].mflg == TRUE  )                \
                &&      ( IS_TM( system, SYSTEM_FALSE)    )                \
                &&      ( IS_TM( console, CONSOLE_TRUE)   )                \
        )

/**************************************************************************/
/*                                                                        */
/* PUT_EXIT_MENU is a macro define when it has to display an exit menu if */
/*               the test unit have displayed a previous menu. This macro */
/*               takes one paramater is the Test unit number.             */
/*                                                                        */
/**************************************************************************/

#define PUT_EXIT_MENU( TU_NUM )                                            \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM     )      \
                        ||      IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                        )                                                  \
                        && ( tmode[/**/TU_NUM].mflg == TRUE       )        \
                        && ( tmode[/**/TU_NUM].wrapflg == TRUE    )        \
                        && ( IS_TM( system, SYSTEM_FALSE)         )        \
                        && ( IS_TM( console, CONSOLE_TRUE)        )        \
                )

/**************************************************************************/
/*                                                                        */
/* PUT_INSTRUCTION is a macro define when instruction will be displayed.  */
/*                 Istruction menu will be displayed before executing the */
/*                 test unit, instruct the user to do something while the */
/*                 test is running.                                       */
/*                                                                        */
/**************************************************************************/

#define PUT_INSTRUCTION( TU_NUM )                                          \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM     )      \
                        ||      IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                        )                                                  \
                        && ( tmode[/**/TU_NUM].mflg & INST        )        \
                        && ( IS_TM( system, SYSTEM_FALSE)         )        \
                        && ( IS_TM( console, CONSOLE_TRUE)        )        \
                )

/**************************************************************************/
/*                                                                        */
/* PUT_QUEST_MENU is a macro define when question menu will be displayed  */
/*                Menus will be displayed after executing the test unit,  */
/*                to verify status of device.                             */
/*                                                                        */
/**************************************************************************/

#define PUT_QUEST_MENU( TU_NUM )                                           \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM     )      \
                        ||      IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                        )                                                  \
                        && ( tmode[/**/TU_NUM].mflg & QUEST       )        \
                        && ( IS_TM( system, SYSTEM_FALSE)         )        \
                        && ( IS_TM( console, CONSOLE_TRUE)        )        \
                )

/**************************************************************************/
/*                                                                        */
/* DISPLAY_TESTING is a macro define when a title line would be displayed */
/*                                                                        */
/**************************************************************************/

#define DISPLAY_TESTING                                                    \
                (                                                          \
                        (                                                  \
                                IS_TM( loopmode, LOOPMODE_NOTLM   )        \
                           ||   IS_TM( loopmode, LOOPMODE_ENTERLM )        \
                        )                                                  \
                        && ( IS_TM( console, CONSOLE_TRUE         )   )    \
                )

/**************************************************************************/
/*                                                                        */
/* DISPLAY_LOOP_COUNT is a macro define when loop count will be displayed */
/*                                                                        */
/**************************************************************************/

#define DISPLAY_LOOP_COUNT                                                 \
                                (                                          \
                                      (                                    \
                                        IS_TM( system,SYSTEM_FALSE )       \
                                    &&  (                                  \
                                        IS_TM( loopmode,LOOPMODE_INLM )    \
                                     || IS_TM( loopmode,LOOPMODE_EXITLM )  \
                                        )                                  \
                                    &&  IS_TM( console,CONSOLE_TRUE )      \
                                      )                                    \
                                ||    (                                    \
                                        IS_TM( system,SYSTEM_TRUE )        \
                                    && !(IS_TM( loopmode,LOOPMODE_NOTLM )) \
                                    &&  IS_TM( console,CONSOLE_TRUE )      \
                                      )                                    \
                                )

/**************************************************************************/
/*                                                                        */
/*      check the return code the Test Unit if it is in a valid range     */
/*      logical return TRUE or FALSE                                      */
/*                                                                        */
/**************************************************************************/

#define ERROR_RANGE( VAR )(                                                \
                                ( /**/VAR > 0 )                            \
                             &&                                            \
                                ( /**/VAR <= tmode[ar_index].high_range ) \
                          )

/**************************************************************************/
/*                                                                        */
/*      da_title is an array of structure holds the message set           */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_msg to tell the user        */
/*      Dials diagnostics is in testing.                                  */
/*                                                                        */
/**************************************************************************/

struct  msglist da_title[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_GENERIC,        DIAL_STND_BY    },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      wrp_yes_no is an array of structure holds the message set         */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_display to ask the user     */
/*      for a wrap plug.                                                  */
/*                                                                        */
/**************************************************************************/

struct  msglist wrp_yes_no[]=
                {
                        {  DIAL_WRP_NUM,        DIAL_WRP_TITLE  },
                        {  DIAL_GENERIC,        DIAL_YES        },
                        {  DIAL_GENERIC,        DIAL_NO         },
                        {  DIAL_GENERIC,        DIAL_ACTION     },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      rmv_cable is an array of structure holds the message set          */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure used to ask the user to remove the cable           */
/*      if attached, from the dial device and to plug a  wrap             */
/*      plug into the Dials cable.                                        */
/*                                                                        */
/**************************************************************************/


struct  msglist rmv_cable[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_RMV_ATTACHEMENT,DIAL_UNPLUG_CBL },
                        {  DIAL_RMV_ATTACHEMENT,DIAL_PLUG_WRP   },
                        {  DIAL_GENERIC,        DIAL_PRS_ENT    },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      connect_cbl is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to ask         */
/*      the user to remove the wrap plug and to reconnect the cable       */
/*      into the dial device.                                             */
/*                                                                        */
/**************************************************************************/

struct  msglist connect_cbl[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_RECONNECT,      DIAL_UNPLUG_WRP },
                        {  DIAL_RECONNECT,      DIAL_PLUG_CBL   },
                        {  DIAL_GENERIC,        DIAL_PRS_ENT    },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      order_w_inst is an array of structure holds the message set       */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to instruct    */
/*      the user to turn each dial to the right and then to left.         */
/*                                                                        */
/**************************************************************************/

struct  msglist order_w_inst[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_ORD,            DIAL_ORD_INST   },
                        {  DIAL_GENERIC,        DIAL_INST_PRS_ENT       },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      order_inst is an array of structure holds the message set         */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to instruct    */
/*      the user to turn each dial to the right, to the left or to stop   */
/*      testing press ESC or F3 to exit.                                  */
/*                                                                        */
/**************************************************************************/

struct  msglist order_inst[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_ORD,            DIAL_ORD_EXITA  },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      order_quest is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to ask the     */
/*      user if dials worked properly.                                    */
/*                                                                        */
/**************************************************************************/

struct  msglist order_quest[]=
                {
                        {  DIAL_ORD,            DIAL_ORD_QUEST  },
                        {  DIAL_GENERIC,        DIAL_YES        },
                        {  DIAL_GENERIC,        DIAL_NO         },
                        {  DIAL_GENERIC,        DIAL_ACTION     },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      random_inst is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/**************************************************************************/

struct  msglist random_w_inst[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_RND,            DIAL_RND_INST   },
                        {  DIAL_GENERIC,        DIAL_INST_PRS_ENT       },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      random_inst is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

struct  msglist random_inst[]=
                {
                        {  DIAL_GENERIC,        DIAL_TITLE      },
                        {  DIAL_RND,            DIAL_RND_EXITA  },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      order_quest is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to ask the     */
/*      user if dials worked properly.                                    */
/*                                                                        */
/**************************************************************************/

struct  msglist random_quest[]=
                {
                        {  DIAL_RND,            DIAL_RND_QUEST  },
                        {  DIAL_GENERIC,        DIAL_YES        },
                        {  DIAL_GENERIC,        DIAL_NO         },
                        {  DIAL_GENERIC,        DIAL_ACTION     },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/*      random_inst is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

struct  msglist misc_dial[]=
                {
                        {  DIAL_MISC,   DIAL_NAME       },
                        {  DIAL_MISC,   DIAL_ROT        },
                        {  DIAL_MISC,   DIAL_CLK        },
                        {  DIAL_MISC,   DIAL_CNT_CLK    },
                        {  DIAL_MISC,   DIAL_BLANK      },
                        {  DIAL_MISC,   DIAL_PLUS       },
                        (int)NULL
                };

/**************************************************************************/
/*                                                                        */
/* da_info structure holds information related to DA.                     */
/* variables in the structure:                                            */
/*                                                                        */
/* tu_num : test unit number, cause an execution of a particular test.    */
/*                                                                        */
/*                                                                        */
/* mflg   : TRUE, FALSE or number of menus could be displayed in one      */
/*          TEST.                                                         */
/*                                                                        */
/* wrapflg: TRUE, FALSE values indicate if the user inserted a wrap       */
/*          plug on the NIO Card.                                         */
/*                                                                        */
/*                                                                        */
/* high_rc : high range of TU return code, high used to keep track of     */
/*           invalid return code.                                         */
/*                                                                        */
/**************************************************************************/

struct da_info
{
        int     tu_num;         /* tu test number.                        */
        int     mflg;           /* display a menu to the user when this   */
                                /* flag is greater then zero.             */
        int     wrapflg;        /* wrap plug flag to indicate if the user */
                                /* inserted a wrap plug                   */
        int     high_range;     /* High range of the error number returned*/
                                /* from the test Unit                     */
};

/**************************************************************************/
/*                                                                        */
/*    assign  structured array of size equal to the number of possible    */
/*    tests.                                                              */
/*                                                                        */
/**************************************************************************/

struct da_info tmode[] =
{
        { DIAL_CBL_WRP  , TRUE     , FALSE, MAX_CBL_WRP         },
        { DIAL_INT_WRAP , FALSE    , FALSE, MAX_INT_WRAP        },
        { DIAL_ORD_COMP , INST_QUEST, FALSE, MAX_ORD_COMP       },
        { DIAL_RND_COMP , INST_QUEST, FALSE, MAX_RND_COMP       },
        { DIAL_RESET    , FALSE    , FALSE, MAX_RESET           },
};

/**************************************************************************/
/*                                                                        */
/*      frub_wrp_cbl struct holds related information to FRU bucket       */
/*      variables in this structure are explained in DA CAS (SJDIAG)      */
/*      under the function name ADDFRU.                                   */
/*      One of these frus will be reported if a failure return code       */
/*      returned from the test unit.                                      */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_wrp_cbl[]=
{

        { "", FRUB1,  0x929, 0x121, DIAL_COM_FAILED,
                        {
                                { 100 , "Cable" , "", DIAL_CABLE,
                                        0, EXEMPT },
                        },
        },

};

/* this struct for dial connected to async port  */

struct fru_bucket frub_wrp_cbl_async[]=
{

        { "", FRUB1,  0x929, 0x122, DIAL_COM_FAILED,
                        {
                                { 90 , "" , "", 0,
                                        PARENT_NAME, (char)NULL},
                                { 10 , "Cable" , "", DIAL_CABLE,
                                        0, EXEMPT },
                        },
        },

};

/**************************************************************************/
/*                                                                        */
/*      frub_bat_code struct holds related information to FRU bucket      */
/*      variables in this structure are explained in DA CAS (SJDIAG)      */
/*      under the function name ADDFRU.                                   */
/*      One of these frus will be reported if a failure return code       */
/*      returned from the test unit.                                      */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_bat_code[]=
{

        { "", FRUB1,  0x929, 0x131, DIAL_RESET_FAILED,
                        {
                                { 90, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 10 , "Cable" , "", DIAL_CABLE,
                                        0, EXEMPT },
                        },
        },
};

/* this fru bucket for dial connected to async port */

struct fru_bucket frub_bat_code_async[]=
{

        { "", FRUB1,  0x929, 0x132, DIAL_LCL_WRAP,
                        {
                                { 50, "Power Supply" , "" , DIAL_POW,
                                        0, (char)NULL   },
                                { 45, "" , "", DIAL_FAILED,
                                        DA_NAME, (char)NULL     },
                                { 5 , "Cable" , "", DIAL_CABLE,
                                        0, EXEMPT },
                        },
        },
};

/**************************************************************************/
/*                                                                        */
/*      frub_dial struct holds related information to FRU bucket          */
/*      variables in this structure are explained in DA CAS (SJDIAG)      */
/*      under the function name ADDFRU.                                   */
/*      One of these frus will be reported if a failure return code       */
/*      returned from the test unit.                                      */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_dial[]=
{

        { "", FRUB1,  0x929, 0x141, DIAL_FAILED,
                        {
                                { 100, "" , "" , 0, DA_NAME, (char)NULL },
                        },
        },
};

/**************************************************************************/
/*      this fru is issued when configuration for devices fails           */
/**************************************************************************/
struct fru_bucket config_fail_frub[]=
{

        { "", FRUB1,  0x929, 0x800, DIAL_CONFIG_FAIL,
                        {
                                { 80, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 20, "" , "" , 0, PARENT_NAME,  (char)NULL},
                        },
        },
};
struct fru_bucket config_fail_frub_gio[]=
{

        { "", FRUB1,  0x929, 0x801, DIAL_CONFIG_FAIL,
                        {
                                { 80, "" , "" , 0, PARENT_NAME,  (char)NULL},
                                { 10, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 10, "" , "" , DIAL_SOFTWARE_ERROR,
                                                        0,      NONEXEMPT},
                        },
        },
};
/**************************************************************************/
/*      this fru is issued when opening device failure with EIO or ENODEV */
/*      or EXIO                                                           */
/**************************************************************************/
struct fru_bucket open_fail_frub[]=
{

        { "", FRUB1,  0x929, 0x802, DIAL_OPEN_ERROR,
                        {
                                { 80, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 10, "" , "" , 0, PARENT_NAME,  (char)NULL},
                                { 10, "" , "" , DIAL_SOFTWARE_ERROR,
                                                        0,      NONEXEMPT},
                        },
        },
};

struct fru_bucket open_fail_frub_gio[]=
{

        { "", FRUB1,  0x929, 0x803, DIAL_OPEN_ERROR,
                        {
                                { 80, "" , "" , 0, PARENT_NAME,  (char)NULL},
                                { 10, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 10, "" , "" , DIAL_SOFTWARE_ERROR,
                                                        0,      NONEXEMPT},
                        },
        },
};

struct fru_bucket software_frub[]=
{

        { "", FRUB1,  0x929, 0x804, DIAL_CONFIG_FAIL,
                        {
                                { 80, "" , "" , DIAL_SOFTWARE_ERROR,
                                                        0,      NONEXEMPT},
                                { 10, "" , "" , 0, DA_NAME,      (char)NULL},
                                { 10, "" , "" , 0, PARENT_NAME,  (char)NULL},
                        },
        },
};



/***********************************************************************/
struct tucb_data {
        struct tucb_t   header; /* Standard TU input.             */
        struct {
                    int error;          /* Error code of failure. 0=OK    */
                    int key;            /* Key code which terminated      */
                                        /* an interactive test. (output)  */
                    char *error_msg;    /* Pointer to message associate   */
                                        /* with error.                    */
                }       gio;
        struct {                /* For TU 140 and 150             */
                    int granularity;    /* Granularity to set. (input)    */
                    int dial_no;        /* Dial which was turned. (output)*/
                    int dir_amt;        /* Direction/amount of turn.      */
                                        /* (output) +/- for direction.    */
                } dial;
        };


WINDOW  *dial_screen();
ASL_SCR_INFO    w_uinfo[(sizeof(misc_dial))];
struct  tm_input        tm_input;

int     filedes = ERR_FILE_OPEN;/* Pointer to the device address          */

nl_catd catd = CATD_ERR;        /* pointer to the catalog file            */
void    int_handler(int);
void    retry_time(int);
void    timeout(int);
void    set_timer();
nl_catd diag_catopen();
int     device_type=0;
int     device_state=0;
int     dial_state=0;
ulong   curpd = -1;

struct
{
        int test_unit;
        int direction;
        int dial_number;
} set_dial;

int     tu_open_flag=FALSE;
void    sw_timeout(int);
void    clean_malloc();
int     alarm_timeout=FALSE;
struct  sigaction  invec;       /* interrupt handler structure  */

/* This section for 4.1 where ucfgdiag and cfgdiag is implemented and diagex */
char    *msgstr;
char    *msgstr1;
char    option[256];
char    *new_out, *new_err;
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
char    *dial_configure_method;
char    *dial_unconfigure_method;
char    *lpfk_device_configure_method;
char    *lpfk_device_unconfigure_method;
char    *sa_device_configure_method;
char    *sa_device_unconfigure_method;
char    *tty_device_configure_method;
char    *tty_device_unconfigure_method;
struct  PdDv    *pddv_p;
struct  CuDv    *cudv_p;
int     unconfigure_lpp=FALSE;
int     dial_unconfigure_lpp=FALSE;
int     lpfk_device_unconfigure_lpp=FALSE;
int     tty0_device_unconfigure_lpp=FALSE;
int     tty1_device_unconfigure_lpp=FALSE;
int     sa0_device_unconfigure_lpp=FALSE;
int     sa1_device_unconfigure_lpp=FALSE;
int     serial_gio=FALSE;
char    dial[16];
char    lpfk_device[16];
char    sa0_device[16];
char    sa1_device[16];
char    tty0_device[16];
char    tty1_device[16];
char    gio_device_name[16];
int     set_device_to_diagnose = FALSE;
int     gio_device_state = -1;

/*
 * NAME: main()
 *
 * FUNCTION: Main driver for Dial Diagnostic applications.
 *           This function will call several functions depends on
 *           the environment, such as initializing and executing
 *           several TU's depend on the environment.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS:  (NONE)
 */

main(argc,argv)
int     argc;
char    *argv[];
{
        int     da_errno = 0;
        int     rc;
        struct sigaction act;
        extern  int     errno ;


        setlocale (LC_ALL, "");


        /* set up interrupt handler     */
        act.sa_handler = int_handler;
        sigaction(SIGINT, &act, (struct sigaction *)NULL);

        initialize_all();

        if ( DISPLAY_LOOP_COUNT )
                chk_asl_stat(diag_msg_nw(0x929110,catd,DIAL_INLOOP,
                        DIAL_INLOOP_TITLE, tm_input.lcount,tm_input.lerrors));

        else if ( DISPLAY_TESTING )
                put_title(0x929001);

        if ( IS_TM(dmode,DMODE_ELA))
                chk_ela();
        else
        {
                unconfigure_lpp_device();

                /**********************************************************/
                /*              Run all the necessary tests               */
                /**********************************************************/
                /* opening tu to test   */
                rc = tu_open (tm_input.dname);
                if (rc != 0)
                {
                        report_fru(&software_frub);
                }
                else
                        tu_open_flag=TRUE;

                if (rc != ERR_FILE_OPEN)
                {
                        if (EXECUTE_CBL_WRP)    /* TU #120                */
                        {
                                if (connect_to_gio ())
                                {
                                     tu_test(IND_CBL_WRP,frub_wrp_cbl, 0);
                                }
                                /* Serial port test 120 is not run      */
                        }

                        if (connect_to_gio())
                        {
                                /* TU # 210     */
                                tu_test(IND_RESET,frub_wrp_cbl, 0);
                                /* TU # 130     */
                                tu_test(IND_WRP_TST,frub_bat_code, 0);
                        }
                        else
                        {
                                tu_test(IND_WRP_TST,frub_bat_code_async, 0);
                                /* for dial connected to serial port,
                                test IND_WRP_TST is not run     */
                        }

                        if ( IS_TM( console, CONSOLE_TRUE))
                                diag_display(0x929033,catd,misc_dial,
                                DIAG_MSGONLY,NULL, NULL,w_uinfo);

                        if (EXECUTE_ORDER_COMP)
                        {
                                /* Test Unit 150        */
                                tu_test(IND_RANDOM_COMP,frub_dial, 2);
                        }
                }

                if ( IS_TM ( dmode,DMODE_PD ))
                        chk_ela();

                if ( MORE_RESOURCE )
                        DA_SETRC_TESTS( DA_TEST_NOTEST );
                else
                        DA_SETRC_TESTS( DA_TEST_FULL );
                clean_up( );

        }
}

/*
 * NAME: tu_test()
 *
 * FUNCTION: Designed to execute the test units if the test unit has
 *           the correct environment mode. If the test unit excuted
 *           without any error returned then this function will return
 *           to the calling function otherwise this function will report
 *           to the diagnostic controller a FRU bucket failure indicat
 *           the failing problem and the program will stop the execution.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * NOTE: tu_test will not return to the caller function if test unit
 *       failed.
 *
 * RETURNS: NONE
 */

tu_test ( ar_index, fru_buckets, granularity )
int     ar_index;
struct  fru_bucket      fru_buckets[];
int granularity;        /* Granularity to set. (input)                    */
{
        struct  tucb_data       dial_tucb;
        int     rc = 0;
        int     dialnum = 0;
        int     error_num = FALSE;      /* error number from the test unit*/
        WINDOW  *w;
        extern  exectu();



        (void) memset (&dial_tucb, 0, sizeof(dial_tucb));

        dial_tucb.gio.key = -1;
        dial_tucb.header.tu = tmode[ar_index].tu_num; /* Assign TU number */
        dial_tucb.header.r1 = 0x10;     /* DIALS is define in gio.h     */
        dial_tucb.header.loop = 1;
        dial_tucb.dial.granularity = granularity;

        /* if test unit 120 then we have to pass gio?   */
        /* else need to pass dial?                      */

        if (dial_tucb.header.tu == 120)
                strcpy (device_name, tm_input.parent);
        else
                strcpy (device_name, tm_input.dname);

        if (dial_tucb.header.tu == 150)
        {

                dial_tucb.header.tu = 140;
                dial_tucb.gio.key = 0;
                dial_tucb.gio.error = 0;
                dial_tucb.dial.dial_no = 0;
                dial_tucb.dial.dir_amt = 0;
                dial_tucb.dial.granularity= 8;
                rc = exectu (device_name, &dial_tucb);
                dial_tucb.header.tu = 150;
        }

        if ( PUT_INSTRUCTION(ar_index) )
                instruction_menu( ar_index);

        if ( DISPLAY_MENU(ar_index) )
        {
                rc = insert_wrap_menu( ar_index);
                if (rc != YES)
                {
                        return (0);
                }
        }

        if (    EXECUTE_TU_WITH_MENU(ar_index)
             || EXECUTE_TU_WITHOUT_MENU(ar_index) )
        {
                switch(dial_tucb.header.tu)
                {
                        case DIAL_ORD_COMP:             /* Test Unit 140 */
                                error_num = chk_all_dial(&dial_tucb);
                                break;
                        case DIAL_RND_COMP:             /* Test Unit 150 */
                                error_num = rand_dial_chk(&dial_tucb);
                                if (question_menu (ar_index) == -1)
                                {
                                        report_fru(&frub_dial);
                                }
                                break;
                        default:
                                /* Non interactive tests                   */
                                set_timer ();
                                error_num = exectu (device_name, &dial_tucb);
                                alarm (0);
                                break;
                }
                if ((error_num == -1) && (dial_tucb.gio.error== 10))
                {

                        if ( PUT_EXIT_MENU( ar_index ) )
                                remove_wrap_menu( ar_index );
                        report_fru(&software_frub);
                }
                if(dial_tucb.gio.key != -1)
                        chk_asl_stat(dial_tucb.gio.key);

        }

        if ( PUT_EXIT_MENU( ar_index ) )
                remove_wrap_menu( ar_index );

        PUT_QUEST_MENU( ar_index );

        /* checkin the error return from test unit                      */
        /* The error from test 150 is checked after ask the user        */
        /* if the display is good                                       */
        if (( error_num == -1 ) && (dial_tucb.header.tu != 150))
        {
                switch (dial_tucb.header.tu )
                {
                        case 120:
                                if (verify_rc (&tu120[0], dial_tucb.gio.error))
                                {
                                    if ((dial_tucb.gio.error & 0xFFFFFFFF) == 1)
                                    {
                                        /*Cable wrap test failed */
                                        if (connect_to_gio())
                                            report_fru(&frub_wrp_cbl_async[0]);
                                        else
                                            report_fru(&frub_wrp_cbl[0]);
                                    }
                                }
                                break;
                        case 130:
                                if (verify_rc (&tu130[0], dial_tucb.gio.error))
                                {
                                    if ((dial_tucb.gio.error & 0xFFFFFFFF) == 1)
                                    {
                                        /*Cable wrap test failed */
                                        if (connect_to_gio())
                                            report_fru(&frub_wrp_cbl_async[0]);
                                        else
                                            report_fru(&frub_wrp_cbl[0]);
                                    }
                                }
                                break;
                        case 210:
                                if (verify_rc (&tu210[0], dial_tucb.gio.error))
                                {
                                        /*Cable wrap test failed */
                                        if (connect_to_gio())
                                            report_fru(&frub_bat_code_async[0]);
                                        else
                                            report_fru(&frub_bat_code[0]);
                                }
                                break;
                        default:
                                break;
                }
                clean_up();
        }
        chk_terminate_key();

}

/**************************************************************************/


chk_ela()
{
#if     0
        /*       check if da_mode bit if it has a ELA request               */
        struct  errdata err_data;
        char    *crit="-N gio -T PERM -d H";
        int     rc=0;
        int     error_num=0;

        (void) memset(&err_data,0,sizeof(err_data));
        error_num = error_log_get(INIT,crit,&err_data);
        for (;err_data.err_id != 0xb9c0107e &&  error_num >0;)
                error_num = error_log_get(SUBSEQ,crit,&err_data);


        error_num = error_log_get(TERMI,crit,&err_data);
        if(err_data.err_id == 0xb9c0107e)
                report_fru(&ela_frub[0]);

#endif
}

/*
 * NAME: clean_up()
 *
 * FUNCTION:  exit from the DA and return an appropriate return
 *            code to the Daignostic controller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

clean_up()
{
        if (tu_open_flag )
        {
                tu_close ();
                tu_open_flag= FALSE;
        }

        configure_lpp_device();
        clean_malloc();

        term_dgodm();

        if (catd != CATD_ERR)
                catclose( catd );
        if ( IS_TM( console, CONSOLE_TRUE ) )   /* Check if Console TRUE */
                diag_asl_quit(NULL);
        DA_EXIT();
}

/*
 * NAME: insert_wrap_menu()
 *
 * FUNCTION: Designed to ask the user on the first pass only, if the user
 *           have a wrap plug, if he selected yes then the user must
 *           remove the cable from the dial device and insert a wrap
 *           plug, otherwise return to the caller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: 0, -1
 */
int
insert_wrap_menu(index)
int     index;
{
        ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

        if (IS_TM( advanced, ADVANCED_FALSE))
                menutypes.cur_index = FALSE;
        else
        {
                menutypes.cur_index=1;
                chk_asl_stat(diag_display(0x929003,catd,wrp_yes_no, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
        }

        if ( menutypes.cur_index != TRUE )
        {
                /* NO wrap plug */
                put_title(0x929004);
                tmode[index].wrapflg = FALSE;
                return (0);
        }
        chk_asl_stat(diag_display(0x929004,catd,rmv_cable, DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
        put_title(0x929004);

        tmode[index].wrapflg = TRUE;

        return(YES);
}

/*
 * NAME: remove_wrap_menu()
 *
 * FUNCTION: Designed to tell the user to remove the wrap plug from the
 *           cable end and to reconnect the cable to the dial device.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

remove_wrap_menu(index)
int     index;
{
        chk_asl_stat(diag_display(0x929005,catd,connect_cbl,DIAG_IO,
                        ASL_DIAG_NO_KEYS_ENTER_SC, NULL,NULL));
        diag_asl_beep();

        put_title(0x929005);

        tmode[index].wrapflg = FALSE;
}

/*
 * NAME: instruction_menu()
 *
 * FUNCTION: Designed to give the user instructions only when running
 *           certain test units.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
instruction_menu(index)
int     index;
{
        switch (tmode[index].tu_num)
        {
                case DIAL_RND_COMP: /* display when running TU #150        */
                case DIAL_ORD_COMP: /* display when running TU #140        */
                        chk_asl_stat(diag_display(0x929006,catd,order_w_inst,
                                DIAG_IO,ASL_DIAG_KEYS_ENTER_SC , NULL,NULL));
                        sleep(1);
                        chk_asl_stat(diag_display(0x929007,catd,order_inst,
                                DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );
                        diag_asl_beep();
                        break;

                default:
                        tmode[index].wrapflg= FALSE;
        };
}

/*
 * NAME: question_menu()
 *
 * FUNCTION: Designed to check if all the Dials worked correctly
 *           when the dial was turned cursor postion indicated the
 *           movement in the desired direction
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
question_menu(index)
int     index;
{
        int     wait=TRUE;

        ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS ;
        menutypes.cur_index=1;
        switch (tmode[index].tu_num)
        {
                case DIAL_RND_COMP:
                case DIAL_ORD_COMP:
                        chk_asl_stat(diag_display(0x929005,catd,order_quest,
                        DIAG_IO,ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                        break;
                default:
                        return(0);

        };

        if ( menutypes.cur_index == TRUE )
        {
                put_title(0x929011);
                return (0);
        }
        return(-1);
}

/*
 * NAME: timeout()
 *
 * FUNCTION: Desined to handle timeout interrupt handlers.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void timeout(int sig)
{
        alarm(0);
        report_fru(&frub_wrp_cbl[0]);
}

/*
 * NAME: int_handler
 *
 * FUNCTION: Perform general clean up on receipt of an interrupt
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This should describe the execution environment for this
 *      procedure. For example, does it execute under a process,
 *      interrupt handler, or both. Can it page fault. How is
 *      it serialized.
 *
 * RETURNS: NONE
 */

void int_handler(int sig)
{
        if ( IS_TM( console, CONSOLE_TRUE ) )
                diag_asl_clear_screen();
        clean_up();
}

/*
 * NAME: chg_mn_adv()
 *
 * FUNCTION: Designed to change the message pointer to advanced mode.
 *           This function will be called only in advanced mode.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

void
chg_mn_adv()
{
        da_title[0].msgid++;
        wrp_yes_no[0].msgid++;
        rmv_cable[0].msgid++;
        connect_cbl[0].msgid++;
        order_w_inst[0].msgid++;
        order_inst[0].msgid++;
        order_quest[0].msgid++;
        random_w_inst[0].msgid++;
        random_inst[0].msgid++;
        random_quest[0].msgid++;
}

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to report a FRUB to the DC and to set exit
 *           code depends on the testing.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

report_fru( fru_buckets)
struct  fru_bucket      *fru_buckets;
{
        int     rc = 0;
        extern  addfrub();              /* add a FRU bucket to report     */
        extern  insert_frub();


        strncpy(fru_buckets->dname, tm_input.dname, NAMESIZE);
        rc = insert_frub( &tm_input,fru_buckets);
        if ((rc != 0) || (addfrub( fru_buckets) != 0 ))
        {
                DA_SETRC_ERROR ( DA_ERROR_OTHER );
                clean_up();
        }
        if ( IS_TM(system, SYSTEM_FALSE))
        {
                if      /* parent is gio */
                   (connect_to_gio())
                {
                        /* Tell diagnostic controller to test parent (gio)  */
                        DA_SETRC_MORE(DA_MORE_CONT);
                }
        }


        DA_SETRC_STATUS( DA_STATUS_BAD );
        DA_SETRC_TESTS( DA_TEST_FULL );

        clean_up( );
}

/*
 * NAME: chk_terminate_key()
 *
 * FUNCTION: Designed to check if the user hit ESC of F3 keys between
 *           running test units.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_terminate_key()
{
        if ( IS_TM( console, CONSOLE_TRUE ) )
                chk_asl_stat(asl_read(ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
}

/*
 * NAME: initialize_all()
 *
 * FUNCTION: Designed to initialize the odm, get TM input and to
 *           initialize ASL.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

initialize_all()
{
        int     rc;

        if (init_dgodm() == ERR_FILE_OPEN)      /* Initialize odm.        */
        {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }

        if(getdainput( &tm_input ) !=0 ) /* get Diagnostic TM input       */
        {
                DA_SETRC_ERROR( DA_ERROR_OTHER );
                clean_up();
        }
        if ( IS_TM( advanced, ADVANCED_TRUE ) ) /* Check if Advanced mode */
                chg_mn_adv();                   /* Change titles number   */

        if ( IS_TM( console, CONSOLE_TRUE ) )
        {
                chk_asl_stat( diag_asl_init(NULL) );/* initialize ASL     */

                if ( ( catd =  diag_catopen( MF_DDIALS,0) ) == CATD_ERR )
                {                         /* catalog file not found       */
                        DA_SETRC_ERROR( DA_ERROR_OTHER );
                        clean_up();
                }
        }


        msgstr  = (char *) malloc (1200 *sizeof (char));
        msgstr1 = (char *) malloc (1200 *sizeof (char));
        lpp_configure_method = (char *) malloc (1200 *sizeof (char));
        lpp_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        dial_configure_method = (char *) malloc (1200 *sizeof (char));
        dial_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        lpfk_device_configure_method = (char *) malloc (1200 *sizeof (char));
        lpfk_device_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        tty_device_configure_method = (char *) malloc (1200 *sizeof (char));
        tty_device_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        sa_device_configure_method = (char *) malloc (1200 *sizeof (char));
        sa_device_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        cudv_p = (struct CuDv *) malloc (1200 *sizeof (char));
        pddv_p = (struct PdDv *) malloc (1200 *sizeof (char));

        if ((msgstr == (char *) NULL) || (msgstr1 == (char *) NULL) ||
                (lpp_configure_method == (char * )NULL) ||
                (lpp_unconfigure_method == (char *)NULL) ||
                (dial_configure_method == (char * )NULL) ||
                (dial_unconfigure_method == (char * )NULL) ||
                (lpfk_device_configure_method == (char *)NULL) ||
                (lpfk_device_unconfigure_method == (char *) NULL) ||
                (tty_device_configure_method == (char *)NULL) ||
                (tty_device_unconfigure_method == (char *) NULL) ||
                (sa_device_configure_method == (char *)NULL) ||
                (sa_device_unconfigure_method == (char *) NULL) ||
                (cudv_p == (struct CuDv *) NULL) ||
                (pddv_p == (struct PdDv *) NULL))
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                clean_up();
        }
}

/*
 * NAME: chk_asl_stat()
 *
 * FUNCTION: Desined to check ASL return code and take an appropriate action.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

chk_asl_stat(returned_code)
long    returned_code;
{
        switch ( returned_code )
        {
                case DIAG_ASL_OK:
                        break;
                case DIAG_MALLOCFAILED:
                case DIAG_ASL_ERR_NO_SUCH_TERM:
                case DIAG_ASL_ERR_NO_TERM:
                case DIAG_ASL_ERR_INITSCR:
                case DIAG_ASL_ERR_SCREEN_SIZE:
                case DIAG_ASL_FAIL:
                {
                        DA_SETRC_ERROR(DA_ERROR_OTHER);
                        clean_up();
                        break;
                }
                case DIAG_ASL_CANCEL:
                {
                        DA_SETRC_USER(DA_USER_QUIT);
                        clean_up();
                        break;
                }
                case DIAG_ASL_EXIT:
                {
                        DA_SETRC_USER(DA_USER_EXIT);
                        clean_up();
                        break;
                }
                default:
                        break;
        }
}

/*
 * NAME: put_title()
 *
 * FUNCTION: Designed to display the diagnostics header line.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 *
 * RETURNS: NONE
 */

put_title(screen_id)
int     screen_id;
{
        chk_asl_stat( diag_display(screen_id,catd, da_title,DIAG_IO,
                        ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );
}

/*
 * NAME: dial_screen()
 *
 * FUNCTION: Display screen for the dial indicating for each dial
 *           the dial number.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

WINDOW  *
dial_screen()
{
        WINDOW  *window;
        int     row,x,y;

        window = newwin (12,60,11,5);   /* Create a new window     */
        wcolorout(window,Bxa);
        cbox(window);                   /* Frame window with box   */
        wcolorend(window);
        for ( row =1; row<9;row++)
        {
                if ( row > 4)   /* Assign X/Y coordinates          */
                {
                        y = row -4 ;
                        x = 30;
                }
                else
                {
                        y = row;
                        x = 3;
                }
                wmove(window,(y*2),x);
                wprintw(window,w_uinfo[0].text,row);
        }
        wrefresh(window);
        return(window);
}

/*
 * NAME: dial_screen2()
 *
 * FUNCTION: Display screen for the dial indicating for each dial
 *           the dial number.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

WINDOW  *
dial_screen2()
{
        WINDOW  *window;
        int     row,x,y;

        window = newwin (12,60,11,5);   /* Create a new window     */
        wcolorout(window,Bxa);
        cbox(window);                   /* Frame window with box   */
        wcolorend(window);
        for ( row =1; row<9;row++)
        {
                wmove(window,row+1,3);
                wprintw(window,w_uinfo[0].text,row);
        }
        wrefresh(window);
        return(window);
}

/*
 * NAME: dial_sub_screen2()
 *
 * FUNCTION: Display and define main frame for the dials movement indicator.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

WINDOW  *
dial_sub_screen2()
{
        WINDOW  *window;
        int     row,x,y;

        window = newwin (10,44,12,18);  /* Create a new window     */
        wcolorout(window,Bxa);
        cbox(window);                   /* Frame window with box   */
        wcolorend(window);
        wrefresh(window);
        return(window);
}

/*
 * NAME: dial_position()
 *
 * FUNCTION: Designed to display the movement of each dial when is
 *           rotated.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 *
 * RETURNS: NONE
 */

dial_position(w,ptr)
WINDOW  *w;             /* Pointer to subwindow screen             */
struct  tucb_data       *ptr;
{
        static  int     cord[]={21,21,21,21,21,21,21,21};

        short   dialno;
        if( ptr->dial.dir_amt != 0)
        {
                dialno = ptr->dial.dial_no+1;
                wmove(w,dialno,cord[dialno-1]);
                wprintw(w,w_uinfo[4].text); /* Display ' '         */
                cord[dialno-1]+=(ptr->dial.dir_amt/ptr->dial.granularity);
                if (cord[dialno-1]>= (w->_maxx - 1 ))
                        cord[dialno-1]-=(w->_maxx - 2);

                if (cord[dialno-1]<= 0 )
                        cord[dialno-1]+=(w->_maxx - 2);
                wmove(w,dialno,cord[dialno-1]);
                wprintw(w,w_uinfo[5].text); /* Display '+'         */
                wrefresh(w);
        }
}
/*
 * NAME: screen_update()
 *
 * FUNCTION: Update dial window status, such as the dial is rotated
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

screen_update(window,row,str)
WINDOW  *window;
int     row;
char    *str;
{
        int     x,y;
        if ( row > 4)
        {
                y = row -4 ;
                x = 40;
        }
        else
        {
                y = row;
                x = 13;
        }
        wmove(window,(y*2),x);
        waddstr(window,w_uinfo[1].text);
        wmove(window,10,22);
        waddstr(window,str);
        wrefresh(window);
}

/*
 * NAME: chk_all_dial()
 *
 * FUNCTION: Force the user to turn each dial clockwise or counterclockwise
 *           until the screen indiacate that the dial is rotated.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 *
 * RETURNS: NONE
 */

int
chk_all_dial(dial_tucb)
struct  tucb_data       *dial_tucb;
{

        WINDOW  *w;
        int     rc=0;           /* TU return code               */
        w = dial_screen();
        /* loop until all the dials are turned             */
        dial_tucb->gio.key = 1000;
        dial_tucb->header.tu = 140;
        dial_tucb->dial.dial_no = -1;
        dial_tucb->dial.granularity= 8;
        while (TRUE)
        {
                rc = exectu (device_name, dial_tucb);

                if (rc != FALSE || dial_tucb->gio.key==DIAG_ASL_EXIT
                    || dial_tucb->gio.key==DIAG_ASL_ENTER
                    || dial_tucb->gio.key==DIAG_ASL_CANCEL)
                        break;
                if(dial_tucb->dial.dir_amt >0)
                        screen_update(w,dial_tucb->dial.dial_no+1
                                        ,w_uinfo[2].text);
                else
                        screen_update(w,dial_tucb->dial.dial_no+1
                                        ,w_uinfo[3].text);
                dial_tucb->gio.key = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL);
                if (dial_tucb->gio.key == DIAG_ASL_EXIT ||
                   dial_tucb->gio.key == DIAG_ASL_ENTER ||
                    dial_tucb->gio.key == DIAG_ASL_CANCEL )
                        break;
        }
        delwin(w);
        return(rc);
}

/*
 * NAME: put_title()
 *
 * FUNCTION: Indicate cursor movement of each dial when is turnned.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 *
 * RETURNS: NONE
 */

int
rand_dial_chk(dial_tucb)
struct  tucb_data       *dial_tucb;
{
        WINDOW  *w;
        int     rc=0;           /* TU return code               */

        w = dial_screen();
        /* Discard the garbage          */
        while (TRUE)
        {
                dial_tucb->header.tu = 150;
                dial_tucb->gio.key = 0;
                dial_tucb->gio.error = 0;
                dial_tucb->dial.dial_no = 0;
                dial_tucb->dial.dir_amt = 0;
                dial_tucb->dial.granularity= 2;
                rc = exectu (device_name, dial_tucb);
                if (rc == -1)
                        break;
        }
        /* loop until  user says it quits        */
        while (TRUE)
        {
                dial_tucb->header.tu = 150;
                dial_tucb->gio.key = 0;
                dial_tucb->gio.error = 0;
                dial_tucb->dial.dial_no = 0;
                dial_tucb->dial.dir_amt = 0;
                dial_tucb->dial.granularity= 2;
                rc = exectu (device_name, dial_tucb);
                if (rc != -1)
                {
                        if(dial_tucb->dial.dir_amt >0)
                        {
                                screen_update(w,dial_tucb->dial.dial_no+1
                                        ,w_uinfo[2].text);
                        }
                        else
                                screen_update(w,dial_tucb->dial.dial_no+1
                                        ,w_uinfo[3].text);
                }
                dial_tucb->gio.key =
                        diag_asl_read(ASL_DIAG_KEYS_ENTER_SC,NULL,NULL);

                if (dial_tucb->gio.key == DIAG_ASL_EXIT ||
                   dial_tucb->gio.key == DIAG_ASL_ENTER ||
                    dial_tucb->gio.key == DIAG_ASL_CANCEL )
                        break;
        }
        delwin(w);
        return(rc);
}

static  jmp_buf jmp;
void retry_time(int sig)
{
        alarm(0);
        longjmp(jmp,0);
}

error_other()
{
        DA_SETRC_ERROR ( DA_ERROR_OTHER );
        clean_up();
}


/************************************************************************/
/*      This procedure check if the parent is connected to gio card     */
/*      Return TRUE     : if DIAL is connected to gio card              */
/*                        else return FALSE                             */
/************************************************************************/

int     connect_to_gio ()
{
        char    device[4];
        char    parent[4];
        int     rc;


        strcpy (parent, tm_input.parent);
        strcpy (device, "gio");
        device[3]='\0';
        parent[3]='\0';
        rc = strcmp (parent, device);

        if      /* parent is gio */
           (rc == 0)
        {
                return (TRUE);
        }
        return (FALSE);

}       /* connect_to_gio */

/*--------------------------------------------------------------*/
/*      NAME: unconfigure_lpp_device                            */
/*      Description:                                            */
/*      Return          : 0 Good                                */
/*                        -1 BAD                                */
/*--------------------------------------------------------------*/
int     unconfigure_lpp_device()
{
        struct listinfo v_info;
        char    criteria[128];
        char    uniquetype[128];
        char    *result_p;
        int     result=0;


        if (connect_to_gio()) {  /* Attached to GIO adapter */

                /* Set parent and dial device to DIAGNOSE state */
                result = diagex_cfg_state (tm_input.parent);

                switch (result) {
                   case 1:
                   case 2:
                      DA_SETRC_ERROR(DA_ERROR_OPEN);
                      clean_up();
                      break;
                   case 3:
                      sprintf(msgstr, (char *)diag_cat_gets ( catd, DEVICE,
                              LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
                              tm_input.dname, tm_input.dnameloc);
                      menugoal (msgstr);
                      clean_up();
                      break;
                   case -1:
                      DA_SETRC_ERROR(DA_ERROR_OTHER);
                      clean_up();
                } /* endswitch */
                set_device_to_diagnose= TRUE;
                return (0);

        }
        else {  /* Serially attached.  Need to do the unconfigure manually */
                /* because diagex_cfg_state() is not 'smart' enough to     */
                /* follow the 'ttydevice' link between a tty and the dial. */


                /* find out if there is any dial that is the children of gio    */
                /* and not MISSING ,MISSING is define as 3                      */

                sprintf (criteria, "name=%s AND chgstatus!=3", tm_input.dname);

                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                if (( cudv_p != (struct CuDv *) NULL )&&( cudv_p != (struct CuDv *)-1 ))
                {
                        /* Search for the device config and unconfigure method */

                        strcpy (dial, cudv_p->name);
                        /* find out the config and unconfig method of this dial */
                        strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                        sprintf (criteria, "uniquetype=%s", uniquetype);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &v_info, 1, 1);
                        if(!strncmp(pddv_p->subclass, "sgio", 4))
                                serial_gio = TRUE;
                        if(cudv_p->status == 1) {
                                strcpy (dial_unconfigure_method, pddv_p->Unconfigure);
                                strcpy (dial_configure_method, pddv_p->Configure);

                                sprintf (option," -l %s", dial);
                                result = odm_run_method(dial_unconfigure_method,
                                        option, &new_out, &new_err);
                                if (result != -1)
                                {
                                        dial_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                           catd, DEVICE,NEW_LPP_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
                        }
                }

                /* find out if lpfk device name that is available*/

                sprintf (criteria, "name like lpf*  AND status=1");
                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                if (( cudv_p != (struct CuDv *) NULL ) && ( cudv_p != (struct CuDv *)-1 ))
                {
                        /* Device is not missing        */
                        strcpy (lpfk_device, cudv_p->name);
                        /* find out the config and unconfig method of this dial */
                        strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                        sprintf (criteria, "uniquetype like %s", uniquetype);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &v_info, 1, 1);
                        strcpy (lpfk_device_unconfigure_method, pddv_p->Unconfigure);
                        strcpy (lpfk_device_configure_method, pddv_p->Configure);

                        sprintf (option," -l %s", lpfk_device);
                        result = odm_run_method(lpfk_device_unconfigure_method,
                                option, &new_out, &new_err);
                        if (result != -1)
                        {
                                lpfk_device_unconfigure_lpp = TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, DEVICE,LPFK_DEVICE_CANNOT_UNCONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                menugoal (msgstr);
                                clean_up();
                        }
                }

                /* If serially attached, both native serial ports and the tty's */
                /* attached to them need to be put into defined state */
                if(serial_gio) {

                        /* Check if AVAILABLE tty attached to Serial Port 1 */
                        sprintf (criteria, "parent=sa0 AND status=1");
                        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                        if (( cudv_p != (struct CuDv *) NULL ) &&
                            ( cudv_p != (struct CuDv *)-1 ))
                        {
                                /* Device is not missing        */
                                strcpy (tty0_device, cudv_p->name);
                                /* find out the config and unconfig method of this tty */
                                strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                                sprintf (criteria, "uniquetype like %s", uniquetype);
                                pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                      &v_info, 1, 1);
                                strcpy (tty_device_unconfigure_method,
                                      pddv_p->Unconfigure);
                                strcpy (tty_device_configure_method,
                                      pddv_p->Configure);

                                sprintf (option," -l %s", tty0_device);
                                result = odm_run_method(tty_device_unconfigure_method,
                                        option, &new_out, &new_err);
                                if (result == 0)
                                {
                                        tty0_device_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                                catd, DEVICE,
                                                TTY_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
                        }

                        /* Check if AVAILABLE tty attached to Serial Port 2 */
                        sprintf (criteria, "parent=sa1 AND status=1");
                        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                        if (( cudv_p != (struct CuDv *) NULL ) &&
                            ( cudv_p != (struct CuDv *)-1 ))
                        {
                                /* Device is not missing        */
                                strcpy (tty1_device, cudv_p->name);
                                /* find out the config and unconfig method of this tty */
                                strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                                sprintf (criteria, "uniquetype like %s", uniquetype);
                                pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                      &v_info, 1, 1);
                                strcpy (tty_device_unconfigure_method,
                                      pddv_p->Unconfigure);
                                strcpy (tty_device_configure_method,
                                      pddv_p->Configure);

                                sprintf (option," -l %s", tty1_device);
                                result = odm_run_method(tty_device_unconfigure_method,
                                        option, &new_out, &new_err);
                                if (result == 0)
                                {
                                        tty1_device_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                                catd, DEVICE,
                                                TTY_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
                        }

                        /* Check if Serial Port 1 is AVAILABLE */
                        sprintf (criteria, "name=sa0 AND status=1");
                        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                        if (( cudv_p != (struct CuDv *) NULL ) &&
                            ( cudv_p != (struct CuDv *)-1 ))
                        {
                                /* Device is not missing        */
                                strcpy (sa0_device, cudv_p->name);
                                /* find out the config and unconfig method of this port */
                                strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                                sprintf (criteria, "uniquetype like %s", uniquetype);
                                pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                      &v_info, 1, 1);
                                strcpy (sa_device_unconfigure_method,
                                      pddv_p->Unconfigure);
                                strcpy (sa_device_configure_method,
                                      pddv_p->Configure);

                                sprintf (option," -l %s", sa0_device);
                                result = odm_run_method(sa_device_unconfigure_method,
                                        option, &new_out, &new_err);
                                if (result == 0)
                                {
                                        sa0_device_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                                catd, DEVICE,
                                                SA_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
                        }

                        /* Check if Serial Port 2 is AVAILABLE */
                        sprintf (criteria, "name=sa1 AND status=1");
                        cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                        if (( cudv_p != (struct CuDv *) NULL ) &&
                            ( cudv_p != (struct CuDv *)-1 ))
                        {
                                /* Device is not missing        */
                                strcpy (sa1_device, cudv_p->name);
                                /* find out the config and unconfig method of this port */
                                strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                                sprintf (criteria, "uniquetype like %s", uniquetype);
                                pddv_p = get_PdDv_list (PdDv_CLASS, criteria,
                                      &v_info, 1, 1);
                                strcpy (sa_device_unconfigure_method,
                                      pddv_p->Unconfigure);
                                strcpy (sa_device_configure_method,
                                      pddv_p->Configure);

                                sprintf (option," -l %s", sa1_device);
                                result = odm_run_method(sa_device_unconfigure_method,
                                        option, &new_out, &new_err);
                                if (result == 0)
                                {
                                        sa1_device_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                                catd, DEVICE,
                                                SA_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
                        }

                } /* endif serial_gio */


                /* checking for the device gio  */
                sprintf (criteria, "name like gio* AND status=1 ");
                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                if (( cudv_p != (struct CuDv *) NULL ) && ( cudv_p != (struct CuDv *)-1 ))
                {
                        /* find out the config and unconfig method of this device*/

                        gio_device_state = cudv_p->status;
                        strcpy (gio_device_name, cudv_p->name);
                        strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                        sprintf (criteria, "uniquetype like %s", uniquetype);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &v_info, 1, 1);
                        strcpy (lpp_unconfigure_method, pddv_p->Unconfigure);
                        strcpy (lpp_configure_method, pddv_p->Configure);

                        sprintf (option," -l %s", gio_device_name);
                        result = odm_run_method(lpp_unconfigure_method,
                                option, &new_out, &new_err);
                        if (result != -1)
                        {
                                unconfigure_lpp = TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                        catd, 1,GIO_DEVICE_CANNOT_UNCONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                menugoal (msgstr);
                                clean_up();
                        }
                        sprintf (option," -l %s", gio_device_name);
                        strcpy (criteria,"/usr/lib/methods/cfgdiag");

                        result = odm_run_method(criteria,option, &new_out, &new_err);
                        if (result != -1)
                        {
                                set_device_to_diagnose= TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, DEVICE,
                                        LPP_DEVICE_CANNOT_SET_TO_DIAGNOSE),
                                        cudv_p->name, cudv_p->location);
                                menugoal (msgstr);
                                clean_up();
                        }

                }
                return (0);
        }
}
/*---------------------------------------------------------------------------
 * NAME: configure_lpp_device ()
 *
 * FUNCTION: configure_lpp_device ()
 *           Unload the diagnostic device driver
 *           setting the device to the DEFINE state (No matter what state
 *           it is in before running the test. We do not want left it in
 *           DIAGNOSE state after running diagnostics
 *           clean_up will restore the AVAILABLE state if it is so before
 *           running the test
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 *--------------------------------------------------------------------------*/
int     configure_lpp_device()
{
        struct listinfo v_info;
        char    criteria[128];
        char    *result_p;
        int     result;
        int     rc;

        if(connect_to_gio()) {
                if (set_device_to_diagnose) {
                        result = diagex_initial_state (tm_input.parent);
                        if (result != 0) {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                  catd, DEVICE,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                         tm_input.dname, tm_input.dnameloc);
                                menugoal (msgstr);
                        } /* endif */
                        else
                                set_device_to_diagnose = FALSE;
                }
        }
        else {

                /* Setting the device state to DEFINE state */
                /* clean_up will restore the state to original  */
                /* The DIAGNOSE state of the device is set to DEFINE state */
                /* Even though before running diagnostics it is in DIAGNOSE state */
                if (set_device_to_diagnose)
                {
                        sprintf (option," -l %s", gio_device_name);
                        strcpy (criteria,"/usr/lib/methods/ucfgdiag");
                        result = odm_run_method(criteria,option,
                                 &new_out, &new_err);
                        if (result != -1 )
                        {
                                set_device_to_diagnose= FALSE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                  catd, DEVICE,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                         tm_input.dname, tm_input.dnameloc);
                                menugoal (msgstr);
                        }
                }

                /* Configure lpp device driver (gio) first      */

                if (gio_device_state == AVAILABLE)
                {
                        sprintf (option," -l %s", gio_device_name);
                        rc= odm_run_method(lpp_configure_method,option,
                                           &new_out, &new_err);
                        if (rc == -1)
                        {
                                sprintf(msgstr, (char *)diag_cat_gets ( catd, 1,
                                        GIO_DEVICE_CANNOT_CONFIGURED),gio_device_name);
                                menugoal (msgstr);
                        }
                }

                /* Configuring  Serial ports and devices        */
                if (serial_gio) {
                        if(sa0_device_unconfigure_lpp) {
                                sprintf (option," -l %s", sa0_device);
                                rc= odm_run_method(sa_device_configure_method,option,
                                                   &new_out, &new_err);

                        }

                        if(sa1_device_unconfigure_lpp) {
                                sprintf (option," -l %s", sa1_device);
                                rc= odm_run_method(sa_device_configure_method,option,
                                                   &new_out, &new_err);

                        }

                        if(tty0_device_unconfigure_lpp) {
                                sprintf (option," -l %s", tty0_device);
                                rc= odm_run_method(tty_device_configure_method,option,
                                                   &new_out, &new_err);

                        }

                        if(tty1_device_unconfigure_lpp) {
                                sprintf (option," -l %s", tty1_device);
                                rc= odm_run_method(tty_device_configure_method,option,
                                                   &new_out, &new_err);

                        }

                } /* endif serial_gio */

                /* Configuring  Dials device                    */
                if (dial_unconfigure_lpp)
                {
                        sprintf (option," -l %s", dial);
                        rc= odm_run_method(dial_configure_method,option,
                                           &new_out, &new_err);
                }

                /* Configuring  lpfk device             */
                if (lpfk_device_unconfigure_lpp)
                {
                        sprintf (option," -l %s", lpfk_device);
                        rc= odm_run_method(lpfk_device_configure_method,option,
                                           &new_out, &new_err);
                }
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
        if(msgstr != (char *) NULL)
                free (msgstr);
        if(msgstr1 != (char *) NULL)
                free (msgstr1);
        if(lpp_configure_method != (char *) NULL)
                free (lpp_configure_method);
        if(lpp_unconfigure_method != (char *) NULL)
                free (lpp_unconfigure_method);
        if(dial_configure_method != (char *) NULL)
                free (dial_configure_method);
        if(dial_unconfigure_method != (char *) NULL)
                free (dial_unconfigure_method);
        if(lpfk_device_configure_method != (char *) NULL)
                free (lpfk_device_configure_method);
        if(lpfk_device_unconfigure_method != (char *) NULL)
                free (lpfk_device_unconfigure_method);
        if(tty_device_configure_method != (char *) NULL)
                free (tty_device_configure_method);
        if(tty_device_unconfigure_method != (char *) NULL)
                free (tty_device_unconfigure_method);
        if(sa_device_configure_method != (char *) NULL)
                free (sa_device_configure_method);
        if(sa_device_unconfigure_method != (char *) NULL)
                free (sa_device_unconfigure_method);
        if (cudv_p != (struct CuDv *) NULL)
                free (cudv_p);
        if (pddv_p != (struct PdDv *) NULL)
                free (cudv_p);
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

void sw_timeout(int sig)
{
        alarm(0);
        alarm_timeout = TRUE;
        clean_up();
}

/* --------------------------------------------------------------------------
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
 ----------------------------------------------------------------------- */

void set_timer()
{
        invec.sa_handler = sw_timeout;
        alarm_timeout = FALSE;
        sigaction( SIGALRM, &invec, (struct sigaction *) NULL );
        alarm(300);
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

        for (; *rc_ptr; rc_ptr++)
        {
                if (*rc_ptr == rc )
                        return(TRUE);
        }

        DA_SETRC_ERROR (DA_ERROR_OTHER);
        return(FALSE);
}

