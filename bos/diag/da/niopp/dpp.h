/* @(#)05       1.14  src/bos/diag/da/niopp/dpp.h, daniopp, bos412, 9446B 11/11/94 10:23:52 */
/*
 * COMPONENT_NAME: DANIOPP
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the Standard Parallel Port diagnostic application.
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 *
 */

#include        <stdio.h>
#include        <fcntl.h>
#include        <signal.h>
#include        <cf.h>
#include        <nl_types.h>
#include        <limits.h>
#include        <locale.h>
#include        <sys/devinfo.h>
#include        <sys/ioctl.h>
#include        <sys/types.h>
#include        <sys/mdio.h>
#include        <sys/errno.h>
#include        "diag/diagodm.h"
#include        <sys/cfgodm.h>
#include        "diag/tm_input.h"
#include        "diag/tmdefs.h"
#include        "diag/da.h"
#include        "diag/diag.h"
#include        "diag/diago.h"
#include        "diag/diag_exit.h"
#include        "diag/atu.h"
#include        "diag/ttycb.h"
#include        "diag/dcda_msg.h"
#include        "dpp_msg.h"

extern  int     getdainput ();
extern  int     putdavar ();
extern  int     getdavar ();
extern  long    diag_display ();
extern  long    diag_asl_read ();
extern  char    diag_cat_gets ();
extern  int     insert_frub ();
extern  int     exectu ();
extern  int     addfrub ();
extern  int     errno;
extern  nl_catd diag_catopen (char *, int);

/*  diagnostic application mode definitions  */

#ifndef ZERO
#define ZERO                    0
#endif
#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    1
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
#define ADV_T                   ADVANCED_TRUE           /* 1 */
#define ADV_F                   ADVANCED_FALSE          /* 0 */
int     a_mode;

/*  system - short: TRUE or FALSE  */
#define SYS_T                   SYSTEM_TRUE             /* 1 */
#define SYS_F                   SYSTEM_FALSE            /* 0 */
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
#define CONS_T                  CONSOLE_TRUE            /* 1 */
#define CONS_F                  CONSOLE_FALSE           /* 0 */
int     c_mode;

/*  run loop mode tests  */
#define L_T                     0x0c0

/*  run non-interactive tests  */
#define N_I                     0x0a0
#define R_0                     0x0d0

/*  run regression test set 1  */
#define R_1                     0x0e0

/*  run regression test set 2  */
#define R_2                     0x0f0

/*  run all tests  */
#define A_T                     0x0b0
#define INVALID_TM_INPUT        -1
int     testmode;
int     odm_flag = -1;

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
int     diskette = FALSE;

/*      CPU id      */

#define	FIREBALL	0x080000a6
#define	PEGASUS		0x080000a0
#define	PANOLA		0x080000a3

/*      Planar types - These defines are in ttycb.h      */

/* #define SIO			95	* 0x5f - SIO planar (release 1)	*/
/* #define SIO2			230	* 0xe6 - SIO planar (Release 2)	*/
/* #define SIO3			254	* 0xfe - SIO planar (Salmon)	*/
/* #define SIO7			217	* 0xD9 - SIO planar (IOD)	*/

/*      Test unit definitions   */

#define TEST10                  0x10
#define TEST20                  0x20
#define TEST30                  0x30
#define TEST40                  0x40
#define TEST50                  0x50
#define TEST60                  0x60
int     tu60 = FALSE;
int     tu50 = FALSE;
int     tu40 = FALSE;
int     tu30 = FALSE;

/*      Device driver file and catalog file variables   */

#ifndef CATD_ERR
#define CATD_ERR        -1
#endif
nl_catd catd = CATD_ERR;
int     ddfd = -1;
char    devppa[32];
char    devlpn[NAMESIZE];
int     mddfd = -1;
char    devmdd[] = "/dev/nvram";
#define OPEN_RWND (O_RDWR | O_NDELAY)
#define OPEN_LPX (open(devppa,OPEN_RWND))
#define OPEN_MODE (O_RDWR)
#define OPEN_MDD (open(devmdd,OPEN_MODE))

/*      Structures and variables used by Parallel diagnostics   */

struct  sigaction act;          /* interrupt handler vector structure   */
void    int_handler(int);       /*  interrupt handler  */
char    posreg[] = {0, 0, 0, 0, 0, 0};
int     maxreg = 4;
MACH_DD_IO              mddRecord;
int     io_rc;          /* Return code from ioctl function      */
#define MAXCONN         1
#define MAXLEVELS       3
char    parents[MAXLEVELS][NAMESIZE+1] = { "", "", "", };
int     made_device = FALSE;
int     undo_device = FALSE;
int     made_lp0;
char    made_dev[128];
char    made_prt[128];
char    lp_name[NAMESIZE];
int     drivers_open = FALSE;
struct  tm_input        da_input;
struct  listinfo        p_info;
struct  PDiagDev        *pdiagdev;
struct  PdDv            *pddv;
struct  PdCn            *pdcn;
int     num_PdCn;
struct  listinfo        c_info;
struct  CuDv            *P_cudv;
struct  CuDv            *C_cudv;
int     num_CuDv;
struct  tucb_data       tucb_ptr;
struct  fru_bucket      frub[] =
{
        { "", FRUB1, 0x827, 0, 0,
                {
                        { 0, "", "", 0, DA_NAME, NONEXEMPT },
                        { 0, "", "", 0, 0, '\0' },
                },
        },
};
int     fru_set = FALSE;
int     tu_err = FALSE;
int     conf1 = 100;
int     conf2 = 90;
int     conf3 = 10;
int     conf4 = 95;
int     conf5 = 5;
int     asl_rc = DIAG_ASL_OK;
long    menu_nmbr = 0x827000;
int     slctn;
int     wrap_7 = TRUE;          /* parallel port wrap plug flag              */
