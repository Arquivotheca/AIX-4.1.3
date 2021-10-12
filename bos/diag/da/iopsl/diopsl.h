/* @(#)59	1.2.1.2  src/bos/diag/da/iopsl/diopsl.h, daiopsl, bos411, 9428A410j 7/13/93 14:08:00 */
#ifndef _H_IOP
#define _H_IOP
/*
 * COMPONENT_NAME: daiopsl
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1991,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "diag/da.h"
#include "diag/atu.h"
#include "diag/dcda_msg.h"

/* Device Driver name to open */
#define DIOP_DD              "/dev/bus0"

/* TU names and return codes */
#define NVRAM_TEST           10
#define LED_TEST             20
#define TOD_TEST             30
#define KEY_LOCK_TEST        40
#define VPD_TEST             50
#define BATTERY_TEST         60
#define DEAD_BATTERY_TEST    70

/* FRU array defines */
#define NVRAM_TEST_FAILURE         0
#define LED_TEST_FAILURE           1
#define TOD_TEST_FAILURE           2
#define KEYLOCK_TEST_FAILURE       3
#define VPD_TEST_FAILURE           4
#define BATTERY_TEST_FAILURE       5

#define LOCKED_KEY_POSITION        1
#define SERVICE_KEY_POSITION       2
#define NORMAL_KEY_POSITION        3

#define INVALID_TU_CALL          256
#define INTERACTIVE_TEST_MODE      1
#define NO_VIDEO_TEST_MODE         2
#define TITLES_ONLY_TEST_MODE      3
#define INVALID_TM_INPUT          -1
#define A_LINE                    (80 * sizeof(" ") )


/* Defines used to specify whether the machine type has a keylock or not.*/
#define HAS_KEY_LOCK 1
#define NO_KEY_LOCK  0
#define KEY_LOCK     0x41
#define KEY_LESS     0x43

#ifndef NO_ERROR
#define NO_ERROR                   0
#endif

#ifndef TRUE
#define TRUE                       1
#define FALSE                      0
#endif

#ifndef AIX_ERROR
#define AIX_ERROR                 -1
#endif

#define ANSWERED_YES               1

#ifndef MIONVCHCK
#define MIONVCHCK                  15
#endif

/************************************************************************/
/*                                                                      */
/* fru_bucket structure holds related information to the FRU bucket.    */
/* Variables in this structure are explained in RIOS Diagnostic         */
/* Subsystem CAS under the function name addfrub().                     */
/*                                                                      */
/************************************************************************/

struct fru_bucket frub[] = {
/* 0 */ { "" , FRUB1 , 0x814, 0x112, DIOP_ERR_NVRAM,
                {
                    { 100, ""        , "" , 0, DA_NAME       , NONEXEMPT  },
                },
        },
/* 1 */ { "" , FRUB1, 0x816, 0x140, DIOP_ERR_OP,
                {
                   {  50, "Display"    , "" ,IOP_LED  , NOT_IN_DB, EXEMPT  },
                   {  45, "OP Panel"   , "" ,OP_PANEL , NOT_IN_DB, EXEMPT  },
                   {   5, ""           , "" , 0       , DA_NAME  , EXEMPT  },
                },
        },
/* 2 */ { "" , FRUB1 , 0x817, 0x123, DIOP_ERR_TOD,
                {
                    { 100, ""        , ""  , 0, DA_NAME       , NONEXEMPT },
                },
        },
/* 3 */ { "" , FRUB1, 0x816, 0x185,DIOP_ERR_KEY  ,
                {
                   { 65 ,"OP Panel", ""   , OP_PANEL, NOT_IN_DB , EXEMPT    },
                   { 30 ,"Keylock" , ""   , KEYLOCK , NOT_IN_DB , EXEMPT    },
                   {  5 , ""       , ""   , 0       , DA_NAME   , NONEXEMPT },
                },
         },
/* 4 */ { "" , FRUB1 , 0x814, 0x113, DIOP_ERR_VPD,
                {
                    { 100, ""        , "" , 0, DA_NAME       , NONEXEMPT  },
                },
        },
/* 5 */ { "" , FRUB1, 0x817, 0x212, DIOP_ERR_BAT,
                {
                     { 90, "Battery"  , "" , BATT   , NOT_IN_DB, EXEMPT  },
                     { 10, "OP Panel" , "" ,OP_PANEL, NOT_IN_DB, EXEMPT },
                },
        },
/* 6 */ { "" , FRUB1, 0x816, 0x285,DIOP_ERR_MODE_SWITCH  ,
                {
                   { 95 ,"OP Panel", ""   , OP_PANEL, NOT_IN_DB , EXEMPT    },
                   {  5 , ""       , ""   , 0       , DA_NAME   , NONEXEMPT },
                },
         },
};

#endif
