/* @(#)22	1.4  src/bos/usr/include/diag/da_cmn.h, cmddiag, bos411, 9428A410j 2/19/91 17:14:26 */
/*
 *   COMPONENT_NAME: CMDDIAG
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


/******************************************************************************/
/*             diagnostic application mode bit definitions                    */
/******************************************************************************/

#define NOTLM           LOOPMODE_NOTLM
#define ENTERLM         LOOPMODE_ENTERLM
#define INLM            LOOPMODE_INLM
#define EXITLM          LOOPMODE_EXITLM
#define CONSOLE         CONSOLE_TRUE
#define NO_CONSOLE      CONSOLE_FALSE
#define ADVANCED        ADVANCED_TRUE
#define NOT_ADVANCED    ADVANCED_FALSE
#define SYSTEM          SYSTEM_TRUE
#define NOT_SYSTEM      SYSTEM_FALSE
#define IPL             EXENV_IPL
#define STD             EXENV_STD
#define REGR            EXENV_REGR
#define SYSX            EXENV_SYSX
#define CONC            EXENV_CONC
#define ELA             DMODE_ELA
#define PD              DMODE_PD
#define REPAIR          DMODE_REPAIR
#define MS1             DMODE_MS1
#define MS2             DMODE_MS2
#define FREELANCE       DMODE_FREELANCE

/******************************************************************************/
/*      Definitions for all the test cases                                    */
/******************************************************************************/

#define NO_MENU_TEST_MODE       0x0a0
#define ALL_TESTS_MODE          0x0b0
#define LOOP_MODE_TESTS         0x0c0
#define INVALID_TM_INPUT           -1

#define TEST10  10
#define TEST20  20
#define TEST30  30
#define TEST40  40
#define TEST50  50
#define TEST60  60
#define TEST70  70
#define TEST80  80
#define TEST90  90

#define uchar       unsigned char
nl_catd  catd;          /* pointer to the catalog file                       */

long    menu_nmbr;              /* display a menu number to the user         */
int     wrap_1 = TRUE;          /* standard serial port wrap plug flag       */
int     wrap_2 = TRUE;          /* 8 port adapter wrap plug flag             */
int     wrap_3 = TRUE;          /* 16 port adapter wrap plug flag            */
int     wrap_4 = TRUE;          /* 64 port controller wrap plug flag         */
int     wrap_5 = TRUE;          /* 64 port controller cable wrap plug flag   */
int     wrap_6 = TRUE;          /* concentrator box wrap plug flag           */
int     wrap_7 = TRUE;          /* D shell cable wrap plug flag              */
int     da_sn = 0;
int     da_mode;                /* execution mode for diagnostics            */
int     l_mode;                 /* NOTLM, ENTERLM, INLM, EXITLM              */
int     c_mode;                 /* CONSOLE_TRUE, CONSOLE_FALSE               */
int     a_mode;                 /* ADVANCED_TRUE, ADVANCED_FALSE             */
int     s_mode;                 /* SYSTEM_TRUE, SYSTEM_FALSE                 */
int     e_mode;                 /* IPL, STD, MNT, CONC                       */
int     d_mode;                 /* PD, REPAIR, ELA, MS1, MS2                 */
short   conf1 = 100;
short   conf2 = 90;
short   conf3 = 10;
short   conf4 = 85;
short   conf5 = 15;
short   conf6 = 95;
short   conf7 = 5;

extern  int     getdainput();
extern  int     putdavar();
extern  int     getdavar();
extern  long    diag_display();
extern  long    diag_msg();
extern  long    diag_msg_nw();
extern  int     insert_frub();
extern  int     exectu();
extern  int     addfrub();
extern  int     menugoal();
extern  int     errno;

struct  tucb_t tucb_ptr;
struct  tm_input tm_input;
