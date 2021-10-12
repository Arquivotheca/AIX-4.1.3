/* @(#)65	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhedud.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:39:01  */
/*
 *   COMPONENT_NAME: ils-zh_CN
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/******************************************************************************/
/*                                                                            */
/* MODULE NAME:         ZHEDUD.H                                              */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method contant file                     */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/******************************************************************************/

#include "stdio.h"

#define LEFT_ARROW_FLAG      1
#define RIGHT_ARROW_FLAG     2

#define UD_FOUND             1
#define UD_NOT_FOUND         0

#define TRUE                 1
#define FALSE                0

#define FOUND_CANDIDATE      1
#define NOT_FOUND_CANDIDATE  0

#define LEFT_ARROW_FLAG      1
#define RIGHT_ARROW_FLAG     2

#define FOUND                1
#define NOT_FOUND            0

#define UD_FOUND             1
#define UD_NOT_FOUND         0

#define TRUE                 1
#define FALSE                0

#define FOUND_CANDIDATE      1
#define NOT_FOUND_CANDIDATE  0

#define USER_DEFINED_SELECTED_ON     1
#define SELECT_OFF           0
#define ERROR               -1
#define USER_DEFINED_MODE    5
#define REPLACE_MODE         0
#define INSERT_MODE          1
#define FOUND_WORD           1

#define INPUT_SELECTED       0
#define INPUT_NO_SELECTED    1

#define OK_OK                1

/*************************************/
/*         Key  Definition           */
/*************************************/
#define NO_USE_KEY            0
#define ROW_COLUMN_KEY        1
#define PINYIN_KEY            2
#define ENGLISH_CHINESE_KEY   3
#define LEGEND_SWITCH_KEY     4
#define ABC_KEY               5
#define USER_DEFINED_KEY      6
#define ALPHA_NUM_KEY         7
#define FULL_HALF_KEY         8
#define CAPS_LOCK_KEY         9
#define CTRL_RIGHT_KEY        10
#define BACK_SPACE_KEY        11
#define TSANG_JYE_KEY         12
#define CONVERT_KEY           0x20
#define UP_KEY                13
#define DOWN_KEY              14
#define RETURN_KEY            15
#define LEFT_ARROW_KEY        16
#define RIGHT_ARROW_KEY       17
#define INSERT_KEY            18
#define DELETE_KEY            19
#define HOME_KEY              20
#define CTRL_HOME_KEY         21
#define PGUP_KEY              22
#define PGDN_KEY              23
#define NUM_LOCK_KEY          24
#define NON_CONVERT_KEY       25

#define ESC_KEY               27
#define ADD_KEY               0x2b
#define NUM_KEY0              0x30
#define NUM_KEY1              0x31
#define NUM_KEY2              0x32
#define NUM_KEY3              0x33
#define NUM_KEY4              0x34
#define NUM_KEY5              0x35
#define NUM_KEY6              0x36
#define NUM_KEY7              0x37
#define NUM_KEY8              0x38
#define NUM_KEY9              0x39
#define ALT_NUM_KEY0          0xFF30
#define ALT_NUM_KEY1          0xFF31
#define ALT_NUM_KEY2          0xFF32
#define ALT_NUM_KEY3          0xFF33
#define ALT_NUM_KEY4          0xFF34
#define ALT_NUM_KEY5          0xFF35
#define ALT_NUM_KEY6          0xFF36
#define ALT_NUM_KEY7          0xFF37
#define ALT_NUM_KEY8          0xFF38
#define ALT_NUM_KEY9          0xFF39

typedef int (*UdFunc)();

typedef struct {
        UdFunc         UdInitialCB;
        UdFunc         UdOtherInitialCB;
        UdFunc         UdEraseCurRadicalCB;
        UdFunc         UdEraseAllRadicalCB;
        UdFunc         UdRadicalInputCB;
        UdFunc         BackSpaceCB;
        UdFunc         NonConvertCB; 
        UdFunc         ErrorBeepCB;
        UdFunc         UdListBoxCB;
        UdFunc         UdGetCandCB;
        UdFunc         UdCloseAuxCB;
        UdFunc         UdFreeCandCB; 
        UdFunc         CursorCB;
        UdFunc         NoWordCB;
        UdFunc         UdShowCursorCB;
        UdFunc         halffullCB;
} UdCallBack;

typedef struct{
       unsigned char   *candbuf[10];   /* User Defined IM candidate buffer   */
                                       /* The user defined routines put the  */
                                       /* candidates into the buffer. IM API */
                                       /* get the candidates from this       */
                                       /* buffer, and display them in the    */
                                       /* candidates list box.               */
       int             inputlen   ;   /* Length Of Input Buffer              */
       UdCallBack      udcb;
       unsigned short  more   ;        /* Number Of Remained Candidates       */
       int             (*UserDefinedMain)();    /* User Defined Entry Point   */
       int             (*UserDefinedInit)();    /* Initialization             */
       int             (*UserDefinedInitial)(); /* Initialization             */
       int             (*UserDefinedFilter)();  /* Process User Defined IM    */
                                                /* Input key                  */
       int             (*UserDefinedClose)();
       char            *FileName;               /* Pointer to User Defined    */
                                                /* Dictionary File            */
       int             ret;                     /* User Defined return code   */
       int             maxlen;                  /* The Max. Length of input   */
                                                /* Radical                    */
       caddr_t         udata;
}  UdimCommon;

UdimCommon *udimcomm;

typedef struct {
       int             (*Ud_SwInit)();
       int             (*Ud_SwMain)();
       int             (*Ud_SwInitial)();
       int             (*Ud_SwClose)();
} Ud_Switch;

Ud_Switch               *udswitch;
