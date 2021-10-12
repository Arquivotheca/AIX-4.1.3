static char sccsid[] = "@(#)86	1.13.2.9  src/bos/diag/da/ped/dped.c, daped, bos411, 9428A410j 3/23/94 08:45:25";
/*
 * COMPONENT_NAME: DAPED  -  diagnostic application to test Gt4 and Gt4x.
 *
 * FUNCTIONS: tu_test, clean_up, test_config,
 *            stand_by_screen, loop_stand_by_screen, check_rc
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <locale.h>
#include <cf.h>

#include <sys/cfgodm.h>

#include "diag/da.h"
#include "diag/diago.h"
#include "diag/diag.h"
#include "diag/tm_input.h"
#include "diag/tmdefs.h"
#include "diag/diag_exit.h"
#include "diag/dcda_msg.h"
#include "ped_msg.h"
#include "tu_type.h"


/**************************************************************************/
/* IS_TM is a macro that accepts two input variables VAR1 & VAR2.         */
/* VAR1 is an object of the structure tm_input.                           */
/* VAR2 is a variable or a defined value that will be compared to the     */
/*      tm_input object class variable.                                   */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define IS_TM( VAR1, VAR2 ) ( (int) ( tm_input./**/VAR1 ) == /**/VAR2 )

/**************************************************************************/
/* INTERACTIVE is a macro that returns TRUE if conditions exist that      */
/*              allow the running of extended tests.  Extended tests are  */
/*              those tests which will run for several minutes.           */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define INTERACTIVE             (                                          \
                                      !(IS_TM( loopmode,LOOPMODE_INLM ))   \
                                &&      IS_TM( system,SYSTEM_FALSE )       \
                                &&      IS_TM( console,CONSOLE_TRUE )      \
                                )

/**************************************************************************/
/* ADVANCED is a macro that returns TRUE if the customer selected         */
/*              advanced mode of operation.                               */
/* NOTE: macro returns logical TRUE or FALSE                              */
/**************************************************************************/

#define ADVANCED                (                                          \
                                        IS_TM( advanced,ADVANCED_TRUE )    \
                                )



/* SPECIFIC TU SEQUENCES */

int ped_tu_seq[22] =
{
        2,3,4,5,6,7,8,9,10,20,21,12,13,14,22,24,15,16,17,18,19,23
};

int pipe_tu_seq[5] =
{
        71,72,73,74,75
};

int ped24_tu_seq[4] =
{
        50,51,52,53
};

int rgb_tu_seq[3] =
{
        31,32,33
};

int pretest_tu_seq[6] =
{
        2,3,8,9,12,14
};



/* Diagnostic microcode list */
char *diag_mcode[] =
{
        "8ee3p4d1.00",
        "8ee3p4d2.00",
        "8ee3p4d3.00",
        "8ee3p4d4.00",
        "8ee3p4d5.00"
};


/* fru_bucket is a structure that holds information for the diagnostic
   program to return to the diagnostic controller when a failure is found
   that needs to be reported. (FRU means Field Replacable Unit) */

struct fru_bucket frub[]=
{
/* [0] PROCESSOR CARD (ALL) */
        {"", FRUB1, 0x878, 0x100, R_PROC,
                {
                    {90, "", "", 0, DA_NAME, NONEXEMPT},
                    {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },

/* [1] */
        {"", FRUB1, 0x878, 0x110, R_PROC,
                {
                    {100, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [2] */
        {"", FRUB1, 0x878, 0x610, R_PROC,
                {
                    {90, "", "", 0, PARENT_NAME, NONEXEMPT},
                    {10, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },



/* [3] P4 2 PROCESSOR BASE CARD (GT4) */
        {"", FRUB1, 0x878, 0x800, R_PROC,
                {
                    {90, "", "", 0, DA_NAME, NONEXEMPT},
                    {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },

/* [4] */
        {"", FRUB1, 0x878, 0x811, R_PROC,
                {
                    {100, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [5] */
        {"", FRUB1, 0x878, 0x861, R_PROC,
                {
                    {90, "", "", 0, PARENT_NAME, NONEXEMPT},
                    {10, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },



/* [6] P4 6 PROCESSOR BASE CARD (GT4x) */
        {"", FRUB1, 0x878, 0x900, R_PROC,
                {
                    {90, "", "", 0, DA_NAME, NONEXEMPT},
                    {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },

/* [7] */
        {"", FRUB1, 0x878, 0x911, R_PROC,
                {
                    {100, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [8] */
        {"", FRUB1, 0x878, 0x961, R_PROC,
                {
                    {90, "", "", 0, PARENT_NAME, NONEXEMPT},
                    {10, "", "", 0, DA_NAME, NONEXEMPT},
                },
        },




/* [9] ORIGINAL 8-BIT GRAPHICS CARD */
        {"", FRUB1, 0x878, 0x300, R_GRAPHIC,
                {
                {80,"", "", 0, CHILD_NAME, NONEXEMPT},
                {20,"", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [10] */
        {"", FRUB1, 0x878, 0x310, R_GRAPHIC,
                {
                {100, "", "", 0, CHILD_NAME, EXEMPT},
                },
        },

/* [11] NEW INTEGRATED 8-BIT GRAPHICS CARD */
        {"", FRUB1, 0x878, 0x830, R_GRAPHIC,
                {
                {80,"", "", 0, CHILD_NAME, NONEXEMPT},
                {20,"", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [12] */
        {"", FRUB1, 0x878, 0x831, R_GRAPHIC,
                {
                {100, "", "", 0, CHILD_NAME, NONEXEMPT},
                },
        },

/* [13] ORIGINAL 24-BIT GRAPHICS OPTION CARD */
        {"", FRUB1, 0x878, 0x410, R_OPTION,
                {
                {90,"", "", 0, CHILD_NAME, NONEXEMPT},
                {10,"", "", 0, DA_NAME, NONEXEMPT},
                },
        },

/* [14] NEW INTEGRATED 24-BIT GRAPHICS CARD */
        {"", FRUB1, 0x878, 0x832, R_OPTION,
                {
                {90,"", "", 0, CHILD_NAME, NONEXEMPT},
                {10,"", "", 0, DA_NAME, NONEXEMPT},
                },
        },


/* [15] NEW INTEGRATED 24-BIT GRAPHICS CARD */
        {"", FRUB1, 0x878, 0x833, R_OPTION,
                {
                {100,"", "", 0, CHILD_NAME, NONEXEMPT},
                },
        },


/* [16] ORIGINAL PIPE OPTION (GT4X) CARD */
        {"", FRUB1, 0x878, 0x420, R_PIPE,
                {
                {90, "", "", 0, CHILD_NAME, NONEXEMPT},
                {10,"", "", 0, DA_NAME, NONEXEMPT},
                },
        },



/* [17] */
        {"", FRUB1, 0x878, 0x500, R_OPEN,
                {
                        {90, "", "", 0, DA_NAME, NONEXEMPT},
                        {10, "", "", 0, PARENT_NAME, NONEXEMPT},
                },
        },


/* [18] */
        {"", FRUB1, 0x878, 0x600, R_MON,
                {
                        {65, "", "", 0, DA_NAME, NONEXEMPT},
                        {35, "Monitor", "", F_MON, NOT_IN_DB, NONEXEMPT},
                },
        },

};




struct msglist da_title[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_STANDBY},
        (int)NULL
};

struct msglist l_da_title[] =
{
        {DPED_MSGS, DPED_L8B},
        /*{DPED_MSGS, DPED_L_STANDBY},*/
        (int)NULL
};

struct msglist havedisp[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_HAVEDISP},
        (int)NULL
};

struct msglist f3_exit[] =
{
	{DPED_MSGS, DPED_C8B},
	{DPED_MSGS, F3_EXIT},
	(int)NULL
};

struct msglist redscreen[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_REDCUR},
        (int)NULL
};

struct msglist greenscreen[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_GREENCUR},
        (int)NULL
};

struct msglist bluescreen[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_BLUECUR},
        (int)NULL
};

struct msglist selfred[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_REDCUR},
        (int)NULL
};

struct msglist selfgreen[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_GREENCUR},
        (int)NULL
};

struct msglist selfblue[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_BLUECUR},
        (int)NULL
};

struct msglist color_screen[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, COLOR_SCREEN},
        (int)NULL
};

struct msglist builtin[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_EXT_YES},
        {DPED_MSGS, DPED_EXT_NO},
        {DPED_MSGS, DPED_PDPQ},
        (int)NULL
};

struct msglist procedure[] =
{
        {DPED_MSGS, DPED_C8B},
        {DPED_MSGS, DPED_RP},
        (int)NULL
};



static ASL_SCR_TYPE menutypes = DM_TYPE_DEFAULTS;
static ASL_SCR_INFO uinfo[DIAG_NUM_ENTRIES(havedisp)];
static ASL_SCR_INFO m_uinfo[DIAG_NUM_ENTRIES(da_title)];
static ASL_SCR_INFO selfuinfo[DIAG_NUM_ENTRIES(selfred)];
static ASL_SCR_INFO final[DIAG_NUM_ENTRIES(builtin)];
static ASL_SCR_INFO procs[DIAG_NUM_ENTRIES(procedure)];
static ASL_SCR_INFO f3_uinfo[DIAG_NUM_ENTRIES(f3_exit)];


#define IS_CONSOLE  ( (int) (tm_input.console == CONSOLE_TRUE) )
/* ODM DEFINITIONS */
#define  OPTION_ID   "pop"   /* ORIG 24 BIT OPTION CARD */
#define  PROC_ID     "ppr"   /* ALL PROCESSOR CARDS */
#define  PGT_ID      "pgt"   /* NEW 24 BIT GRAPHICS */
#define  PGE_ID      "pge"   /* NEW 8 BIT GRAPHICS */
#define  PGR_ID      "pgr"   /* ORIGINAL 9 BIT BASE GRAPHICS */
#define  PIPE_ID     "ppc"   /* ORIGINAL PIPE OPTION */

/* SUBTYPE DEFINITIONS */
#define PED     "Other"
#define PED4_6  "Other6"
#define PED4_2  "Other2"

#define MAXCOLORS 512
#define CHAR_TO_INT(x) x[0]<<24 | x[1]<<16 | x[2]<<8 | x[3]
int     errno_rc;

PED4_TU_TYPE    tucb_ptr;       /* pointer to the previous structure */
struct tm_input tm_input;       /* info. from dc to this program     */
struct listinfo c_info;

int             rc;             /* to hold functions return codes    */
extern  nl_catd diag_catopen(char *, int);
nl_catd         catd;           /* file descriptor for catalog file  */
void            int_handler(int);
int             i;              /* loop counter */
int             ped2_flag = FALSE;     /* ORIGINAL BASE CARD */
int             pipe_flag = FALSE;     /* PIPE PRESENT ON BASE CARD */
int             ped4_2_flag = FALSE;   /* SINGLE PROC WITH 2 C30'S */
int             ped4_6_flag = FALSE;   /* SINGLE PROC WITH 6 C30'S */
int             option_flag = FALSE;   /* 24 BIT OPTION CARD       */
int		pgr_flag = FALSE;      /* ORIGINAL BASE GRAPHICS   */
int             pge_flag = FALSE;      /* 8-BIT, SINGLE GRAPHICS   */
int             pgt_flag = FALSE;      /* 24-BIT, SINGLE GRAPHICS  */
int             envflag;
int             diskette_based = DIAG_FALSE;
int             menunum = 0x878000;
int             l_menunum = 0x878010;
int             colornum = 0x878020;
int             colorselfnum = 0x878030;
int             tu_exit_flg = 0;
int             disp_attached = 0;
int             diag_asl_flg=FALSE;
int		fru_number;
char            card1loc[LOCSIZE];
char            card2loc[LOCSIZE];
char            card3loc[LOCSIZE];
char           *crit;           /* buffer */
char            DEVICE[50];     /* device name */
char           *dname;
char            no_rcm_msg[512];
char            no_diag_msg[512];

extern          exectu();
extern          getdainput();
extern          addfrub();
struct  sigaction       invec;  /* interrupt handler structure     */
void    ret_handler(int);
/*#define DEBUG 1*/
#if     DEBUG
FILE    *pedbug;
#endif

main()
{
    char            id[DEVIDSIZE];      /*device id search crit */
    int             tmp[10];

    setlocale(LC_ALL,"");
    crit = (char *) malloc(256);
    dname = (char *) malloc(256);

    (void)memset (&tucb_ptr, 0, sizeof(PED4_TU_TYPE));
    /* initialize interrupt handler */

    invec.sa_handler =  int_handler;
    sigaction( SIGINT, &invec, (struct sigaction *) NULL );
    sigaction( SIGTERM, &invec, (struct sigaction *) NULL );


#if     DEBUG
        pedbug=fopen("/tmp/ped4.debug","a+");
#endif
    /* Initialize return codes to the controller */
    DA_SETRC_STATUS(DA_STATUS_GOOD);
    DA_SETRC_ERROR(DA_ERROR_NONE);
    DA_SETRC_USER(DA_USER_NOKEY);
    DA_SETRC_TESTS(DA_TEST_FULL);
    DA_SETRC_MORE(DA_MORE_NOCONT);
    catd = (nl_catd) -1;

    if ((rc = init_dgodm()) != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }

    /* Call the external function to get the input environment. */
    if ((rc = getdainput(&tm_input)) != 0) {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        DA_EXIT();
    }


    for (i=0; i<18; ++i)
        strncpy(frub[i].dname,tm_input.dname,sizeof(frub[i].dname));

    FindWhichPed();
    FindDisplayType();

    strcpy(dname, tm_input.dname);

    if (tm_input.console == CONSOLE_TRUE)
    {
        diag_asl_init(ASL_INIT_DEFAULT);
        diag_asl_flg=TRUE;
        catd = diag_catopen(MF_PED,0);

        if (option_flag || pgt_flag) {      /* IF 24-BIT VERSION */
            menunum++;
            l_menunum++;
            da_title[0].msgid++;
            l_da_title[0].msgid++;
            havedisp[0].msgid++;
            f3_exit[0].msgid++;
            redscreen[0].msgid++;
            greenscreen[0].msgid++;
            bluescreen[0].msgid++;
            selfred[0].msgid++;
            selfgreen[0].msgid++;
            selfblue[0].msgid++;
            color_screen[0].msgid++;
            builtin[0].msgid++;
            procedure[0].msgid++;
        }

        if (pipe_flag || ped4_6_flag) {     /* IF 6 PROCESSOR VERSION */
            menunum += 2;
            l_menunum += 2;
            da_title[0].msgid += 2;
            l_da_title[0].msgid += 2;
            havedisp[0].msgid += 2;
            f3_exit[0].msgid += 2;
            redscreen[0].msgid += 2;
            greenscreen[0].msgid += 2;
            bluescreen[0].msgid += 2;
            selfred[0].msgid += 2;
            selfgreen[0].msgid += 2;
            selfblue[0].msgid += 2;
            color_screen[0].msgid += 2;
            builtin[0].msgid += 2;
            procedure[0].msgid += 2;
        }
    }

    if (tm_input.loopmode == LOOPMODE_NOTLM)
    {
        stand_by_screen();
        check_microcode();
    }
    else
        loop_stand_by_screen();


    /* TU initialization */
    tucb_ptr.loop = 1;

    /* Check for pre-test */
    if (IS_TM (exenv,EXENV_IPL)) {
        for (i=0; i<6; ++i)
            tu_test(pretest_tu_seq[i]);
        clean_up();
    }

    invec.sa_handler =  ret_handler;
    sigaction( SIGRETRACT, &invec, (struct sigaction *) NULL );

    if (tm_input.system == SYSTEM_TRUE) {
        if (tm_input.loopmode == LOOPMODE_ENTERLM) {
            check_microcode();
        }
        sys_tu_seq();
    }
    else if (tm_input.loopmode == LOOPMODE_NOTLM) {
            sys_tu_seq();
            if (IS_CONSOLE)
                interact_tu_seq();
    }
    else { /* must be loop mode */
         if (tm_input.loopmode == LOOPMODE_ENTERLM) {
             check_microcode();
             sys_tu_seq();
             if (IS_CONSOLE)
                 interact_tu_seq();
         }
         else if (tm_input.loopmode == LOOPMODE_INLM)
             sys_tu_seq();
    };

    clean_up();


} /* end main */




/************************************************************************
* NAME: FindWhichPed
*
* FUNCTION: Determines which Gt4 cards are present (original or integrated).
*           Get the physical display id of the hardware to test.
*
* EXECUTION ENVIRONMENT:
*
*       Called by the main procedure.
*
* NOTES:
*       Set flags to identify which adapter is under test
*       Set the physical device id.
*
* RETURNS: NONE
*/

FindWhichPed()
{
struct CuDv *cudv;
struct CuAt *cuat;
struct listinfo  c_info;
char   crit[100];
char   id[PREFIXSIZE];
int    i;
int    how_many;

    sprintf(crit,"parent=%s and connwhere=%c and chgstatus != 3",
            tm_input.parent, tm_input.dnameloc[4]);
    cudv = get_CuDv_list(CuDv_CLASS,crit,&c_info,2,1);
    if ((cuat = (struct CuAt *) getattr(cudv->name, "subtype", FALSE,
                                        &how_many)) == (struct CuAt *) NULL)
        {
                DA_SETRC_ERROR(DA_ERROR_OTHER);
                clean_up();
        }

    if (strcmp(cuat->value,"Other")==0)
        ped2_flag = TRUE;
    else if (strcmp(cuat->value,"Other2")==0)
        ped4_2_flag = TRUE;
    else if (strcmp(cuat->value,"Other6")==0)
        ped4_6_flag = TRUE;
}




/************************************************************************
* NAME: FindDisplayType
*
* FUNCTION: Determines what type of adapter, 8-bit or 24-bit, are being asked
*           to test on.
*           Get the physical display id of the hardware to test.
*
* EXECUTION ENVIRONMENT:
*
*       Called by the main procedure.
*
* NOTES:
*       Set presence flags for the pipe card and option cards.
*       Set the physical device id.
*
* RETURNS: NONE
*/

FindDisplayType()
{
struct CuDv *cudv;
struct CuDv *cudva;
struct CuAt *cuat;
struct listinfo  c_info;
char   crit[100];
char   id[PREFIXSIZE];
int    i;
int    how_many;

/* Search for and save off information for the processor card */

    sprintf(crit, "name = %s and chgstatus != 3", tm_input.dname);
    cudva = get_CuDv_list(CuDv_CLASS, crit, &c_info, 1, 1);
    if ((cudva == (struct CuDv *) -1) || (cudva == (struct CuDv *) NULL))
    {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        clean_up();
    }

        (void)strncpy(frub[0].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[0].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[1].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[1].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[2].frus[1].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[2].frus[1].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[3].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[3].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[4].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[4].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[5].frus[1].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[5].frus[1].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[6].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[6].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[7].frus[0].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[7].frus[0].floc,cudva->location,LOCSIZE);
        (void)strncpy(frub[8].frus[1].fname,cudva->name,NAMESIZE);
        (void)strncpy(frub[8].frus[1].floc,cudva->location,LOCSIZE);

/* Also, fill in info for frub[18] - monitor fru */
    (void)strncpy(frub[18].frus[0].fname,cudva->name,NAMESIZE);
    (void)strncpy(frub[18].frus[0].floc,cudva->location,LOCSIZE);

    /* Search for and save off information into appropriate fru buckets for
       PGE, PGT, PGR, POP and PIP cards */

    sprintf(crit, "parent = %s and chgstatus != 3", tm_input.dname);
    cudv = get_CuDv_list(CuDv_CLASS, crit, &c_info, 1, 2);
    if ((cudv == (struct CuDv *) -1) || (cudv == (struct CuDv *) NULL))
    {
        DA_SETRC_ERROR(DA_ERROR_OTHER);
        clean_up();
    }

    for (i=0; i<c_info.num; ++i)
    {
        /*strncpy( id, cudv[i].PdDvLn->devid, DEVIDSIZE);*/
        strncpy( id, cudv[i].PdDvLn->prefix, PREFIXSIZE);

        if ( (strcmp(PGR_ID,id)) == 0)
        {
            (void)strncpy(frub[9].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[9].frus[0].floc,cudv[i].location,LOCSIZE);
            (void)strncpy(frub[10].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[10].frus[0].floc,cudv[i].location,LOCSIZE);
	    pgr_flag = TRUE;
        }


        if ( (strcmp(OPTION_ID,id)) == 0)
        {
            (void)strncpy(frub[13].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[13].frus[0].floc,cudv[i].location,LOCSIZE);
            option_flag = TRUE;
        }

        if ( (strcmp(PIPE_ID,id)) == 0)
        {
            (void)strncpy(frub[16].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[16].frus[0].floc,cudv[i].location,LOCSIZE);
            pipe_flag = TRUE;
        }

        if ( (strcmp(PGE_ID,id)) == 0)
        {
            (void)strncpy(frub[11].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[11].frus[0].floc,cudv[i].location,LOCSIZE);
            (void)strncpy(frub[12].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[12].frus[0].floc,cudv[i].location,LOCSIZE);
            pge_flag = TRUE;
        }

        if ( (strcmp(PGT_ID,id)) == 0)
        {
            (void)strncpy(frub[14].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[14].frus[0].floc,cudv[i].location,LOCSIZE);
            (void)strncpy(frub[15].frus[0].fname,cudv[i].name,NAMESIZE);
            (void)strncpy(frub[15].frus[0].floc,cudv[i].location,LOCSIZE);
            pgt_flag = TRUE;
        }
    }


} /* end FindDisplayType */





/****************
*
*  sys_tu_seq
*
*****************/

int
sys_tu_seq()
{
 int i;
    for (i=0; i<22; ++i)
            tu_test(ped_tu_seq[i]);
    if (pipe_flag || ped4_6_flag)
        for (i=0; i<5; ++i)
            tu_test(pipe_tu_seq[i]);
    if (option_flag || pgt_flag)
        for (i=0; i<4; ++i)
            tu_test(ped24_tu_seq[i]);
}
/* end sys_tu_seq */




/*****************
*
*  interact_tu_seq
*
******************/

int
interact_tu_seq()
{
    int color;

    /* run TU_CLOSE so that we can display a message */
    if (tu_exit_flg == 1)
    {
	tu_exit();
    }

    /* Ask if there is a monitor attached to the adapter */
    insert_msg(&havedisp,uinfo,&menutypes,ASL_DIAG_LIST_CANCEL_EXIT_SC);
    menutypes.cur_index = 1;
    rc = diag_display(colornum, catd, NULL, DIAG_IO,
                      ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);
    check_rc(rc);
    if (rc == DIAG_ASL_COMMIT)
	switch (DIAG_ITEM_SELECTED(menutypes)) {
	    case 1 : disp_attached = 1;
		     break;
  	    case 2 : 
	    default: clean_up();
	}

    /* Run RGB if display is attached */
    if (disp_attached) {
	    for (i=0; i<3; ++i) {
		  /* run TU_CLOSE so that we can display a message */
    		  if (tu_exit_flg == 1)
		  {
			tu_exit();
    		  }

		  /* get the color that is going to be displayed */
	        switch (i) 
		  {
			case 0 : /* RED */
				color = RED;
			 	break;
			case 1 : /* GREEN */
				color = GREEN;
			 	break;
			case 2 : /* BLUE */
				color = BLUE;
				break;
	        	default: 
				break;
	        }

	        /* Put up screen telling that a color will be displayed */
              insert_color(&f3_exit,f3_uinfo,&menutypes,
						ASL_DIAG_KEYS_ENTER_SC,color);
              rc = diag_display(0x878040, catd, NULL, DIAG_IO,
                               ASL_DIAG_KEYS_ENTER_SC, &menutypes,f3_uinfo); 
	        check_rc(rc);

	        tu_test(rgb_tu_seq[i]);

		  /*  wait 6 seconds */
		  sleep(7);


		  /* run TU_CLOSE so that we can display a message */
    		  if (tu_exit_flg == 1)
		  {
			tu_exit();
    		  }

	        /* Put up screen asking if the screen was the right color */
              insert_color(&color_screen,uinfo,&menutypes,
						ASL_DIAG_LIST_CANCEL_EXIT_SC,color);

	        colornum += 1;
              rc = diag_display(colornum, catd, NULL, DIAG_IO,
                             ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, uinfo);
	        check_rc(rc);
	        if (menutypes.cur_index != 1) {
	    	    /* report fru */
                    insert_frub(&tm_input, &frub[18]);
                    addfrub(&frub[18]);
        	    DA_SETRC_STATUS(DA_STATUS_BAD);
		    clean_up();
                };

        } 
        /* Ask user to run stand alone monitor test */
        insert_msg(&builtin,final,&menutypes,ASL_DIAG_LIST_CANCEL_EXIT_SC);
        menutypes.cur_index = 1;
        rc = diag_display(0x878123, catd, NULL, DIAG_IO,
                          ASL_DIAG_LIST_CANCEL_EXIT_SC, &menutypes, final);
        check_rc(rc);
        if (rc == DIAG_ASL_COMMIT)
                switch (DIAG_ITEM_SELECTED(menutypes)) {
                case 1 : insert_msg(&procedure,procs,&menutypes,
                                    ASL_DIAG_KEYS_ENTER_SC);
                         rc = diag_display(0x878234, catd, NULL, DIAG_IO,
                              ASL_DIAG_KEYS_ENTER_SC, &menutypes, procs);
                         check_rc(rc);
                         break;
                case 2 :
                default: clean_up();
                }
    } /* end display attached */
    return(0);
}





/*
* NAME: tu_test
*
* FUNCTION: Executing test units and report fru(s) to the controller if a
*           failure is found.
*
* EXECUTION ENVIRONMENT:
*
*       Called by the main program to execute test units.
*       Call external routine exectu to actually execute the test units.
*       Call external routine diag_asl_read to get user's input to screen.
*       Call check_rc to check if user has entered the Esc or Cancel key.
*       Call external routines insert_frub and addfrub when a failure is found.
*       Call clean_up after a fru is reported to the controller.
*
* RETURNS: NONE
*/

tu_test(testptr)
    int             testptr;            /* determines which test unit to run */
{
    int         turc;                   /* return code from test unit */
    int         how_many;
    struct      CuDv *cudva;
    struct      CuAt *cuat_rrate;
    char        buff[256];


    if (tu_exit_flg == 0) {
	tucb_ptr.loop = 1;
        tucb_ptr.tu = TU_OPEN; /* new value is 90 */
        turc = exectu(tm_input.dname, &tucb_ptr);
        tu_exit_flg = 1;
#if DEBUG
    fprintf(pedbug,"after entering monitor mode \n");
    fflush(pedbug);
#endif

        sprintf(buff,"parent=%s and connwhere=%c and chgstatus != 3",
        tm_input.parent, tm_input.dnameloc[4]);
        cudva = get_CuDv_list(CuDv_CLASS,buff,&c_info,2,1);
        cuat_rrate = (struct CuAt *)getattr(cudva->name, "refresh_rate", FALSE,
                                   &how_many);
        if (strcmp (cuat_rrate->value,"77") == 0) {
	    tucb_ptr.loop = 1;
            tucb_ptr.tu = 77;
            turc = exectu(tm_input.dname, &tucb_ptr);
        }
        else { /* 60 Hz */
	    tucb_ptr.loop = 1;
            tucb_ptr.tu = 60;
            turc = exectu(tm_input.dname, &tucb_ptr);
        };
    }

    tucb_ptr.loop = 1;
    tucb_ptr.tu = testptr;

    turc = exectu(tm_input.dname, &tucb_ptr);

#if DEBUG
    fprintf(pedbug,"after executing tu number %d\n", testptr);
    fflush(pedbug);
    fprintf(pedbug,"tu return code is  %d\n", turc);
    fflush(pedbug);
#endif


    if (IS_CONSOLE) {
        rc = diag_asl_read(ASL_DIAG_KEYS_ENTER_SC, FALSE, NULL);
        check_rc(rc);
    }


    if (turc != 0) /* Check for common return code */
        check_common(turc);

} /* end tu_test */




/*
* NAME: clean_up
*
* FUNCTION: Closing file descriptors and return to diagnostic controller.
*
* EXECUTION ENVIRONMENT:
*       Called by main program and tu_test.
*
* RETURNS: NONE
*/

clean_up()
{
    if (tu_exit_flg == 1)
         tu_exit();
#if DEBUG
    fprintf(pedbug,"after leaving tu_exit\n");
    fflush(pedbug);
#endif
    free(crit);
    free(dname);
    if (tm_input.console == CONSOLE_TRUE && diag_asl_flg)
        diag_asl_quit(NULL);    /* close ASL */
    if( catd != ((nl_catd) -1))
        catclose(catd);
    term_dgodm();       /* close ODM */
#if DEBUG
    fflush(pedbug);
    fclose(pedbug);
#endif
    DA_EXIT();
} /* end clean_up */




/*
* NAME: stand_by_screen
*
* FUNCTION: Displaying screen to user indicating test units are being executed.
*
* EXECUTION ENVIRONMENT:
*       Called by main program.
*       Call check_rc.
*
* RETURNS: NONE
*/

int
stand_by_screen()
{
    if (tm_input.console == CONSOLE_TRUE) {
        if (tm_input.advanced == ADVANCED_TRUE) {
            da_title[0].msgid += 4;
            menunum += 4;
        }
        insert_msg(&da_title,m_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
        rc = diag_display(menunum, catd, NULL, DIAG_IO,
                          ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, m_uinfo);
        sleep(2);
        check_rc(rc);
    }
} /* end stand_by_screen */




/*
* NAME: loop_stand_by_screen
*
* FUNCTION: Displaying screen to user indicating test units are being executed
*           in loop mode.
*
* EXECUTION ENVIRONMENT:
*       Called by main program.
*       Call check_rc.
*
* RETURNS: NONE
*/

int
loop_stand_by_screen()
{
    if (tm_input.loopmode == LOOPMODE_ENTERLM)
        stand_by_screen();
    else if (IS_CONSOLE)
    {
        insert_msg(&l_da_title,m_uinfo,&menutypes,ASL_DIAG_OUTPUT_LEAVE_SC);
        rc = diag_display(menunum, catd, NULL, DIAG_IO,
                          ASL_DIAG_OUTPUT_LEAVE_SC, &menutypes, m_uinfo);
        sleep(2);
        check_rc(rc);
    }
} /* end loop_stand_by_screen */





/*
* NAME: check_rc
*
* FUNCTION: Checks if the user has entered the Esc or Cancel key while a screen
*           is displayed.
*
* EXECUTION ENVIRONMENT:
*       Called by main program and some other routines.
*
* RETURNS: rc, the input parameter
*/

int
check_rc(rc)
    int             rc;                         /* user's input */
{
    if (rc == DIAG_ASL_CANCEL) {
        tm_input.loopmode = LOOPMODE_EXITLM;    /* force microcode swap */
        DA_SETRC_USER(DA_USER_QUIT);
        clean_up();
    }
    if (rc == DIAG_ASL_EXIT) {
        tm_input.loopmode = LOOPMODE_EXITLM;    /* force microcode swap */
        DA_SETRC_USER(DA_USER_EXIT);
        clean_up();
    }
    return (rc);
} /* end check_rc */




/*
* NAME: check_microcode
*
* FUNCTION: Checks if the functional and diagnostic microcodes are present
*
* EXECUTION ENVIRONMENT:
*       Called by main program and some other routines.
*
* RETURNS: NONE
*/

int
check_microcode()
{
    int		i;
    char        mpath[255];

    /* Check if the functional microcode file xxxx.xxx is present.
       Check only if diagnostics is run off hard disk */
    envflag = ipl_mode(&diskette_based);
    if (diskette_based == DIAG_FALSE) {
        if (pipe_flag || ped4_6_flag) {
            if (0 == (rc = findmcode("8ee3.01", mpath, VERSIONING, NULL))) {
                sprintf(no_rcm_msg,catgets(catd,NO_RCM,NO_RCM_TITLE,NULL));
                menugoal(no_rcm_msg);
		clean_up();
            }
        }
        else {
            if (0 == (rc = findmcode("8ee3n.01", mpath, VERSIONING, NULL))) {
                sprintf(no_rcm_msg,catgets(catd,NO_RCM,NO_RCM_TITLE,NULL));
                menugoal(no_rcm_msg);
		clean_up();
            }
        };
    }

    /* Check if all the diagnostic microcode files are present. */
    for (i=0; i<5; ++i )
    {
        rc = findmcode(diag_mcode[i], mpath, VERSIONING, NULL);
        if (0 == rc) {
            sprintf(no_diag_msg,catgets(catd,NO_DIAG,NO_DIAG_TITLE,NULL));
            menugoal(no_diag_msg);
            clean_up();
        }
    }
}





insert_msg(msg_list,uinfo,menutypes,type)
struct  msglist msg_list[];
ASL_SCR_INFO    uinfo[];
ASL_SCR_TYPE    *menutypes;
long    type;
{
        char    string[1024];
        menutypes->max_index = 0;
        menutypes->cur_index = 1;
        menutypes->cur_win_offset = 0;
        menutypes->cur_win_index = 0;
        menutypes->multi_select = 'y';
        menutypes->text_size = 0;
        menutypes->ask = '\0';

        diag_display(0x0,catd, msg_list,
                     DIAG_MSGONLY,type, menutypes,uinfo);
        if (tm_input.loopmode != LOOPMODE_NOTLM)
            sprintf(string,uinfo[0].text,tm_input.dname,tm_input.dnameloc,
                      tm_input.lcount, tm_input.lerrors);
         else
            sprintf(string,uinfo[0].text,tm_input.dname,tm_input.dnameloc);
                      uinfo[0].text = (char *) malloc(strlen(string) +1);
         strcpy(uinfo[0].text, string);
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

void
int_handler(int sig)
{
        invec.sa_handler =  int_handler;
        sigaction( sig, &invec, (struct sigaction *) NULL );

        if ( IS_TM( console, CONSOLE_TRUE ) )
           diag_asl_clear_screen();
        clean_up();
}




void
ret_handler(int sig)
{
        sleep(1); /* wait for the tu to finish then clean_up */
        clean_up();
}
/*
 * NAME: chk_return()
 *
 * FUNCTION: Designed to check for an invalid return code from a function.
 *           Invalid return code is -1.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
chk_return(ret_code)
int     ret_code;
{
        if ( ret_code == -1 )
        {
                DA_SETRC_ERROR ( DA_ERROR_OTHER );
                clean_up();
        }
}



/*
 * NAME: tu_exit
 *
 * FUNCTION: leave monitor mode
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
tu_exit()
{
int   turco;
   tucb_ptr.tu = TU_CLOSE; /* new value is 91 */
   tucb_ptr.loop = 1;
   turco = exectu(tm_input.dname, &tucb_ptr);
   tu_exit_flg = 0;
}



/*
 * NAME: check_common
 *
 * FUNCTION: check for common error return code
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Environment must be a diagnostics environment which is described
 *      in Diagnostics subsystem CAS.
 *
 * RETURNS: NONE
 */

int
check_common(c_code)
int c_code;
{
    /* SOFTWARE ERROR */
    if (((c_code>=0x10)&&(c_code<=0x16)) ||
         ((c_code>=0x3a)&&(c_code<=0x46)) ||
         (c_code==0x99)||(c_code==0x200)) /* rc 90B is taken out */
    {
           DA_SETRC_ERROR( DA_ERROR_OTHER );
           clean_up();
    }


    /* MOST LIKELY PROCESSOR CARD , POSSIBLY SYSTEM */
    if (((c_code>=0x17)&&(c_code<=0x39)) ||
        ((c_code>=0x201)&&(c_code<=0x4ff)) ||
        ((c_code>=0x800)&&(c_code<=0x8ff)) ||
        ((c_code>=0x900)&&(c_code<=0x90a)) ||
        ((c_code>=0xa00)&&(c_code<=0xaff)))
        {
        if (ped2_flag)
            {
            insert_frub(&tm_input, &frub[0]);
            frub[0].sn=0x878;
            addfrub(&frub[0]);
            };
        if (ped4_6_flag)
            {
            insert_frub(&tm_input, &frub[6]);
            frub[6].sn=0x878;
            addfrub(&frub[6]);
            };
        if (ped4_2_flag)
            {
            insert_frub(&tm_input, &frub[3]);
            frub[3].sn=0x878;
            addfrub(&frub[3]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
        };


    /* MOST LIKELY PIPE OR PED4    */
    if  ((c_code>=0x100)&&(c_code<=0x1ff))
        {
        if (pipe_flag)
            {
            insert_frub(&tm_input, &frub[15]);
            frub[15].sn=0x878;
            addfrub(&frub[15]);
            };
        if (ped4_6_flag)
            {
            insert_frub(&tm_input, &frub[7]);
            frub[7].sn=0x878;
            addfrub(&frub[7]);
            };
        if (ped4_2_flag)
            {
            insert_frub(&tm_input, &frub[4]);
            frub[4].sn=0x878;
            addfrub(&frub[4]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
    }


    /* MOST LIKELY BASE GRAPHICS CARD (8-BIT) OR EITHER 8 OR 24 BIT NEW */
    if (((c_code>=0xc00)&&(c_code<=0xdff)) ||
        ((c_code>=0x1500)&&(c_code<=0x15ff)))
        {
        if (pgr_flag)
            {
            insert_frub(&tm_input, &frub[9]);
            frub[9].sn=0x878;
            addfrub(&frub[9]);
            };
        if (pge_flag)
            {
            insert_frub(&tm_input, &frub[11]);
            frub[11].sn=0x878;
            addfrub(&frub[11]);
            };
        if (pgt_flag)
            {
            insert_frub(&tm_input, &frub[14]);
            frub[14].sn=0x878;
            addfrub(&frub[14]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
        };


    /* BASE GRAPHICS CARD (8-BIT) OR EITHER 8 OR 24 BIT NEW */
    if (((c_code>=0xe00)&&(c_code<=0x14ff)) ||
        ((c_code>=0x2000)&&(c_code<=0x21ff)) ||
        ((c_code>=0x2400)&&(c_code<=0x24ff)))
        {
        if (pgr_flag)
            {
            insert_frub(&tm_input, &frub[10]);
            frub[10].sn=0x878;
            addfrub(&frub[10]);
            };
        if (pge_flag)
            {
            insert_frub(&tm_input, &frub[12]);
            frub[12].sn=0x878;
            addfrub(&frub[12]);
            };
        if (pgt_flag)
            {
            insert_frub(&tm_input, &frub[15]);
            frub[15].sn=0x878;
            addfrub(&frub[15]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
        };


    /* MOST LIKELY 24-BIT INTEGRATED CARD OR 24-BIT OPTION CARD */
    if ((c_code>=0x3200)   &&(c_code<=0x35ff))
        {
        if (option_flag)
            {
            insert_frub(&tm_input, &frub[13]);
            frub[13].sn=0x878;
            addfrub(&frub[13]);
            };
        if (pgt_flag)
            {
            insert_frub(&tm_input, &frub[14]);
            frub[14].sn=0x878;
            addfrub(&frub[14]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
        };


    /* SYSTEM BOARD MOST LIKELY , POSSIBLY PROCESSOR CARD */
    if (c_code == 0x90b)
    {
        if (ped2_flag)
            {
            insert_frub(&tm_input, &frub[2]);
            frub[2].sn=0x878;
            addfrub(&frub[2]);
            };
        if (ped4_6_flag)
            {
            insert_frub(&tm_input, &frub[8]);
            frub[8].sn=0x878;
            addfrub(&frub[8]);
            };
        if (ped4_2_flag)
            {
            insert_frub(&tm_input, &frub[5]);
            frub[5].sn=0x878;
            addfrub(&frub[5]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
    }


    /* ORIGINAL PROCESSOR WITH PIPE ONLY */
    if ((c_code>=0x100)&&(c_code<=0x1ff)&&(pipe_flag))
            {
            insert_frub(&tm_input, &frub[16]);
            frub[16].sn=0x878;
            addfrub(&frub[16]);
            };



    /* PROCESSOR BOARD ONLY */
    if (((c_code>=0x50)&&(c_code<=0x74))   ||
        ((c_code>=0x100)&&(c_code<=0x1ff)&&(ped4_6_flag)) ||
        ((c_code>=0x500)&&(c_code<=0x5ff)) ||
        ((c_code>=0x600)&&(c_code<=0x6ff)) ||
        ((c_code>=0x700)&&(c_code<=0x7ff)) ||
        ((c_code>=0xb00)&&(c_code<=0xbff)) ||
        ((c_code>=0x1e00)&&(c_code<=0x1fff)))
    {
        if (ped2_flag)
            {
            insert_frub(&tm_input, &frub[1]);
            frub[1].sn=0x878;
            addfrub(&frub[1]);
            };
        if (ped4_6_flag)
            {
            insert_frub(&tm_input, &frub[7]);
            frub[7].sn=0x878;
            addfrub(&frub[7]);
            };
        if (ped4_2_flag)
            {
            insert_frub(&tm_input, &frub[4]);
            frub[4].sn=0x878;
            addfrub(&frub[4]);
            };
        DA_SETRC_STATUS(DA_STATUS_BAD);
        clean_up();
    }

    /* For undefined return code - return software error */

    DA_SETRC_ERROR( DA_ERROR_OTHER );
    clean_up();
}



/*******************************************************************************
*
* NAME: insert_color
*                                                                    
* FUNCTION: initialize menutypes structure and call diag_display routine to
*           display menus
*                                                                    
* EXECUTION ENVIRONMENT:
*	Called by main program and some other routines.
*                                                                   
* RETURNS: NONE
*
********************************************************************************/

insert_color(msg_list,t_uinfo,menutypes,type,color)
struct  msglist msg_list[];
ASL_SCR_INFO    t_uinfo[];
ASL_SCR_TYPE    *menutypes;
long    type;
int color;
{
	char    string[1024];
	static int first = TRUE;

	diag_display(0x0,catd, msg_list, DIAG_MSGONLY,type, menutypes,t_uinfo);
	if(first == TRUE)
	{
		first = FALSE;
		sprintf(string,t_uinfo[1].text, diag_cat_gets(catd,DPED_MSGS,color));
		t_uinfo[1].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[1].text, string);

	}
	else
	{
		first = TRUE;
		menutypes->cur_index = 1;
		menutypes->multi_select = 'y';
		sprintf(string,t_uinfo[3].text, diag_cat_gets(catd,DPED_MSGS,color));
		t_uinfo[3].text = (char *) malloc(strlen(string) +1);
		strcpy(t_uinfo[3].text, string); 

	}

	sprintf(string,t_uinfo[0].text,tm_input.dname,tm_input.dnameloc);
	t_uinfo[0].text = (char *) malloc(strlen(string) +1);
	strcpy(t_uinfo[0].text, string); 
}

