/* @(#)26       1.12.2.21  src/bos/diag/da/async/dasync.h, daasync, bos41J, 9520B_all 5/17/95 11:15:19 */
/*
 * COMPONENT_NAME: DAASYNC
 *
 * FUNCTIONS:   This file contains global defines, variables, and structures
 *              for the Async adapter diagnostic application.
 *
 * ORIGINS: 27, 83
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * LEVEL 1, 5 Years Bull Confidential Information
 */


#include        <stdio.h>
#include        <string.h>
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
#include        <sys/ioctl.h>
#include        <sys/types.h>
#include        <sys/mdio.h>
#include        <sys/rs.h>
#include        <sys/cxma.h>
#include        <sys/stat.h>
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
#include        "dasync_msg.h"
#include        <sys/termio.h>
#include        "diag/modid.h"

void    int_handler(int);       /* interrupt handler */

/*      External functions and variables        */

extern  int     getdainput ();
extern  int     putdavar ();
extern  int     getdavar ();
extern  long    diag_display ();
extern  long    diag_asl_read ();
extern  char    diag_cat_gets ();
extern  char    diag_device_gets ();
extern  int     file_present ();
extern  int     exectu ();
extern  int     insert_frub ();
extern  int     addfrub ();
extern  int     errno;
extern  nl_catd diag_catopen (char *, int);
extern  nl_catd diag_device_catopen (char *, int);

/*      diagnostic application mode definitions         */

#ifndef FALSE
#define FALSE                   0
#endif
#ifndef TRUE
#define TRUE                    1
#endif
#ifndef FAIL
#define FAIL                    -1
#endif
#define NOTLM                   LOOPMODE_NOTLM
#define ENTERLM                 LOOPMODE_ENTERLM
#define INLM                    LOOPMODE_INLM
#define EXITLM                  LOOPMODE_EXITLM
#define CONSOLE                 CONSOLE_TRUE
#define NO_CONSOLE              CONSOLE_FALSE
#define ADVANCED                ADVANCED_TRUE
#define NOT_ADVANCED            ADVANCED_FALSE
#define SYSTEM                  SYSTEM_TRUE
#define NOT_SYSTEM              SYSTEM_FALSE
#define IPL                     EXENV_IPL
#define STD                     EXENV_STD
#define REGR                    EXENV_REGR
#define CONC                    EXENV_CONC
#define SYSX                    EXENV_SYSX
#define ELA                     DMODE_ELA
#define PD                      DMODE_PD
#define REPAIR                  DMODE_REPAIR
#define MS1                     DMODE_MS1
#define MS2                     DMODE_MS2
#define FREELANCE               DMODE_FREELANCE
#define NO_MENU_TEST_MODE       0x0a0
#define ALL_TESTS_MODE          0x0b0
#define LOOP_MODE_TESTS         0x0c0
#define INVALID_TM_INPUT        -1
#define Null                    '\0'
#define Console                 2
#define Available               1
#define In_Use                  0
#define N_T                     0
#define F_G                     32
#define F_B                     33
#define F_E_G                   34
#define F_E_B                   35
#define F_Q_G                   36
#define F_Q_B                   37
#define F_O                     40
#define F_O_G                   48
#define F_O_B                   49
#define S_G                     64
#define SH_G                    128
#define M_G                     256
#define SET                     1
#define RESET                   2
int     diskette = FALSE;
int     dd_state = Available;
int     da_mode;                /* execution mode for diagnostics         */
int     l_mode;                 /* NOTLM, ENTERLM, INLM, EXITLM           */
int     c_mode;                 /* CONSOLE_TRUE, CONSOLE_FALSE            */
int     a_mode;                 /* ADVANCED_TRUE, ADVANCED_FALSE          */
int     s_mode;                 /* SYSTEM_TRUE, SYSTEM_FALSE              */
int     e_mode;                 /* IPL, STD, MNT, CONC                    */
int     d_mode;                 /* PD, REPAIR, ELA, MS1, MS2              */

/*      Test unit definitions   */

#define TEST10                  10
#define TEST20                  20
#define TEST30                  30
#define TEST40                  40
#define TEST50                  50
#define TEST60                  60
#define TEST70                  70
#define TEST80                  80
#define TEST110                 110
int     tu80 = FALSE;
int     tu70 = FALSE;
int     tu60 = FALSE;
int     tu50 = FALSE;
int     tu40 = FALSE;
int     tu30 = FALSE;

/*      Async adapter and standard serial port LED values       */

#define SP1                     0x826   /* Standard serial port S1        */
#define SP2                     0x831   /* Standard serial port S2        */
#define SP3                     0x946   /* Standard serial port S3        */
#define EIA_232_8               0x841   /* 8-port EIA-232 adapter         */
#define EIA_232_8ISA            0x830   /* 8-port EIA-232 adapter (ISA)   */
#define EIA_422_8               0x842   /* 8-port EIA-422 adapter         */
#define M_S_188_8               0x843   /* 8-port MIL_STD-188 adapter     */
#define EIA_232_16              0x847   /* 16-port EIA-232 adapter        */
#define EIA_422_16              0x848   /* 16-port EIA-422 adapter        */
#define EIA_232_64              0x834   /* 64-port EIA-232 controller     */
#define EIA_232_128             0x836   /* 128-port EIA-232 controller    */
#define EIA_232_128ISA          0x709   /* 128-port EIA-232 ctrlr (ISA)   */
#define EIA_232_16CONC          0x837   /* 128-port EIA-232 RAN           */
#define MAXCONN                 128     /* Maximum number of ports on an  */
                                        /* individual async adapter       */

/*      The following defines correspond to the message numbers in dcda.msg
 *      and should not be changed.  Also if making any changes to the async
 *      messages in dcda.msg never insert any new messages between the
 *      existing messages.                                                */

#define RM_ICPN                 2       /* FRU description message        */
#define RM_ACPN                 3       /* FRU description message        */
#define RM_CIPN                 4       /* FRU description message        */
#define RM_CPN6                 6       /* FRU description message        */
#define RM_CPN7                 7       /* FRU description message        */
#define RM_CC8                  8       /* error reason message           */
#define RM_CC9                  9       /* error reason message           */
#define RM_TU10                 10      /* tu10 error reason message      */
#define RM_TU20                 11      /* tu20 error reason message      */
#define RM_TU30                 12      /* tu30 error reason message      */
#define RM_TU40                 13      /* tu40 error reason message      */
#define RM_TU50                 14      /* tu50 error reason message      */
#define RM_TU60                 15      /* tu60 error reason message      */
#define RM_TU70                 16      /* tu70 error reason message      */
#define RM_TU80                 17      /* tu80 error reason message      */
#define RM_LM                   18      /* loop test error reason message */
#define RM_PN19                 19      /* FRU description message        */
#define RM_PN20                 20      /* FRU description message        */
#define RM_TU60C                21      /* tu60 error reason message      */
#define RM_PN22                 22      /* FRU description message        */
#define RM_PN23                 23      /* FRU description message        */
#define RM_CPN24                24      /* FRU description message        */
#define RM_PTCPN                25      /* FRU description message        */



/*      The next set of defines correspond to message identifiers in the
 *      catalog file set ASYNC_CABLE.  If changes are made to this set
 *      in the catalog, the appropriate change must be made to these
 *      defines or incorrect devices will be called out on the SRN menu.  */

#define CONVERTER_CABLE         1
#define CABLE_ASSEMBLY          2
#define INTERPOSER              3
#define CONCENTRATOR            4
#define NULL_MSG                5
#define CABLE                   6
#define REMOTE_ASYNC_NODE       7
#define ADAPTER                 8
#define CONTROLLER_LINE         9



/*      Structures used by async diagnostics    */

struct  sigaction act;          /* interrupt handler vector structure   */
char    posreg[4];
MACH_DD_IO      *mddRecord;
struct  tm_input da_input;
struct  PDiagDev *pdiagdev;
struct  PdDv *pddv;
struct  PdCn *pdcn;
int     num_PdCn;
struct  CuDv *P_cudv;
struct  CuDv *D_cudv;
struct  CuDv *C_cudv;
int     num_CuDv;
struct  listinfo c_info;
struct  listinfo p_info;
struct  tucb_data tucb_ptr;
struct  CuAt *t_cuat;
struct  CuAt *d_cuat;
struct  CuAt *a_cuat;
int     num_CuAt;
char    ba[] = "speed";
char    bv[8];
char    pa[] = "parity";
char    pv[] = "none";
char    ca[] = "bpc";
char    cv[] = "8";
char    sa[] = "stops";
char    sv[] = "1";
int     arblvl;
struct  rs_info Irs_info;
struct  rs_info Ors_info;
struct  fru_bucket frub[] =
{
        { "", FRUB1, 0, 0, 0,
                {
                        { 0, "", "", 0, 0, '\0'  },
                        { 0, "", "", 0, 0, '\0'  },
                },
        },
};  /* end frub */


/*      Configuration defines and variables     */

#define LAMPASAS  0x00000004
#define MACH_MASK 0x0000000c
#define RAINBOW3  0x08010046
#define RAINBOW3P 0x08010049
#define FIREBALL  0x080000a6
#define PANOLA    0x080000a3
int     lampasas = FALSE;
int     sacasil = FALSE;
#define MAXLEVELS 4
char    parents[MAXLEVELS][NAMESIZE+1] = { "", "", "" };
int     made_device = 0;
char    pdef[] = "pd";
char    pcfg[] = "pc";


/*      Device driver file and catalog file variables   */

#ifndef CATD_ERR
#define CATD_ERR                -1
#endif
nl_catd catd = CATD_ERR;        /* Device catalog file descripter         */
nl_catd dv_catd = CATD_ERR;     /* System catalog file descripter         */
char    dv_name[NAMESIZE];      /* System catalog file name               */
int     Set_num;                /* set number in system catalog file      */
int     Msg_num;                /* message number in system catalog file  */
int     fdes = -1;              /* pointer to the device driver file      */
char    devtty[32];             /* device file name                       */
char    lp_name[NAMESIZE];      /* Printer device driver file name        */
int     dtr_set = FALSE;        /* True if diagnostics changes dtr        */
char    dd_name[DD_NAME_LEN];   /* Device driver name                     */
int     mddfd = -1;
char    devmdd[] = "/dev/nvram";
#define OPEN_TTY (O_RDWR | O_NDELAY)
#define OPEN_MODE (O_RDWR)
#define OPEN_MDD (open (devmdd, OPEN_MODE))

/*      Wrap plug and cable part numbers and flags      */
char   *wp_1 = "6298966";     /* 10 pin standard serial port wrap plug    */
char   *wp_2 = "22F9694";     /* 78 pin 8 port adapter wrap plug          */
char   *wp_3 = "53F3312";     /* 78 pin 16 port adapter wrap plug         */
char   *wp_4 = "53F3623";     /* 8 pin 64 port controller wrap plug       */
char   *wp_5 = "53F3205";     /* 8 pin 64 port controller cable wrap plug */
char   *wp_6 = "53F3624";     /* 8 pin concentrator box wrap plug         */
char   *wp_7 = "6298964";     /* 25 pin D shell cable wrap plug - rs232   */
char   *wp_8 = "30F9159";     /* 25 pin D shell cable wrap plug - rs422   */
char   *wp_9 = "43G0928";     /* 10 pin RAN wrap plug                     */
char   *wp_10 = "43G0926";    /* 15 pin D-SUB terminator for 128 port     */
char   *wp_11 = "6298965";    /* 9 pin standard serial port wrap plug     */
char   *cc_pn1 = "59F4533";   /* Converter cable for SIO planar           */
char   *cc_pn2 = "59F3740";   /* Converter cable for SIO planar           */
char   *cc_pn3 = "31F4590";   /* Converter cable for SIO planar           */
char   *cc_pn4 = "31F4126";   /* Converter cable for SIO planar           */
char   *cc_pn5 = "6450242";   /* 9-pin to 25-pin converter cable          */
char   *wrp_plg;              /* Wrp_plg == wp_1 through wp_10 depending  */
                              /* upon test and attached device.           */
int     wrap_1 = TRUE;        /* standard serial port wrap plug flag      */
int     wrap_2 = TRUE;        /* 8 port adapter wrap plug flag            */
int     wrap_3 = TRUE;        /* 16 port adapter wrap plug flag           */
int     wrap_4 = TRUE;        /* 64 port controller wrap plug flag        */
int     wrap_5 = TRUE;        /* 64 port controller cable wrap plug flag  */
int     wrap_6 = TRUE;        /* concentrator box wrap plug flag          */
int     wrap_7 = TRUE;        /* D shell modem cable wrap plug flag       */
int     wrap_8 = TRUE;        /* D shell terminal cable wrap plug flag    */
int     wrap_9 = TRUE;        /* D shell controller wrap plug             */
int     wrap_10 = TRUE;       /* D-SUB terminator                         */
int     wrap_11 = TRUE;       /* 9 pin standard serial port wrap plug     */
int     ccbl = TRUE;          /* Converter cable for SIO planar           */
int     sync_err = 0;

/*      Menu and message variables      */

#define MI_CABLE        0       /* Modem Cable and Interposer                */
#define PT_CABLE        1       /* Printer/Terminal Cable                    */
long    Menu_nmbr;              /* display a menu number to the user         */
int     slctn;
int     slct_port = 1;
int     port_slctd;
int     asl_rc = DIAG_ASL_OK;
int     cudv_ptr[128];
char    dm_c_s[] = "00";
char    portnum[3];             /* Adapter port number being tested          */
int     dm_idev;                /* Message number for device to test.        */
int     Adptr_name = 0x800;     /* Adapter function code number.             */
int     prt_num;                /* Adapter port number being tested          */
int     n_ports;                /* Number of ports on adapter                */
int     att_dev;                /* identifies attached device                */
int     att_cbl=MI_CABLE;       /* attached cable (default=Modem/Interposer) */
int     no_dev_att = FALSE;     /* was "No device attached" selected         */
int     port_tested = FALSE;
char    menu_msg[512];
struct  termio  SaveAttrs;      /* Save value of original Attributes */
struct  termio  WorkingAttrs;   /* Working copy of Attributes */
int     attrs_saved=0;          /* Was the save of the attributes successful */
cxma_t  OrigPacing;             /* User's pacing settings */
cxma_t  NewPacing;              /* Working copy of pacing settings */
int     pacing_saved=0;         /* Was save of the pacing info successful */
int     cxma_adapter=FALSE;     /* Testing cxma adapter without attached RAN */
int     cxma_line_err=FALSE;    /* Has there been a sync line error */
int     fru_found=FALSE;        /* Has a FRU been called out yet */
int     asl_init_done = 0;


/*      FRU variables   */

int     tu_err = 0;
int     insert_fru = FALSE;
int     last_tu = FALSE;
int     fru_set = FALSE;


/*      Confidence percentages  */

int     conf1 = 100;
int     conf2 = 90;
int     conf3 = 10;
int     conf4 = 85;
int     conf5 = 15;
int     conf6 = 95;
int     conf7 = 5;
int     conf8 = 70;
int     conf9 = 30;
