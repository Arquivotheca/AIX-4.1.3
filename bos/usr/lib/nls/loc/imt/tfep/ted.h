/* @(#)03  1.3.1.1  src/bos/usr/lib/nls/loc/imt/tfep/ted.h, libtw, bos411, 9431A411a 7/26/94 17:17:39 */
/*
 *   COMPONENT_NAME: LIBTW
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/******************************************************************************/
/*                                                                            */
/* MODULE NAME:         TED.H                                                 */
/*                                                                            */
/* DESCRIPTIVE NAME:    Chinese Input Method contant file                     */
/*                                                                            */
/* FUNCTION:                                                                  */
/*                                                                            */
/* MODULE TYPE:         C                                                     */
/*                                                                            */
/* COMPILER:            AIX C                                                 */
/*                                                                            */
/* AUTHOR:              Mei Lin                                               */
/*                                                                            */
/* CHANGE ACTIVITY:     010/16/91 - Modified                                       */
/*                                                                            */
/* STATUS:              Chinese Input Method Version                       */
/*                                                                            */
/* CATEGORY:            Contant                                               */
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
#define PHONETIC_MODE         1
#define TSANG_JYE_MODE        2
#define INTERNAL_CODE_MODE    3
#define CAPS_LOCK_MODE        4

/* ind1 */
#define HALF                  0
#define FULL                  1

/* ind2 */
#define SELECT_OFF            0
#define PHONETIC_SELECT_ON    1
#define STJ_SELECT_ON         2
#define STROKE_SELECT_ON      3

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

/************************************/
/*    ListBox is On/Off             */
/*     V4.1                         */
/************************************/
#define SELECTION_ON          1
#define SELECTION_OFF         0

/***********************************
*     Learning is On/Off           *
*       V4.1                       *
************************************/
#define LEARN_ON              1
#define LEARN_OFF             0

/***********************************
*     codeset will be used         *
*       V4.1                       *
************************************/
#define EUC                   0
#define BIG5                  1

/***********************************
*     keyboard layout              *
*       @big5                      *
************************************/
#define IBM_LAYOUT            0
#define NON_IBM_LAYOUT        1
#define ET_LAYOUT             2       /* @et  */



#define ALT_STATE             8
#define CONTROL_STATE         4


/*************************************/
/*         Key  Definition           */
/*************************************/
#define NO_USE_KEY            0
#define ALPH_NUM_KEY          1
#define PHONETIC_KEY          2
#define TSANG_JYE_KEY         3
#define INTERNAL_CODE_KEY     4
#define FULL_HALF_KEY         5
#define CAPS_LOCK_KEY         6
#define CTRL_RIGHT_KEY        7
#define BACK_SPACE_KEY        8
#define CONVERT_KEY           9
#define NON_CONVERT_KEY       10
#define UP_KEY                11
#define DOWN_KEY              12
#define RETURN_KEY            13
#define LEFT_ARROW_KEY        14
#define RIGHT_ARROW_KEY       15
#define INSERT_KEY            16
#define DELETE_KEY            17
#define HOME_KEY              18
#define CTRL_HOME_KEY         19
#define PGUP_KEY              20
#define PGDN_KEY              21
#define NUM_LOCK_KEY          22
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
/*****************************************************/
/*      Suport Ctrl+Shift keys for Altering @big5    */
/*****************************************************/
#define MODE_NUMBER 4
int  MODE_KEY[ MODE_NUMBER ] =
    {ALPH_NUM_KEY,PHONETIC_KEY,TSANG_JYE_KEY,INTERNAL_CODE_KEY};


/*************************************/
/*      constant for tedinit.c       */
/*************************************/
#define MIN_ECHOSIZE          15               /* 11 -> 15 V410 */
#define MIN_AUX               20
#define LONG_AUX_FMT          2
#define DICTNAME_LEN          60

/*************************************/
/*      constant for tedmisc.c       */
/*************************************/
#define Aux_No_Used           0
#define Aux_Candidate_List    1

/****************************************/
/*   Constant for tedctrl.c             */
/****************************************/
#define Reset_Aux       0
#define Change_Length   1

#define MAX_TJ_INPUT_LEN      5
#define MAX_STJ_INPUT_LEN     3
#define MAX_IC_INPUT_LEN      8
#define MAX_BIG5_IC_INPUT_LEN  4                     /* @big5 */

#define NORMAL_ATTR           0
#define UNDERLINE_ATTR        1
#define REVERSE_ATTR          2
#define MIX_ATTR      UNDERLINE_ATTR | REVERSE_ATTR

/**********************************/
/* TIMED process, internal states */
/**********************************/
/* we need some of new states */
#define PROCESSOFF      4       /* suppressed mode                         */

/**********************************/
/* Multi code support   @big5     */
/**********************************/

#define EUC_CODE_RANGE_NO    7
#define BIG5_CODE_RANGE_NO   7

unsigned char S_CODE_FLAG[12]="\244\305\244\325\244\303 ";
unsigned char S_CODE_FLAG_BIG5[]="\242\320\242\327\242\325\242\264 ";
unsigned char CODE_FLAG[12]="\244\305\244\325\244\303\245\355";
unsigned char CODE_FLAG_BIG5[]="\242\320\242\327\242\325\242\264\241\100";


unsigned int EucRange[EUC_CODE_RANGE_NO][6] = {
  {0x00,0x00,0x00,0x00,0x00,0x8d},
  {0x00,0x00,0x00,0x8f,0x00,0xa0},
  {0x00,0x00,0x00,0xc3,0x00,0xc3},
  {0x00,0x00,0x00,0xfe,0x00,0xff},
  {0x00,0x00,0xa1,0xa1,0xfe,0xfe},
  {0x8ea2,0x8ea4,0xa1,0xa1,0xfe,0xfe},
  {0x8eac,0x8ead,0xa1,0xa1,0xfe,0xfe} };

unsigned int Big5Range[BIG5_CODE_RANGE_NO][6] = {
  {0x00,0x00,0x00,0x00,0x00,0x7f},
  {0x00,0x00,0x81,0x40,0xfe,0x7e},
  {0x00,0x00,0x81,0xa1,0xfe,0xfe},
  {0x00,0x00,0x81,0x81,0x8b,0xa0},
  {0x00,0x00,0x8c,0x81,0x8c,0x82},
  {0x00,0x00,0xf2,0x86,0xf2,0xa0},
  {0x00,0x00,0xf3,0x81,0xf9,0xa0} };
/*  0     1    2    3    4    5   */


#define CODE_SET_NO           2

typedef struct {
        unsigned char name[20];
        unsigned char code_type;
        unsigned char code_flag[10];
        unsigned int code_range_no;
        unsigned int *code_range;
        unsigned int max_ic_input_len;
        unsigned char *s_status_line_code_flag;
        unsigned char *l_status_line_code_flag;
} CodeSet;

CodeSet  codesetinfo[CODE_SET_NO]=
{ {"IBM-eucTW",0,"EUC" ,EUC_CODE_RANGE_NO, EucRange ,MAX_IC_INPUT_LEN,
  S_CODE_FLAG,CODE_FLAG },                         /* default system code set */
  {"big5"     ,1,"BIG5",BIG5_CODE_RANGE_NO,Big5Range,MAX_BIG5_IC_INPUT_LEN,
  S_CODE_FLAG_BIG5,CODE_FLAG_BIG5 }
                                    };


/* #define EUC_CODE              0
   #define BIG5_CODE             1
   #define EUC_CODE_name         "IBM-eucTW"
   #define BIG5_CODE_name        "big5"           */
