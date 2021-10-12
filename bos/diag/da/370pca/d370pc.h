/* @(#)69       1.3  src/bos/diag/da/370pca/d370pc.h, da370pca, bos41J, 9523C_all 6/2/95 17:18:13 */
/*
 * COMPONENT_NAME: DA370PCA
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the 370 Parallel Channel Attach diagnostic application.
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_D370PC
#define _H_D370PC

#include        <stdio.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <errno.h>
#include        <cf.h>
#include        <nl_types.h>
#include        <limits.h>
#include        <locale.h>
#include        <memory.h>
#include        <unistd.h>
#include        <sys/devinfo.h>
#include        <sys/types.h>
#include        "diag/diagodm.h"
#include        <sys/cfgodm.h>
#include        "diag/diago.h"
#include        "diag/diag.h"
#include        "diag/da.h"
#include        "diag/tm_input.h"
#include        "diag/tmdefs.h"
#include        "diag/diag_exit.h"
#include        "../../tu/370pca/pscatu.h"
#include        "diag/dcda_msg.h"
#include        "d370pc_msg.h"

/*  External functions and variables  */
extern  int     getdainput ();
extern  int     putdavar ();
extern  int     getdavar ();
extern  long    diag_display ();
extern  long    diag_asl_read ();
extern  char    diag_cat_gets ();
extern  int     file_present ();
extern  int     insert_frub ();
extern  int     addfrub ();
extern  int     exectu ();
extern  int     errno;
extern  nl_catd diag_catopen (char *, int);

/*  Functions  */
int     setdamode ();           /*  select test units to be run  */
int     err_log ();             /*  check error log  */
void    int_handler(int);       /*  interrupt handler  */
int     diag_asl_stat ();       /*  check asl return code  */
int     diag_stdby ();          /*  display diagnostic standby screens  */
int     diag_y_n ();            /*  display yes|no menus  */
int     diag_action ();         /*  display user action required screens  */

/*  diagnostic application mode definitions  */
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    1
#endif
#ifndef DONE
#define DONE                    1
#endif
#ifndef ZERO
#define ZERO                    0
#endif
#ifndef FAIL
#define FAIL                    -1
#endif
#define Null                    '\0'

/*  exenv - short: IPL, STD, CNC, or EXR  */
#define IPL                     EXENV_IPL               /* 1 */
#define STD                     EXENV_STD               /* 2 */
#define CNC                     EXENV_CONC              /* 4 */
#define EXR                     EXENV_SYSX              /* 5 */
int     e_mode;

/*  advanced - short: TRUE or FALSE  */
#define ADT                     ADVANCED_TRUE           /* 1 */
#define ADF                     ADVANCED_FALSE          /* 0 */
int     a_mode;

/*  system - short: TRUE or FALSE  */
#define SYT                     SYSTEM_TRUE             /* 1 */
#define SYF                     SYSTEM_FALSE            /* 0 */
int     s_mode;

/*  dmode - short: ELA, PD, RPR, MS1, MS2  */
#define ELA                     DMODE_ELA               /* 1 */
#define PD                      DMODE_PD                /* 2 */
#define RPR                     DMODE_REPAIR            /* 4 */
#define MS1                     DMODE_MS1               /* 8 */
#define MS2                     DMODE_MS2               /* 16 */
int     d_mode;

/*  loopmode - short: NTL, ENL, INL or EXL  */
#define NT_LP                   LOOPMODE_NOTLM          /* 1 */
#define EN_LP                   LOOPMODE_ENTERLM        /* 2 */
#define IN_LP                   LOOPMODE_INLM           /* 4 */
#define EX_LP                   LOOPMODE_EXITLM         /* 8 */
int     l_mode;

/*  console - short: TRUE or FALSE  */
#define CNT                     CONSOLE_TRUE            /* 1 */
#define CNF                     CONSOLE_FALSE           /* 0 */
int     c_mode;

/*  no tests run  */
#define NO_TEST                 0x0c0

/* customer, system checkout, exerciser, all noninteractive tests */
#define N_I                     0x0a0
#define SET_0                   0x0f
int     set0[15] =
            {2, 7, 23, 1, 14, 4, 5, 6, 3, 12, 17, 18, 25, 24, 9};

/* loopmode test 1, all noninteractive tests plus adapter wrap test */
#define R_1                     0x0e0
#define SET_1                   0x010
int     set1[16] =
            {2, 7, 23, 1, 14, 4, 5, 6, 3, 12, 17, 18, 25, 24, 9, 10};

/* advanced diagnostics, all noninteractive/interactive tests */
#define A_T                     0x0b0
#define SET_3                   0x011
int     set3[17] =
            {2, 7, 23, 1, 14, 4, 5, 6, 3, 12, 17, 18, 25, 24, 9, 10, 15};

#define INVALID_TM_INPUT        -1
int     testmode;
int     maxtest;

/*  Exit codes  */
#define NO_TST                  0       /*  No tests run  */
#define TU_GOOD                 32      /*  Tests successful  */
#define TU_BAD                  33      /*  Test failed - hardware  */
#define USR_EXIT                34      /*  User exit - hardware ok  */
#define USR_QUIT                36      /*  User quit - hardware ok  */
#define FAIL_OPN                40      /*  Open failed - device driver  */
#define SW_ERR                  48      /*  Software error - hardware ok  */
#define SUB_TST                 64      /*  Subtest - hardware ok  */
#define SHR_TST                 128     /*  Shared test - hardware ok  */
#define MNU_GOAL                256     /*  Menu goal  */
int     exitrc;

/*  Test unit definitions  */
#define TEST1                   1
#define TEST2                   2
#define TEST3                   3
#define TEST4                   4
#define TEST5                   5
#define TEST6                   6
#define TEST7                   7
#define TEST9                   9
#define TEST10                  10
#define TEST12                  12
#define TEST14                  14
#define TEST15                  15
#define TEST17                  17
#define TEST18                  18
#define TEST21                  21
#define TEST23                  23
#define TEST24                  24
#define TEST25                  25
int     tu10 = FALSE;
int     tu15 = FALSE;

/*  Structures used by 370pca diagnostics  */
struct  sigaction act;          /* interrupt handler vector structure   */
struct  tm_input da_input;
struct  stat tmpbuf;
TUTYPE  tucb_ptr;

struct  fru_bucket frub[] =
{
        { "", FRUB1, 0x862, 0, 0,
                {
                        { 0, "", "", 0, 0, '\0'  },
                        { 0, "", "", 0, 0, '\0'  },
                        { 0, "", "", 0, 0, '\0'  },
                },
        },
};  /* end frub */

/*  Configuration defines and variables  */
int     diskette = FALSE;
int     made_device = 0;
int     diag_ucd = TRUE;

/*  Device driver file and catalog file variables  */
#ifndef CATD_ERR
#define CATD_ERR                -1
#endif
nl_catd catd = CATD_ERR;        /*  d370pc.cat file descripter  */
nl_catd dc_catd = CATD_ERR;     /*  dcda.cat file descripter  */
char    dc_name[NAMESIZE];      /*  dcda.cat file name  */
char    diag_ucode[] = "fe92d";
char    diag_ucode_path[80];
char    func_ucode[] = "fe92";
char    func_ucode_path[80];
int     Set_num;                /*  set number in dcda.cat file  */
int     Msg_num;                /*  message number in dcda.cat file  */
int     fdes = -1;              /*  d370pc file descriptor  */
char    devcat[NAMESIZE+8];     /*  special file path  */

/*  Wrap plug part numbers and flags  */
int     wrap_1 = TRUE;          /*  78 pin wrap plug flag  */
int     wrap_2 = TRUE;          /*  bus out wrap plug flag  */
int     wrap_3 = TRUE;          /*  tag out wrap plug flag  */
int     ch_off = FALSE;         /*  channel status flag  */

/*  Menu and message variables  */
long    Menu_nmbr;              /*  display a menu number to the user  */
int     Msg_nmbr;
int     slctn;

/*  FRU variables  */
int     tu_err = 0;
int     fru_set = FALSE;
int     conf1 = 100;
int     conf2 = 95;
int     conf3 = 90;
int     conf4 = 80;
int     conf5 = 10;
int     conf6 = 05;
char    fnam1[] = "Cable assembly";
char    fnam2[] = "Software error";

#endif
