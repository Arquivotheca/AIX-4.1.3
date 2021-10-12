/* @(#)79	1.5.1.8  src/bos/diag/da/iop/iop_frus.h, daiop, bos41J, 9513A_all 3/9/95 09:25:27 */
/*
 *   COMPONENT_NAME: DAIOP
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 27, 83
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   LEVEL 1, 5 Years Bull Confidential Information
 */

#include "iop.h"
#include "diag/da.h"
#include "diop_msg.h"


/************************************************************************/
/*                                                                      */
/* fru_bucket structure holds related information to the FRU bucket.    */
/* Variables in this structure are explained in RIOS Diagnostic         */
/* Subsystem CAS under the function name addfrub().                     */
/*                                                                      */
/************************************************************************/


struct fru_bucket frub[] = {
/* 0 */ { "" , FRUB1 , 0 , 0, 0,
                {
                    { 100, "INVALID ERROR", "", 0, NOT_IN_DB , EXEMPT },
                },
        },
/* 1 */ { "" , FRUB1 , 0x817 , 0x216, DIOP_ERR_ECREG,
                {
                    { 100, "", "", 0, DA_NAME , NONEXEMPT },
                },
        },
/* 2 */ { "" , FRUB1 , 0x814, 0x112, DIOP_ERR_NVRAM,
                {
                    { 100, "", "", 0, DA_NAME       , NONEXEMPT  },
                },
        },
/* 3 */ { "" , FRUB1 , 0x817, 0x123, DIOP_ERR_TOD,
                {
                    { 100, ""        , ""  , 0, DA_NAME       , NONEXEMPT },
                },
        },
/* 4 */ { "" , FRUB1, 0x816, 0x140, DIOP_ERR_OP,
                {
                   {  50, "Display"    , "" ,IOP_LED  , NOT_IN_DB, EXEMPT  },
                   {  45, "OP Panel"   , "" ,OP_PANEL , NOT_IN_DB, EXEMPT  },
                   {   5, ""           , "" , 0       , DA_NAME  , EXEMPT  },
                },
        },
/* 5 */ { "" , FRUB1, 0x817, 0x212, DIOP_ERR_BAT,
                {
                     { 90, "Battery"  , "" , BATT   , NOT_IN_DB, EXEMPT  },
                     { 10, "OP Panel" , "" ,OP_PANEL, NOT_IN_DB, EXEMPT },
                },
        },
/* 6 */ { "" , FRUB1, 0x817, 0x210, DIOP_ERR_TOD_P,
                {
                    { 100, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                },
        },
/* 7 */ { "" , FRUB1, 0x817, 0x213, DIOP_ERR_TOD_N,
                {
                    { 100, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                },
        },
/* 8 */ { "" , FRUB1, 0x816, 0x185,DIOP_ERR_KEY  ,
                {
                   { 65 ,"OP Panel", ""   , OP_PANEL, NOT_IN_DB , EXEMPT    },
                   { 30 ,"Keylock" , ""   , KEYLOCK , NOT_IN_DB , EXEMPT    },
                   {  5 , ""       , ""   , 0       , DA_NAME   , NONEXEMPT },
                },
         },
/* 9 */ { "" , FRUB1, 0x817, 0x214, DIOP_EPOW,
                {
                    { 100, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                },
        },
/* 10 */{ "" , FRUB1, 0x817, 0x215, DIOP_DMA_ERR,
                {
                    { 50, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                    { 50, ""       , ""       , 0    , PARENT_NAME, NONEXEMPT },
                },
        },
/* 11 */{ "" , FRUB1, 0x814, 0x114, DIOD_ERR_NVRAM,
                {
                    { 100, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                },
        },
/* 12 */{ "" , FRUB1, 0x816, 0x141, DIOD_ERR_LCD,
                {
                    { 90 ,"OP Panel", ""   , OP_PANEL, NOT_IN_DB , EXEMPT    },
                    { 10 , ""       , ""   , 0       , DA_NAME   , NONEXEMPT },
                },
        },
/* 13 */{ "" , FRUB1, 0x816, 0x186, DIOD_ERR_KEY,
                {
                    { 90 ,"OP Panel", ""   , OP_PANEL, NOT_IN_DB , EXEMPT    },
                    { 10 , ""       , ""   , 0       , DA_NAME   , NONEXEMPT },
                },
        },
/* 14 */{ "" , FRUB1, 0x817, 0x124, DIOD_ERR_TOD_P,
                {
                    { 100, ""       , ""       , 0    , DA_NAME  , NONEXEMPT },
                },
        },
/* 15 */{ "" , FRUB1, 0x817, 0x211, DIOD_ERR_TOD_N,
                {
                    { 97 , ""    , ""   , 0       , DA_NAME     , NONEXEMPT },
                    {  3 , ""    , ""   , 0       , PARENT_NAME , NONEXEMPT },
                },
        },
/* 16 */{ "" , FRUB1, 0x817, 0x217, DIOD_ERR_TOD,
                {
                    { 98 , ""    , ""   , 0       , DA_NAME     , NONEXEMPT },
                    {  2 , ""    , ""   , 0       , PARENT_NAME , NONEXEMPT },
                },
        },
/* 17 */{ "" , FRUB1, 0x817, 0x300, BBU_ERR,
            {
             { 95 ,"Battery Backup" , ""   , BBU_ERR , NOT_IN_DB , EXEMPT    },
             { 5, ""       , ""       , 0    , PARENT_NAME, NONEXEMPT },
            },
        },
/* 18 */{ "" , FRUB1, 0x817, 0x301, CPU_FAN,
            {
             { 95 ,"Cooling fan" , ""   , CPU_FAN , NOT_IN_DB , EXEMPT    },
             { 5, "Interface card",""   , IO_BUS_CARD , NOT_IN_DB   , EXEMPT },
            },
        },
/* 19 */{ "" , FRUB1, 0x817, 0x302, MEDIA_FAN,
            {
             { 95 ,"Cooling fan" , ""   , MEDIA_FAN , NOT_IN_DB , EXEMPT    },
             { 5, "Interface card",""   , IO_BUS_CARD , NOT_IN_DB , EXEMPT },
            },
        },
/* 20 */{ "" , FRUB1, 0x817, 0x303, PS1_FAN,
            {
             { 95 ,"Cooling fan" , ""   , PS1_FAN , NOT_IN_DB , EXEMPT    },
             { 5, "Interface card",""   , IO_BUS_CARD , NOT_IN_DB , EXEMPT },
            },
        },
/* 21 */{ "" , FRUB1, 0x817, 0x304, PS2_FAN,
            {
             { 95 ,"Cooling fan" , ""   , PS2_FAN , NOT_IN_DB , EXEMPT    },
             { 5, "Interface card",""   , IO_BUS_CARD , NOT_IN_DB , EXEMPT },
            },
        },
/* 22 */{ "" , FRUB1, 0x810, 0x401, EXP_CAB_POWER_FAULT,
            {
             { 100 ,"" , ""   , EXP_CAB_PS , NOT_IN_DB , EXEMPT    },
            },
        },
};


