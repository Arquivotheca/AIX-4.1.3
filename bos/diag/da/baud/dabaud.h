/* @(#)71       1.1  src/bos/diag/da/baud/dabaud.h, foxclub, bos411, 9435B411a 8/26/94 13:30:51 */
/*
 *   COMPONENT_NAME: dabaud
 *
 *   FUNCTIONS: none
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

/*************************************************************************
 *     File: dabaud.h                                                    *
 *************************************************************************/

#include "dabaud_msg.h"
/* FFC is 0x716 for Foxclub card */
#define FOXCLUB 0x716

#define         NO_ERROR                0
#define         ERROR_FOUND             -1
#define         YES                     1
#define         NO                      0
#define         QUIT                    -1
#define         OTHER                   (0x9999)
#define         ASL_INITIALIZED         1
#define         ASL_NOT_INITIALIZED     -1

#define         READ_KBD                ASL_OK

/* Defines used for values that are often looked at */
#define         CONSOLE_INSTALLED       (tm_input.console == CONSOLE_TRUE)
#define         ELA_MODE                (tm_input.dmode == DMODE_ELA)
#define         PD_MODE                 (tm_input.dmode == DMODE_PD)
#define         REPAIR_MODE             (tm_input.dmode == DMODE_REPAIR)
#define         ADVANCED                ( tm_input.advanced == ADVANCED_TRUE )
#define         SYSTEM                  ( tm_input.system == SYSTEM_TRUE )
#define         IPL                             (tm_input.exenv == EXENV_IPL)
#define         CONCURRENT              (tm_input.exenv == EXENV_CONC)

/* defines to check to see if it is in a particular mode */
#define         NOTLM                   (tm_input.loopmode == LOOPMODE_NOTLM)
#define         INLM                    (tm_input.loopmode == LOOPMODE_INLM)
#define         EXITLM                  (tm_input.loopmode == LOOPMODE_EXITLM)
#define         ENTERLM                 (tm_input.loopmode == LOOPMODE_ENTERLM)

/* Declarations of functions in the C source */
void main();
static int disp_scrn(int);
static void set_up_sig();
static void ela ();
void intr_handler(int);
static void all_init();
static void check_asl_stat(int);
static void clean_up();
static int test_tu(int *);
static void report_frub(struct fru_bucket *);
static void ela(void);

/*
 *
 *
 * Below is where the device specific stuff starts
 *
 *
 */

/* TEST UNIT STRUCTURES */
#define TU_ORD_IPL tu_ipl
#define TU_ORD_ADV tu_adv
#define TU_OPEN_BAUD       tu_open
#define TU_CLOSE_BAUD      tu_close

int tu_adv[] = {
                                TU_VPD_CHECK,
                                TU_MCI_CHIP,
                                TU_CODEC_TEST,
                                TU_HTX_REC_PLAY,
                                (int) NULL};

int tu_ipl[] = {
                                TU_VPD_CHECK,
                           /*   TU_MCI_CHIP,
                                TU_HTX_REC_PLAY,   */
                                (int) NULL};

int tu_open[] = {TU_OPEN,(int) NULL};
int tu_close[] = {TU_CLOSE,(int) NULL};

#define ERRIDS errids
int errids[] = {0x999, (int) NULL};  /* used for ela */
                                     /* Temporary use 0x999 */

/*
 *  MENU defines
 */
#define CATALOG                                 MF_DABAUD
#define MSG_SET                                 BAUD_MSG
#define ADVANCED_MSG_ID                 ADVANCED_MODE_MENU
#define CUSTOMER_MSG_ID                 CUSTOMER_MODE_MENU
#define LOOPMODE_MSG_ID                 LOOPMODE_MODE_MENU

/*
 * * FRU structures **
 */
struct fru_bucket frus[] = {
    { "", FRUB1, FOXCLUB, 0x101, BAUD_101,  /* 0 - Open diagDD & Init device */
        {
            {80, "", "", 0,  DA_NAME, NONEXEMPT},
            {20, "Software", "", SOFTWARE,  NOT_IN_DB, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x102, BAUD_102,  /* 1 - VPD Registers test */
        {
            {100, "", "", 0,  DA_NAME, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x103, BAUD_103,  /* 2 - MCI Registers test */
        {
            {100, "", "", 0,  DA_NAME, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x104, BAUD_104,  /* 3 - Codec register test */
        {
            {100, "", "", 0,  DA_NAME, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x105, BAUD_105,  /* 4 - Record/playback test */
        {
            {100, "", "", 0,  DA_NAME, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x106, BAUD_106,  /* 5 - Close diagDD  */
        {
            {100, "Software", "", SOFTWARE,  NOT_IN_DB, NONEXEMPT},
        },
    },
    { "", FRUB1, FOXCLUB, 0x107, BAUD_107,  /* 6 - Error Log reported */
        {
            {80, "", "", 0,  DA_NAME, NONEXEMPT},
            {20, "Software", "", SOFTWARE,  NOT_IN_DB, NONEXEMPT},
        },
    },
};  /* end of FOXCLUB frus */


