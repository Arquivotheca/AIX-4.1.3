/* @(#)98       1.6  src/bos/usr/bin/panel20/panel20.h, cmdhia, bos411, 9428A410j 7/21/92 10:26:03 */
/*
 *
 * COMPONENT_NAME: (CMDHIA) Panel20 include file
 *
 * FUNCTIONS: Common Panel20 includes, defines and declarations
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* BAW remove NUM_PAGE define and add num_page variable */
/*  add chan_addr and adapter_arg variables             */
#include <stdio.h>
#include <cur00.h>
#include <cur02.h>
#include <sys/signal.h>

#define NUM_LNKS        16              /* the number of possible links
                                           for both graphics and 3270 */
#define EVAR_GRPH       "GRAPHDEV"      /* the name of the environment var
                                           that is used for graphic config */
#define GRAPH_ONE       1               /* don't want to support grph address
                                           on panel 20 - 01 */
#define SCR_ONE         1               /* number for panel 20-01 */
#define SCR_TWO         2               /* number for panel 20-02 */
#define SCR_THR         3               /* number for panel 20-03 */
#define ENT_PAGE        8               /* entries per page   */
/* definition of some commands for panel20 program */
#define N_PANEL         '\n'            /* next panel, toggle screens */
#define QUIT            KEY_F(10)       /* exit panel20 */
#define N_PAGE          KEY_DOWN        /* next page in panel 20 01*/
#define P_PAGE          KEY_UP          /* previous page in panel 20 01 */

struct panel20_data {
        char graph_3270_switch;         /* 00 = output information for
                                                both graphics and 3270
                                           01 = output for graphics only
                                           02 = output information for
                                                3270 only
                                        */
        char link_speed;                /* 01 = 1 megabit, 02 = 2 megabit */
        char u_code_version[4];         /* version of the microcode */
        unsigned short int lnk_errors;  /* number of link errors reported
                                           as statistical errors */
        unsigned short brd_cst_cnt;     /* broadcast count */
        char brd_frame[17];             /* broadcast frame */
        char enb_3270[16];              /* 1 = enable, 0 = disabled */
        char enb_grph[16];              /* 1 = enable, 0 = disabled */
        struct p20_cntrs {
                unsigned int poll_cnt;  /* poll count for a link address or
                                           -1 */
                unsigned int snrm_cnt;  /* SNRM count for a link address or
                                           -1 */
        } m3270[16],grph[16];
        unsigned long  le_count[16];    /* link error counters */
};

#ifdef DEFINE_SYMS
        long    panel20_fd;             /* file descriptor for panel20 */
        struct panel20_data panel20;    /* actual panel20 data */
        char cur_scrn = SCR_ONE;        /* current screen */
        char config_3270[NUM_LNKS];     /* 1 if configured */
        char config_grph[NUM_LNKS];     /* 1 if configured */
        char lnk_stat_3270[NUM_LNKS];   /* lnk stat buffer for 3270 */
        char lnk_stat_grph[NUM_LNKS];   /* lnk stat buffer for grph */
        char last_scrn = SCR_ONE;       /* last screen used */
        char cur_page = 1;              /* current page of panel 1 */
        char num_page = 1;              /* number of SCR_ONE pages */
        char *evar_grph = EVAR_GRPH;    /* env var name for grph conf */
        char HIA_FILE[80];              /* special file that panel20 reads to
                                                           get link data */
        char do_again = 0;
        int chan_addr = 0;              /* 5080_chan_addr device attribute */
        char adapter_arg[10];           /* argv[1] or hia0 */
#else
        extern long     panel20_fd;             /*file descriptor for panel20*/
        extern struct panel20_data panel20;     /* actual panel20 data */
        extern char cur_scrn;                   /* current screen */
        extern char config_3270[NUM_LNKS];      /* 1 if configured */
        extern char config_grph[NUM_LNKS];      /* 1 if configured */
        extern char lnk_stat_3270[NUM_LNKS];    /* lnk stat buffer for 3270 */
        extern char lnk_stat_grph[NUM_LNKS];    /* lnk stat buffer for grph */
        extern char last_scrn;                  /* last screen used */
        extern char cur_page;                   /* current page of panel 1 */
        extern char num_page;                   /* number of SCR_ONE pages */
        extern char *evar_grph;                 /* env var name for grph conf*/
        extern char HIA_FILE[80];
        extern char do_again;
        extern int chan_addr;                   /* 5080_chan_addr attribute */
        extern char adapter_arg[10];            /* argv[1] or hia0 */
#endif
