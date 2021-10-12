/* @(#)33       1.2  src/bos/diag/da/mpa/dampa.h, mpada, bos411, 9428A410j 10/14/93 11:52:43 */
/*
 *   COMPONENT_NAME: (MPADIAG) MP/A DIAGNOSTICS
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_DMPA
#define _H_DMPA

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
#include        "dampa_msg.h"
#include        "../../tu/mpa/mpa_tu.h"

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
extern  int     user_said_no_wrap;
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

/*  exenv - short: IPL, STD, RGR, CNC, or EXR  */
#define IPL                     EXENV_IPL               /* 1 */
#define STD                     EXENV_STD               /* 2 */
#define RGR                     EXENV_REGR              /* 3 */
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

/* customer, system checkout, all noninteractive tests */
#define N_I                     0x0a0
#define R_0                     0x0d0
#define SET_0                   0x07
int     set0[7] =
	    {1, 5, 7, 36, 39, 52, 55 };

/* note: 36-39 decimal = 0x24-0x27 , they imply to run four different */
/*       versions of tu'2. The DA will use the high order nibble to   */
/*       decide which tu to run, and the low order nibble to set up   */
/*       the input to the tu.                                         */
/*       TU 3 works the same way.                                     */
/*  NOTE: 0-3 do external wrap test, 4-7 internal wrap test.          */

/* regression test 1, all noninteractive tests plus adapter wrap test */
#define R_1                     0x0e0
#define SET_1                   0x07


/* advanced diagnostics, all noninteractive/interactive tests */
#define A_T                     0x0b0
#define SET_3                   0x0A
int     set3[10] =
      {1, 5, 7, 6, 32, 37, 48, 50, 52, 54};

#define INVALID_TM_INPUT        -1
int     testmode;

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
int     tu10 = FALSE;
int     tu15 = FALSE;

/*  Structures used by MP/A diagnostics  */
struct  sigaction act;          /* interrupt handler vector structure   */
struct  tm_input da_input;
struct  stat tmpbuf;
TUTYPE  tucb_ptr;

struct  fru_bucket frub[] =
{
	{ "", FRUB1, 0x996, 0, 0,
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
int     no_drvr     = 0;   /* set to 1 if initially there is no drvr */
int     change_drvr = 0;   /* set to 1 if drivers were changed       */
char    diagdrvr[32];           /* diag driver name */
char    savedrvr[32];           /* save driver name */
char    tmpstr[32];             /* for removing index from device names */

/*  Device driver file and catalog file variables  */
#ifndef MPAD_ERR
#define MPAD_ERR                -1
#endif
nl_catd catd = MPAD_ERR;        /*  dampa.cat file descripter  */
nl_catd dc_catd = MPAD_ERR;     /*  dcda.cat file descripter  */
char    dc_name[NAMESIZE];      /*  dcda.cat file name  */
int     Set_num;                /*  set number in dcda.cat file  */
int     Msg_num;                /*  message number in dcda.cat file  */
int     fdes = -1;              /*  dampa file descriptor  */
char    devcat[NAMESIZE+8];     /*  special file path  */

/*  Wrap plug part numbers and flags  */
int     wrap_1 = TRUE;          /*   wrap plug flag  */

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
