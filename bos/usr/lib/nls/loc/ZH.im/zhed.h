/* @(#)45	1.1  src/bos/usr/lib/nls/loc/ZH.im/zhed.h, ils-zh_CN, bos41B, 9504A 12/19/94 14:38:21  */
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
/* MODULE NAME:         ZHED.H                                                */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method contant file                     */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/******************************************************************************/

/*************************************/
/*          generic contants         */
/*************************************/
#define ON          1
#define OFF         0
#define TRUE        1
#define FALSE       0
#define LOOP        1

/*************************************/
/*     return code to the caller     */
/*************************************/
#define OK          0
#define NOT_OK      1

/*************************************/
/*     return code to the timprocess */
/*************************************/
#define IMED_NOTUSED  3
#define ERROR        -1
#define IMED_USED     0

/*************************************/
/*   definitions for the inputmode   */
/*************************************/
/* ind0 */
#define ALPH_NUM_MODE         0
#define TSANG_JYE_MODE        1
#define PINYIN_MODE           2
#define ENGLISH_CHINESE_MODE  3
#define ABC_MODE              4
#define USER_DEFINED_MODE     5
#define CAPS_LOCK_MODE        6

/* ind1 */
#define HALF                  0
#define FULL                  1

/* ind2 */
#define SELECT_OFF                  0
#define ROW_COLUMN_SELECT_ON        1
#define PINYIN_SELECT_ON            2
#define PINYIN_LEGEND_SELECT_ON     3
#define ENGLISH_CHINESE_SELECT_ON   4
#define ABC_SELECT_ON               5
#define USER_DEFINED_SELECT_ON      6
#define TSANG_JYE_SELECT_ON         7
#define SET_OPTION                  8

/* ind3 */
#define NORMAL_MODE           0
#define SUPPRESSED_MODE       1

/* ind4 */
#define REPLACE_MODE          0
#define INSERT_MODE           1

/* ind5 */
#define BLANK                 0
#define ERROR1                1
#define ERROR2                2
#define RADICAL               3

/* ind6 */
#define INPUT_SELECTED         0
#define INPUT_NO_SELECTED      1

/* ind7 */
#define LEGEND_ON              0
#define LEGEND_OFF             1

/*************************************/
/*           AuxArea Use             */
/*************************************/
#define RESTORE               0
#define BEGINNING             1
#define USE                   1
#define NOTUSE                0
#define AUXCOLMAX             80
#define AUXROWMAX             14

/*************************************/
/*           Beep  Alarm             */
/*************************************/
#define BEEP_OFF              0
#define BEEP_ON               1


#define ALT_STATE             8
#define CONTROL_STATE         4


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
#define CONVERT_KEY           0x20
#define TSANG_JYE_KEY         12
#define UP_KEY                13
#define DOWN_KEY              14
#define RETURN_KEY            15
#define LEFT_ARROW_KEY        16
#define RIGHT_ARROW_KEY       17
#define INSERT_KEY            18
#define DELETE_KEY            19
#define HOME_KEY              20
#define END_KEY               21
#define PGUP_KEY              22
#define PGDN_KEY              23
#define NUM_LOCK_KEY          24
#define NON_CONVERT_KEY       25
#define IMED_SET_OPTION_KEY   26

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

/*************************************/
/*      constant for zhedinit.c      */
/*************************************/
#define MIN_ECHOSIZE          11
#define MIN_AUX               20
#define LONG_AUX_FMT          2
#define DICTNAME_LEN          60

/*************************************/
/*      constant for zhedmisc.c      */
/*************************************/
#define Aux_No_Used           0
#define Aux_Candidate_List    1

/****************************************/
/*   Constant for zhedctrl.c            */
/****************************************/
#define Reset_Aux       0
#define Change_Length   1

#define MAX_PY_INPUT_LEN      8
#define MAX_TJ_INPUT_LEN      5
#define MAX_EN_INPUT_LEN      24
#define MAX_RC_INPUT_LEN      4
#define MAX_ABC_INPUT_LEN     8
#define MAX_UD_INPUT_LEN      30

#define NORMAL_ATTR           0
#define UNDERLINE_ATTR        1
#define REVERSE_ATTR          2
#define MIX_ATTR      UNDERLINE_ATTR | REVERSE_ATTR

/**********************************/
/* ZHIMED process, internal states*/
/**********************************/
/* we need some of new states */
#define PROCESSOFF      26       /* suppressed mode                         */

/****************************************/
/*   Constant for Abc Dictionary File   */
/****************************************/

#define ABCCWD_S        0
#define ABCCWD_T        1
#define ABCOVL          2
#define USRREM          0
#define USR             1

/************************************/
/*   Constant for Dictionary File   */
/************************************/

#define COMM            0
#define GB              1
#define CNS             2
#define JK              3

/************************************/
/*   Constant for Dictionary File   */
/************************************/

#define CHINESE_GB      1
#define CHINESE_CNS     2
#define CHINESE_GLOBLE  3

/*****************************************/
/*  Constant for English_Chinese IM      */
/*****************************************/

#define MASK            0xff

/*****************************************/
/* Constant for Pinyin Territory Options */
/*****************************************/

#define PY_OPTION_TITLE        "   拼音选项设置   "
#define PYOPTION1      " 1  GB汉字开关("
#define PYOPTION2      " 2  CNS汉字开关("
#define PYOPTION3      " 3  CJK汉字开关("
#define PYOPTION_AUXROWMAX           7

/********************************/
/*   Constant for ABC Options   */
/********************************/

#define ABC_OPTION_TITLE        "    智能ABC选项设置   "
#define ABCOPTION1      " 1  标点转换开关("
#define ABCOPTION2      " 2  频度调整开关("
#define ABCOPTION3      " 3  简体汉字开关("
#define ABCOPTION4      " 4  繁体汉字开关("
#define ABCOPTION5      " 5  CJK汉字开关("
#define ABCOPTION_AUXROWMAX           9

/***********************************************/
/* Constant for IM Set Options List Box Bottom */
/***********************************************/
#define OPTION_BOTTOM       "放弃(Esc)  确认(Enter)"
