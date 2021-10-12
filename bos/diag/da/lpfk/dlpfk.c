static char sccsid[] = "@(#)51  1.36  src/bos/diag/da/lpfk/dlpfk.c, dalpfk, bos41J, 9525E_all 6/21/95 09:22:43";
/*
 *   COMPONENT_NAME: DALPFK
 *
 *   FUNCTIONS: CHAR_TO_INT
 *              DISPLAY_MENU
 *              EXECUTE_TU_WITHOUT_MENU
 *              EXECUTE_TU_WITH_MENU
 *              IS_TM
 *              PUT_EXIT_MENU
 *              PUT_INSTRUCTION
 *              PUT_QUEST_MENU
 *              chg_mn_adv
 *              chk_asl_stat
 *              chk_ela
 *              chk_terminate_key
 *              clean_up
 *              connect_to_gio
 *              error_other
 *              if
 *              initialize_all
 *              insert_wrap_menu
 *              instruction_menu
 *              int_handler
 *              main
 *              put_title
 *              question_menu
 *              remove_wrap_menu
 *              report_fru
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
#include        <signal.h>
#include        <setjmp.h>
#include        <errno.h>
#include        <sys/types.h>
#include        <sys/termio.h>
#include        <sys/mdio.h>
#include        <sys/cfgodm.h>
#include        <locale.h>

#include        "diag/diago.h"
#include        "diag/diag.h"
#include        "diag/tm_input.h"
#include        "diag/tmdefs.h"
#include        "diag/da.h"
#include        "diag/diag_exit.h"
#include        "diag/atu.h"
#include        "dlpfk_msg.h"
#include        "lpfk.h"
#include        <sys/termio.h>
#include        <termios.h>

void    sw_timeout(int);
void    set_timer();
void    clean_malloc();
int     alarm_timeout=FALSE;
struct  sigaction  invec;       /* interrupt handler structure  */
int     tu_open_flag=FALSE;

/* This section for 4.1 where ucfgdiag and cfgdiag is implemented and diagex */
char *msgstr;
char *msgstr1;
char    option[256];
char    *new_out, *new_err;
char    *lpp_configure_method;
char    *lpp_unconfigure_method;
char    *child1_configure_method;
char    *child1_unconfigure_method;
char    *lpfk_device_configure_method;
char    *lpfk_device_unconfigure_method;
char    *sa_device_configure_method;
char    *sa_device_unconfigure_method;
char    *tty_device_configure_method;
char    *tty_device_unconfigure_method;
struct  PdDv    *pddv_p;
struct  CuDv    *cudv_p;
int     unconfigure_lpp=FALSE;
int     child1_unconfigure_lpp=FALSE;
int     lpfk_device_unconfigure_lpp=FALSE;
int     tty0_device_unconfigure_lpp=FALSE;
int     tty1_device_unconfigure_lpp=FALSE;
int     sa0_device_unconfigure_lpp=FALSE;
int     sa1_device_unconfigure_lpp=FALSE;
int     serial_gio=FALSE;
char    child1[16];
char    lpfk_device[16];
char    sa0_device[16];
char    sa1_device[16];
char    tty0_device[16];
char    tty1_device[16];
char    gio_device_name[16];
int     set_device_to_diagnose = FALSE;
int     gio_device_state = -1;



#define TIMEOUT 30


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

#define IS_TM( VAR1, VAR2 )  ((int)(tm_input./**/VAR1)== /**/VAR2 )

/**************************************************************************/
/*                                                                        */
/* WRP_CBL_NOT_EXEC: macro define all the conditions where device cable   */
/*                   wrap test unit will not excute.                      */
/*                                                                        */
/**************************************************************************/

#define WRP_CBL_NOT_EXEC (                IS_TM( console, CONSOLE_FALSE )  \
                                ||        IS_TM( system, SYSTEM_TRUE    )  \
                         )

#define EXECUTE_CBL_WRP ( ! WRP_CBL_NOT_EXEC )


/**************************************************************************/
/*                                                                        */
/* LIGHT_ON_NOT_EXEC: macro define all the conditions where to turn ON    */
/*                    LPFK light test unit will not excute.               */
/*                                                                        */
/**************************************************************************/

#define LIGHT_ON_NOT_EXEC (             IS_TM( console, CONSOLE_FALSE   )  \
                                ||      IS_TM( system, SYSTEM_TRUE      )  \
                          )

#define EXECUTE_LIGHT_ON ( ! ( LIGHT_ON_NOT_EXEC ) )

/**************************************************************************/
/*                                                                        */
/* LIGHTS_OFF_NOT_EXEC: macro define all the conditions where to Turn     */
/*                      LPFK lights OFF will not execute.                 */
/*                                                                        */
/**************************************************************************/

#define LIGHTS_OFF_NOT_EXEC (           IS_TM( console, CONSOLE_FALSE   )  \
                                ||      IS_TM( system, SYSTEM_TRUE      )  \
                          )

#define EXECUTE_LIGHT_OFF ( ! ( LIGHTS_OFF_NOT_EXEC ) )

/**************************************************************************/
/*                                                                        */
/* K_ORDER_COMP_NOT_EXEC: macro define all the conditions where testing   */
/*                        LPFKs in order of the light goes ON             */
/*                                                                        */
/**************************************************************************/

#define K_ORDER_COMP_NOT_EXEC (         IS_TM( console, CONSOLE_FALSE   ) \
                                ||      IS_TM( system, SYSTEM_TRUE      )  \
                              )

#define EXECUTE_ORDER_COMP ( ! ( K_ORDER_COMP_NOT_EXEC ) )

/**************************************************************************/
/*                                                                        */
/* RAND_KEY_COMP_NOT_EXEC: macro define all the conditions where testing  */
/*                        LPFKs in order of the light goes ON             */
/*                                                                        */
/**************************************************************************/

#define RAND_KEY_COMP_NOT_EXEC (        IS_TM( console, CONSOLE_FALSE   ) \
                                ||      IS_TM( system, SYSTEM_TRUE      )  \
                              )

#define EXECUTE_RANDOM_COMP ( ! (RAND_KEY_COMP_NOT_EXEC ) )

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
                    (   ( tmode[/**/TU_NUM].wrapflg == TRUE )              \
                      ||( tmode[/**/TU_NUM].wrapflg == FALSE )             \
                      ||( tmode[/**/TU_NUM].mflg & QUEST    )              \
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
                        (  IS_TM( loopmode, LOOPMODE_NOTLM     )           \
                         ||IS_TM( loopmode, LOOPMODE_ENTERLM   )           \
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
                           IS_TM( loopmode, LOOPMODE_NOTLM     )           \
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
/*      da_title is an array of structure holds the message set           */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_msg to tell the user        */
/*      hft failed to open.                                               */
/*                                                                        */
/**************************************************************************/

struct  msglist da_title[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_GENERIC,         LPF_STND_BY     },
                        0
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
                        {  LPF_WRP_NUM,         LPF_WRP_TITLE   },
                        {  LPF_GENERIC,         LPF_YES         },
                        {  LPF_GENERIC,         LPF_NO          },
                        {  LPF_GENERIC,         LPF_ACTION      },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      hft_ok is an array of structure holds the message set             */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_display to ask the user     */
/*      if the order test runs ok on the hft.                             */
/*                                                                        */
/**************************************************************************/

struct  msglist hft_ok[]=
                {
                        {  HFT_ORD_OK,          HFT_ORD_OK_TITLE},
                        {  LPF_GENERIC,         LPF_YES         },
                        {  LPF_GENERIC,         LPF_NO          },
                        {  LPF_GENERIC,         LPF_ACTION      },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      hft_ok_a is an array of structure holds the message set           */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be passed to diag_display to ask the user     */
/*      if the order test runs ok on the hft.                             */
/*                                                                        */
/**************************************************************************/

struct  msglist hft_ok_a[]=
                {
                        {  HFT_ORD_OK,          HFT_ORD_A_OK_TITLE},
                        {  LPF_GENERIC,         LPF_YES           },
                        {  LPF_GENERIC,         LPF_NO            },
                        {  LPF_GENERIC,         LPF_ACTION        },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      rmv_cable is an array of structure holds the message set          */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to ask         */
/*      the user to remove the cable if attached, and to plug a           */
/*      wrap plug into the LPF adapter port.                              */
/*                                                                        */
/**************************************************************************/


struct  msglist rmv_cable[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_RMV_ATTACHEMENT, LPF_UNPLUG_CBL  },
                        {  LPF_RMV_ATTACHEMENT, LPF_PLUG_WRP    },
                        {  LPF_GENERIC,         LPF_PRS_ENT     },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      connect_cbl is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will be used for the user interface to ask         */
/*      the user to remove the wrap plug and to reconnect the cable.      */
/*                                                                        */
/**************************************************************************/

struct  msglist connect_cbl[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_RECONNECT,       LPF_UNPLUG_WRP  },
                        {  LPF_RECONNECT,       LPF_PLUG_CBL    },
                        {  LPF_GENERIC,         LPF_PRS_ENT     },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      light_on is an array of structure holds the message set           */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will used to ask the user if all LPFKs are lit.    */
/*                                                                        */
/**************************************************************************/

struct  msglist light_on[]=
                {
                        {  LPF_LIGHT_ON,        LPF_LIGHT_TITLE },
                        {  LPF_GENERIC,         LPF_YES         },
                        {  LPF_GENERIC,         LPF_NO          },
                        {  LPF_GENERIC,         LPF_ACTION      },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      light_off is an array of structure holds the message set          */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      This structure will used to ask the user if all LPFKs are unlit.  */
/*                                                                        */
/**************************************************************************/

struct  msglist light_off[]=
                {
                        {  LPF_LIGHT_OFF,       LPF_UNLIT_TITLE },
                        {  LPF_GENERIC,         LPF_YES         },
                        {  LPF_GENERIC,         LPF_NO          },
                        {  LPF_GENERIC,         LPF_ACTION      },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      order_w_inst is an array of structure holds the message set       */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      display an instruction menu telling the user running this test    */
/*      will need a user intercation.                                     */
/*                                                                        */
/**************************************************************************/

struct  msglist order_w_inst[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_LIGHT_ORD,       LPF_ORD_INST    },
                        {  LPF_GENERIC,         LPF_INST_PRS_ENT        },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      order_inst is an array of structure holds the message set         */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      display an instruction menu telling the user to press each        */
/*      LPFk as become lit.                                               */
/*                                                                        */
/**************************************************************************/

struct  msglist order_inst[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_LIGHT_ORD,       LPF_ORD_INST    },
                        {  LPF_LIGHT_ORD,       LPF_ORD_EXIT    },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      random_w_inst is an array of structure holds the message set      */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      display an instruction menu telling the user running this test    */
/*      will need a user intercation.                                     */
/*                                                                        */
/**************************************************************************/

struct  msglist random_w_inst[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_LIGHT_RND,       LPF_RND_INST_E  },
                        0
                };

/**************************************************************************/
/*                                                                        */
/*      random_inst is an array of structure holds the message set        */
/*      number and a message id number in the menus catalog file.         */
/*                                                                        */
/*      display an instruction menu telling the user to press any         */
/*      LPFk  the key will toggle ON and OFF.                             */
/*                                                                        */
/**************************************************************************/

struct  msglist random_inst[]=
                {
                        {  LPF_GENERIC,         LPF_TITLE       },
                        {  LPF_LIGHT_RND,       LPF_RND_INST    },
                        {  LPF_LIGHT_RND,       LPF_RND_EXIT    },
                        0
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
        { LPFK_CBL_WRP  , TRUE     , FALSE, MAX_CBL_WRP         },
        { LPFK_INT_WRAP , FALSE    , FALSE, MAX_INT_WRAP        },
        { LPFK_LIGHT_ON , QUEST    , FALSE, MAX_LIGHT_ON        },
        { LPFK_LIGHT_OFF, QUEST    , FALSE, MAX_LIGHT_OFF       },
        { LPFK_ORD_COMP , INST_QUEST, FALSE, MAX_ORD_COMP       },
        { LPFK_RND_COMP , INST_QUEST, FALSE, MAX_RND_COMP       },
        { LPFK_RESET    , FALSE    , FALSE, MAX_RESET           },
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

        { "", FRUB1,  0x930, 0x661, LPFK_COM_FAILED,
                        {
                                { 100 , "Cable" , "", LPFK_CABLE,
                                        0 , EXEMPT },
                        },
        },

};

/* this fru buckete is for lpfk running under async port */
struct fru_bucket frub_wrp_cbl_async[]=
{

        { "", FRUB1,  0x930, 0x662, LPFK_COM_FAILED,
                        {
                                { 90 , "" , "", 0,
                                        PARENT_NAME, 0   },
                                { 10 , "Cable" , "", LPFK_CABLE,
                                        0 , EXEMPT },

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

        { "", FRUB1,  0x930, 0x771, LPFK_RESET_FAILED,
                        {
                                { 90, "" , "" , 0, DA_NAME,      0      },
                                { 10 , "Cable" , "", LPFK_CABLE,
                                        0 , EXEMPT },
                        },
        },

};

/* this fru bucket is for lpfk running under aync port */
struct fru_bucket frub_bat_code_async[]=
{

        { "", FRUB1,  0x930, 0x772, LPFK_LCL_WRAP,
                        {
                                { 50, "Power Supply", "", LPFK_POW,0, 0   },
                                { 45, "" , "", LPFK_FAILED, DA_NAME, 0   },
                                { 5 , "Cable" , "", LPFK_CABLE,
                                        0 , EXEMPT },
                        },
        },

};

/**************************************************************************/
/*                                                                        */
/*      frub_lpfk struct holds related information to FRU bucket          */
/*      variables in this structure are explained in DA CAS (SJDIAG)      */
/*      under the function name ADDFRU.                                   */
/*      One of these frus will be reported if a failure return code       */
/*      returned from the test unit.                                      */
/*                                                                        */
/**************************************************************************/

struct fru_bucket frub_lpfk[]=
{

        { "", FRUB1,  0x930, 0x781, LPFK_FAILED,
                        {
                                { 100, "" , "" , 0, DA_NAME,     0      },
                        },
        },
};

/**************************************************************************/
/*      This fru bucket is issued when configuration for device fails     */
/**************************************************************************/
struct fru_bucket config_fail_frub[]=
{

        { "", FRUB1,  0x930, 0x800, LPFK_CONFIG_FAIL,
                        {
                                { 80, "" , "" , 0, DA_NAME,      0      },
                                { 20, "" , "" , 0, PARENT_NAME,  0      },
                        },
        },
};
struct fru_bucket config_fail_frub_gio[]=
{

        { "", FRUB1,  0x930, 0x801, LPFK_CONFIG_FAIL,
                        {
                                { 80, "" , "" , 0, PARENT_NAME,  0      },
                                { 20, " " , "" , LPFK_IOPLANAR, 0, 0   },
                        },
        },
};
/* this fru is issued when open fails on lpfk when connecto serial port */
struct fru_bucket open_fail_frub[]=
{

        { "", FRUB1,  0x930, 0x802, LPFK_OPEN_ERROR,
                        {
                                { 80, "" , "" , 0, DA_NAME,      0      },
                                { 10, "" , "" , 0, PARENT_NAME,  0      },
                                { 10, " " , "" , LPFK_SOFTWARE_ERROR,
                                                        0, EXEMPT},
                        },
        },
};
/* this fru is issued when open fails on lpfk when connecto gio */
struct fru_bucket open_fail_frub_gio[]=
{

        { "", FRUB1,  0x930, 0x803, LPFK_OPEN_ERROR,
                        {
                                { 90, "" , "" , 0, PARENT_NAME,  0      },
                                { 10, " " , "" , LPFK_SOFTWARE_ERROR,
                                                         0, EXEMPT},
                        },
        },
};

/* This fru happens when calling Test Unit return -1 and error = 10 */
struct fru_bucket software_frub[]=
{
        { "", FRUB1,  0x930  ,  0x804  , LPFK_CONFIG_FAIL,
                {
                        { 80, "",   "", LPFK_SOFTWARE_ERROR,0, EXEMPT},
                        { 10, "",   "", 0,       DA_NAME,     0         },
                        { 10, "",   "", 0,       PARENT_NAME,     0     },
                },
        },
};


struct  tm_input        tm_input;


nl_catd catd = CATD_ERR;        /* pointer to the catalog file            */
extern  nl_catd diag_catopen();
void    int_handler(int);
void    timeout(int);
int     device_type=0;
int     device_state=0;
int     lpfk_state=0;
ulong   curpd = -1;

/*
 * NAME: main()
 *
 * FUNCTION: Main driver for NIO Tablet Diagnostic applications.
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
        int     rc=0;



        setlocale (LC_ALL, "");
        /* initialize interrupt handler structure       */
        (void)signal(SIGINT, int_handler);

        initialize_all();

        if ( DISPLAY_LOOP_COUNT )
                chk_asl_stat(diag_msg_nw(0x930110,catd,LPF_INLOOP,
                        LPF_INLOOP_TITLE, tm_input.lcount,tm_input.lerrors));

        else if ( DISPLAY_TESTING )
                put_title(0x930001);

        if ( IS_TM(dmode,DMODE_ELA))
                chk_ela();
        else
        {
                unconfigure_lpp_device ();


                /**********************************************************/
                /*              Run all the necessary tests               */
                /**********************************************************/

                if (EXECUTE_CBL_WRP)
                {
                        if (connect_to_gio())
                                /* test 60 */
                                tu_test(IND_CBL_WRP,frub_wrp_cbl);
                        /* LPFK connected to serial port test 60 is not run */
                }

                if (connect_to_gio())
                {
                        /* Test Unit 200 .Reset LPFK    */
                        tu_test(IND_RESET,frub_wrp_cbl);
                        /* Test Unit 70         */
                        tu_test(IND_WRP_TST,frub_bat_code);
                }

                else
                        /* not running reset test in serial port */
                        tu_test(IND_WRP_TST,frub_bat_code_async);
                if (EXECUTE_LIGHT_ON )
                        tu_test(IND_LIGHT_ON,frub_lpfk);

                if (EXECUTE_LIGHT_OFF)
                        tu_test(IND_LIGHT_OFF,frub_lpfk);

                if (EXECUTE_ORDER_COMP )
                {
                        tu_test(IND_ORDER_COMP,frub_lpfk);
                        if ( DISPLAY_TESTING )
                                put_title(0x930001);
                }

                if ( IS_TM ( dmode,DMODE_PD ))
                        chk_ela();

                if ( MORE_RESOURCE )
                        DA_SETRC_TESTS( DA_TEST_NOTEST);
                else
                        DA_SETRC_TESTS( DA_TEST_FULL );
                clean_up( );

        }

}  /*                           Main end                                  */

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

tu_test ( ar_index, fru_buckets)
int     ar_index;
struct  fru_bucket      fru_buckets[];
{

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
        }lpf_tucb;
        int     rc = 0;
        int     error_num = FALSE;      /* error number from the test unit*/
        extern  int     insert_frub();
        extern  int     addfrub();      /* add a FRU bucket to report     */
        extern  exectu();
        char    device_name[64];

        (void) memset (&lpf_tucb, 0, sizeof(lpf_tucb));

        lpf_tucb.header.tu = tmode[ar_index].tu_num;
        lpf_tucb.header.r1 = 0x20; /* value LPFK is define in gio.h     */
        lpf_tucb.header.loop = 1;
        rc = tu_open (tm_input.dname);
        if (rc != 0)
        {
                report_fru(&software_frub);
        }
        else
                tu_open_flag=TRUE;

        if (lpf_tucb.header.tu == 60)
                strcpy (device_name, tm_input.parent);
        else
                strcpy (device_name, tm_input.dname);

        if ( PUT_INSTRUCTION(ar_index) )
                instruction_menu( ar_index);

        if ( DISPLAY_MENU(ar_index) )
        {
                rc = insert_wrap_menu( ar_index);
                if (rc != YES)
                {
                        if (tu_open_flag )
                        {
                                tu_close ();
                                tu_open_flag= FALSE;
                        }
                        return (0);
                }
        }


        if (    EXECUTE_TU_WITH_MENU(ar_index)
             || EXECUTE_TU_WITHOUT_MENU(ar_index) )
         {
                error_num = exectu (device_name, &lpf_tucb);
                if ((error_num == -1) && (lpf_tucb.gio.error== 10))
                {
                        if ( PUT_EXIT_MENU( ar_index ) )
                                remove_wrap_menu( ar_index );
                        report_fru(&software_frub);
                }
        }

        if ( PUT_EXIT_MENU( ar_index ) )
                remove_wrap_menu( ar_index );

        if ( PUT_QUEST_MENU( ar_index ) )
        {
                if (question_menu( ar_index ) == -1)
                {
                        report_fru(&frub_lpfk[0]);
                }
        }

        /* check the test units return code for a valid error */

        if ( error_num == -1 )
        {
                switch (lpf_tucb.header.tu )
                {
                        case 60:
                                if (verify_rc (&tu60[0], lpf_tucb.gio.error))
                                {
                                    if ((lpf_tucb.gio.error & 0xFFFFFFFF) == 1)
                                    {
                                        /*Cable wrap test failed */
                                        if (connect_to_gio())
                                            report_fru(&frub_wrp_cbl_async[0]);
                                        else
                                            report_fru(&frub_wrp_cbl[0]);
                                    }
                                }
                                break;
                        case 200:
                                if (verify_rc (&tu200[0], lpf_tucb.gio.error))
                                {
                                        /*Cable wrap test failed */
                                        if (connect_to_gio())
                                            report_fru(&frub_bat_code_async[0]);
                                        else
                                            report_fru(&frub_bat_code[0]);
                                }
                                break;
                        case 70:
                                if (verify_rc (&tu70[0], lpf_tucb.gio.error))
                                {
                                        report_fru(&frub_lpfk[0]);
                                }
                                break;
                        case 80:
                                if (verify_rc (&tu80[0], lpf_tucb.gio.error))
                                {
                                        report_fru(&frub_lpfk[0]);
                                }
                                break;
                        case 90:
                                if (verify_rc (&tu80[0], lpf_tucb.gio.error))
                                {
                                        report_fru(&frub_lpfk[0]);
                                }
                                break;
                        case 100:
                                if (verify_rc (&tu80[0], lpf_tucb.gio.error))
                                {
                                        report_fru(&frub_lpfk[0]);
                                }
                                break;
                }
                clean_up();
        }
        if (tu_open_flag )
        {
                tu_close ();
                tu_open_flag= FALSE;
        }
        chk_terminate_key();
} /* end tu_test */

/*
 * NAME: chk_ela()
 *
 * FUNCTION:  Check for Error logged by the system and upon the result
 *            from the error log a FRU bucket will be rported or return
 *            to the caller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */


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
        if (tu_open_flag)
        {
                tu_open_flag = FALSE;
                tu_close();
        }

        configure_lpp_device();

        term_dgodm();

        if (catd != CATD_ERR)
                catclose( catd );
        if ( IS_TM( console, CONSOLE_TRUE ) )   /* Check if Console TRUE */
                diag_asl_quit(NULL);
        DA_EXIT();
}

error_other()
{
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        clean_up();
}

/*
 * NAME: insert_wrap_menu()
 *
 * FUNCTION: Designed to ask the user if he has a wrap plug, if he selected
 *           wrap flag will be set and a menu will be displayed and ask the
 *           user to insert a wrap plug at the end of LPFK cable. Otherwise
 *           will return to the caller.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: 0 Suc., -1 Failure.
 */

int     insert_wrap_menu(index)
int     index;
{
        ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS;

        if (IS_TM( advanced, ADVANCED_FALSE))
                menutypes.cur_index = FALSE;
        else
        {
                menutypes.cur_index=1;
                chk_asl_stat(diag_display(0x930003,catd,wrp_yes_no, DIAG_IO,
                        ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
        }

        if ( menutypes.cur_index != TRUE )
        {
                put_title(0x930003);
                tmode[index].wrapflg = FALSE;
                return (0);
        }
        chk_asl_stat(diag_display(0x930004,catd,rmv_cable,DIAG_IO,
                                ASL_DIAG_KEYS_ENTER_SC, NULL,NULL));
        put_title(0x930004);

        tmode[index].wrapflg = TRUE;

        return(YES);
}

/*
 * NAME: remove_wrap_menu()
 *
 * FUNCTION: Designed to tell the user to remove the wrap plug and to reconnect
 *           the cable to LPFK device.
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

        chk_asl_stat(diag_display(0x930006,catd,connect_cbl,DIAG_IO,
                                ASL_DIAG_NO_KEYS_ENTER_SC, NULL,NULL));
        put_title(0x930007);

        tmode[index].wrapflg = FALSE;
}

/*
 * NAME: instruction_menu()
 *
 * FUNCTION: Designed to give the user some instructions on what to do
 *           for a specific test.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

instruction_menu(index)
int     index;
{
        switch (tmode[index].tu_num)
        {
                case LPFK_ORD_COMP:
                        chk_asl_stat(diag_display(0x930011,catd,order_w_inst,
                                DIAG_IO, ASL_DIAG_NO_KEYS_ENTER_SC, NULL,NULL));
                        sleep(2);
                        chk_asl_stat(diag_display(0x930012,catd,order_inst,
                                DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );
                        break;

                case LPFK_RND_COMP:
                        chk_asl_stat(diag_display(0x930013,catd,random_w_inst,
                                DIAG_IO,ASL_DIAG_NO_KEYS_ENTER_SC, NULL,NULL));
                        sleep(2);
                        chk_asl_stat(diag_display(0x930014,catd,random_inst,
                                DIAG_IO,ASL_DIAG_OUTPUT_LEAVE_SC, NULL,NULL) );
                        break;
                default:
                        tmode[index].wrapflg= FALSE;
        };
} /* end instruction_menu */

/*
 * NAME: question_menu()
 *
 * FUNCTION: Designed to ask the user if all the lights on the LPFKs are
 *           ON or OFF. Return code depend on the users respond.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment wich is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: 0 Suc., -1 Failure
 */

int     question_menu(index)
int     index;
{

        ASL_SCR_TYPE    menutypes= DM_TYPE_DEFAULTS ;
        menutypes.cur_index=1;
        switch (tmode[index].tu_num)
        {
                case LPFK_LIGHT_ON:
                        chk_asl_stat(diag_display(0x930008,catd,light_on,
                        DIAG_IO,ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                        break;

                case LPFK_LIGHT_OFF:
                        chk_asl_stat(diag_display(0x930009,catd,light_off,
                        DIAG_IO,ASL_DIAG_LIST_CANCEL_EXIT_SC,&menutypes,NULL));
                        break;
                default:
                        return(0);

        };

        if ( menutypes.cur_index == TRUE )      /* Users selection         */
        {
                put_title(0x930011);
                return (0);
        }
        return(-1);
} /* end question_menu */

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

void    chg_mn_adv()
{
        da_title[0].msgid++;
        wrp_yes_no[0].msgid++;
        rmv_cable[0].msgid++;
        connect_cbl[0].msgid++;
        light_on[0].msgid++;
        light_off[0].msgid++;
        order_w_inst[0].msgid++;
        order_inst[0].msgid++;
        random_w_inst[0].msgid++;
        random_inst[0].msgid++;
}

/*
 * NAME: report_fru()
 *
 * FUNCTION: Designed to insert a FRU bucket to the controller and set
 *           the appropriate exit code to the controller.
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
} /* end report_fru */

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
                if ( ( catd =  diag_catopen ( MF_DLPFK,0)) == CATD_ERR )
                {                         /* catalog file not found       */
                        DA_SETRC_ERROR( DA_ERROR_OTHER );
                        clean_up();
                }
        }

        msgstr = (char *) malloc (1200 *sizeof (char));
        msgstr1 = (char *) malloc (1200 *sizeof (char));
        lpp_configure_method= (char *) malloc (1200 *sizeof (char));
        lpp_unconfigure_method= (char *) malloc (1200 *sizeof (char));
        child1_configure_method= (char *) malloc (1200 *sizeof (char));
        child1_unconfigure_method= (char *) malloc (1200 *sizeof (char));
        lpfk_device_configure_method= (char *) malloc (1200 *sizeof (char));
        lpfk_device_unconfigure_method= (char *) malloc (1200 *sizeof (char));
        tty_device_configure_method = (char *) malloc (1200 *sizeof (char));
        tty_device_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        sa_device_configure_method = (char *) malloc (1200 *sizeof (char));
        sa_device_unconfigure_method = (char *) malloc (1200 *sizeof (char));
        cudv_p = (struct CuDv *) malloc (1200 *sizeof (char));
        pddv_p = (struct PdDv *) malloc (1200 *sizeof (char));
        if((msgstr == (char *) NULL) || (msgstr1 == (char *) NULL)
           || (lpp_unconfigure_method == (char *) NULL)
           || (child1_configure_method == (char *) NULL)
           || (child1_unconfigure_method == (char *) NULL)
           || (lpfk_device_configure_method == (char *) NULL)
           || (lpfk_device_unconfigure_method == (char *) NULL)
           || (tty_device_configure_method == (char *) NULL)
           || (tty_device_unconfigure_method == (char *) NULL)
           || (sa_device_configure_method == (char *) NULL)
           || (sa_device_unconfigure_method == (char *) NULL)
           || (cudv_p == (struct CuDv *) NULL)
           || (pddv_p == (struct PdDv *) NULL)
           || (lpp_configure_method == (char *) NULL))
        {
                DA_SETRC_ERROR (DA_ERROR_OTHER);
                clean_up();
        }

} /* end initialize_all */

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
} /* end chk_asl_stat */

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
        report_fru(&frub_bat_code[0]);
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

/************************************************************************/
/*      This procedure check if the parent is connected to gio card     */
/*      Return TRUE     : if LPFK is connected to gio card              */
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

        if (connect_to_gio()) {
                /* Set gio adapter and lpfk device to DIAGNOSE state */
                result = diagex_cfg_state (tm_input.parent);

                switch (result) {
                   case 1:
                   case 2:
                      DA_SETRC_ERROR(DA_ERROR_OPEN);
                      clean_up();
                      break;
                   case 3:
                      sprintf(msgstr, (char *)diag_cat_gets ( catd, HFT_ORD_OK,
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
                /* follow the 'ttydevice' link between a tty and the lpfk. */


                /* Find out if there is any dial that is the children of gio */
                /* and not MISSING.  MISSING is defined as 3.                */

                cudv_p = (struct CuDv *) malloc (1200 *sizeof (char));
                sprintf (criteria, "name like dials*  AND status=1");

                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                if (( cudv_p != (struct CuDv *) NULL )&&( cudv_p != (struct CuDv *)-1 ))
                {
                        /* Search for the device config and unconfigure method */

                        strcpy (child1, cudv_p->name);
                        /* find out the config and unconfig method of this child1 */
                        strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                        sprintf (criteria, "uniquetype=%s", uniquetype);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &v_info, 1, 1);
                        strcpy (child1_unconfigure_method, pddv_p->Unconfigure);
                        strcpy (child1_configure_method, pddv_p->Configure);

                        sprintf (option," -l %s", child1);
                        result = odm_run_method(child1_unconfigure_method,
                                option, &new_out,&new_err);
                        if (result != -1)
                        {
                                child1_unconfigure_lpp = TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                   catd, HFT_ORD_OK,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                   cudv_p->name, cudv_p->location);
                                menugoal (msgstr);
                                clean_up();
                        }
                }

                /* find out if device is available */

                sprintf (criteria, "name=%s AND chgstatus!=3 ", tm_input.dname);
                cudv_p = get_CuDv_list(CuDv_CLASS, criteria, &v_info, 1, 1);
                if (( cudv_p != (struct CuDv *) NULL ) && ( cudv_p != (struct CuDv *)-1 ))
                {
                        /* Device is not missing        */
                        strcpy (lpfk_device, cudv_p->name);
                        /* find out the config and unconfig method of this child1 */
                        strcpy (uniquetype, cudv_p->PdDvLn_Lvalue);
                        sprintf (criteria, "uniquetype like %s", uniquetype);
                        pddv_p = get_PdDv_list (PdDv_CLASS, criteria, &v_info, 1, 1);
                        if(!strncmp(pddv_p->subclass, "sgio", 4))
                                serial_gio = TRUE;
                        if(cudv_p->status == 1) {
                                strcpy (lpfk_device_unconfigure_method, pddv_p->Unconfigure);
                                strcpy (lpfk_device_configure_method, pddv_p->Configure);

                                sprintf (option," -l %s", lpfk_device);
                                result = odm_run_method(lpfk_device_unconfigure_method,
                                        option, &new_out,&new_err);
                                if (result != -1)
                                {
                                        lpfk_device_unconfigure_lpp = TRUE;
                                }
                                else
                                {
                                        sprintf(msgstr, (char *)diag_cat_gets (
                                             catd, HFT_ORD_OK,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                                cudv_p->name, cudv_p->location);
                                        menugoal (msgstr);
                                        clean_up();
                                }
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
                                                catd, HFT_ORD_OK,
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
                                                catd, HFT_ORD_OK,
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
                                                catd, HFT_ORD_OK,
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
                                                catd, HFT_ORD_OK,
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
                                option, &new_out,&new_err);
                        if (result != -1)
                        {
                                unconfigure_lpp = TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                    catd, HFT_ORD_OK,LPP_DEVICE_CANNOT_UNCONFIGURED),
                                        cudv_p->name, cudv_p->location);
                                menugoal (msgstr);
                                clean_up();
                        }
                        sprintf (option," -l %s", gio_device_name);
                        strcpy (criteria,"/usr/lib/methods/cfgdiag");

                        result = odm_run_method(criteria,option, &new_out,&new_err);
                        if (result != -1)
                        {
                                set_device_to_diagnose= TRUE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (catd, HFT_ORD_OK,
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
                                  catd, HFT_ORD_OK, LPP_DEVICE_CANNOT_SET_TO_DEFINE),
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
                                 &new_out,&new_err);
                        if (result != -1 )
                        {
                                set_device_to_diagnose= FALSE;
                        }
                        else
                        {
                                sprintf(msgstr, (char *)diag_cat_gets (
                                  catd,HFT_ORD_OK,LPP_DEVICE_CANNOT_SET_TO_DEFINE),
                                         tm_input.dname, tm_input.dnameloc);
                                menugoal (msgstr);
                        }
                }

                /* Configure lpp device driver (gio) first      */

                if (gio_device_state == AVAILABLE)
                {
                        sprintf (option," -l %s", gio_device_name);
                        rc= odm_run_method(lpp_configure_method,option,
                                           &new_out,&new_err);
                        if (rc == -1)
                        {
                                sprintf(msgstr,(char *)diag_cat_gets(catd,HFT_ORD_OK ,
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

                /* Configuring child 1  Dials                   */
                if (child1_unconfigure_lpp)
                {
                        sprintf (option," -l %s", child1);
                        rc= odm_run_method(child1_configure_method,option,
                                           &new_out,&new_err);
                }

                /* Configuring child 2          lpfk            */
                if (lpfk_device_unconfigure_lpp)
                {
                        sprintf (option," -l %s", lpfk_device);
                        rc= odm_run_method(lpfk_device_configure_method,option,
                                           &new_out,&new_err);
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
        if(child1_configure_method != (char *) NULL)
                free (child1_configure_method);
        if(child1_unconfigure_method != (char *) NULL)
                free (child1_unconfigure_method);
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
        alarm(60);
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

