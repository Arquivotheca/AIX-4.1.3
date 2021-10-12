static char sccsid[] = "@(#)61  1.2  src/bos/usr/lib/nls/loc/swkbd/zh_CN.c, ils-zh_CN, bos41J, 9514A_all 4/3/95 14:02:33";
/*
 * COMPONENT_NAME: (ILS-ZH_CN) Low Function Terminal - zh_CN.c
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*------------
  The program on the bottom of this file writes out the software
  keyboard to a file.

  Since the compiler is a TOC compiler, when the structure definition
  was compiled (cc -c swkb.c) and then stripped (strip -H), variable-
  length table of contents data is left in the data field.  Therefore,
  executing a program (the main routine added to the bottom of the
  structure definition) became necessary.

  ------------*/
#include <sys/types.h>                  /* see if this is RT or RIOS */
#include "lft_swkbd.h"                  /* copied into this directory */
#include "lftcode.h"                    /* copied into this directory */
#include <fcntl.h>                      /* the system one should be OK */
#include <errno.h>                      /* the system one should be OK */

extern errno;

lft_swkbd_t usa_key_map = {
        {'8','8','5','9','-','1','\0','\0'},
        {'z','h','_','C','N','\0','\0','\0'},
        {                               /* keyb_pos_mappings */
                {
   /************** BASE KEY ASSIGNMENTS  ----  Main key bank  ***************/
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* ignore -- not on keyboard      pos 0   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LQUOT}, /* grave                          pos 1   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_1},     /* 1                              pos 2   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_2},     /* 2                              pos 3   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_3},     /* 3                              pos 4   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_4},     /* 4                              pos 5   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_5},     /* 5                              pos 6   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_6},     /* 6                              pos 7   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_7},     /* 7                              pos 8   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_8},     /* 8                              pos 9   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_9},     /* 9                              pos 10  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_0},     /* 0                              pos 11  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_DASH},  /* -                              pos 12  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_EQ},    /* =                              pos 13  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 14  */
{FLAG_SINGLE_CONTROL,CODE_STAT_CURSOR,0,         0,IC_BS},    /* Backspace                      pos 15  */
{FLAG_SINGLE_CONTROL,CODE_STAT_CURSOR,0,         0,IC_HT},    /* cursor horizontal tab          pos 16  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCQ},   /* q                              pos 17  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCW},   /* w                              pos 18  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCE},   /* e                              pos 19  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCR},   /* r                              pos 20  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCT},   /* t                              pos 21  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCY},   /* y                              pos 22  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCU},   /* u                              pos 23  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCI},   /* i                              pos 24  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCO},   /* o                              pos 25  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCP},   /* p                              pos 26  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LSB},   /* left square bracket            pos 27  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_RSB},   /* right square bracket           pos 28  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_BSLASH}, /* Backslash                     pos 29  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* caps Lock key; do not alter    pos 30  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCA},   /* a                              pos 31  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCS},   /* s                              pos 32  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCD},   /* d                              pos 33  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCF},   /* f                              pos 34  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCG},   /* g                              pos 35  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCH},   /* h                              pos 36  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCJ},   /* j                              pos 37  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCK},   /* k                              pos 38  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCL},   /* l                              pos 39  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SEMI},  /* Semicolon                      pos 40  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_1QUOT}, /* Single Left Quote              pos 41  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 42  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0,         0,IC_CR},    /* carriage Return                pos 43  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* Left Shift Key; do not alter   pos 44  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 45  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCZ},   /* z                              pos 46  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCX},   /* x                              pos 47  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCC},   /* c                              pos 48  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCV},   /* v                              pos 49  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCB},   /* b                              pos 50  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCN},   /* n                              pos 51  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LCM},   /* m                              pos 52  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_COM},   /* ,                              pos 53  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PERIOD},/* .                              pos 54  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SLASH}, /* /                              pos 55  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 56  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* Right Shift; do not alter      pos 57  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* control key; do not alter      pos 58  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 59  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* Left Alternate key; do not alt pos 60  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},    /* space bar                      pos 61  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* Right Alternate key; do not alter pos 62  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE},/* not available                  pos 63  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF114}, /* PFK 114                        pos 64  */
 /****************** BASE KEY ASSIGNMENTS  ----  Function keys bank *********/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 74  */
 /************** BASE KEY ASSIGNMENTS  ----  cursor keys bank   ************/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF139},  /* PFK 139 (often used as Insert key) pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0,1,KF_DCH},    /* Delete character                   pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0,1,KF_CUB},    /* cursor Backward                    pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_CURSOR,0,1,KF_HOM},    /* Home                               pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF146},  /* PFK 146                            pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_CURSOR,0,1,KF_CUU},    /* cursor up                          pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_CURSOR,0,1,KF_CUD},    /* cursor Down                        pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF150},  /* PFK 150                            pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF154},  /* PFK 154                            pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_CURSOR,0,1,KF_CUF},    /* cursor forward                     pos 89  */
 /*********** BASE KEY ASSIGNMENTS  ----  Numeric keypad bank **************/
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 90  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 91  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 92  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 93  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 94  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SLASH},  /* slash                       pos 95  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 96  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 97  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 98  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 99  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AST},    /* Asterisk                    pos 100 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 101 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 102 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 103 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter  pos 104 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_DASH},   /* Dash (Minus)                pos 105 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PLUS},   /* Plus                        pos 106 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 107 */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0,         0,IC_CR},     /* carriage Return             pos 108 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 109 */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0,         0,IC_ESC},    /* Escape                      pos 110 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 111 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF1},    /* PFK 1                       pos 112 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF2},    /* PFK 2                       pos 113 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF3},    /* PFK 3                       pos 114 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF4},    /* PFK 4                       pos 115 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF5},    /* PFK 5                       pos 116 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF6},    /* PFK 6                       pos 117 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF7},    /* PFK 7                       pos 118 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF8},    /* PFK 8                       pos 119 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF9},    /* PFK 9                       pos 120 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF10},   /* PFK 10                      pos 121 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF11},   /* PFK 11                      pos 122 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF12},   /* PFK 12                      pos 123 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF209},  /* PFK 209                     pos 124 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF213},  /* PFK 213                     pos 125 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF217},  /* PFK 217                     pos 126 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 127 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 128 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 129 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 130 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 131 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available               pos 132 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}  /* not available               pos 133 */
                },




                {
/*---------------------------------------------------------------------------
           SHIFT KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 0   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_APPROX}, /* Tilde                              pos 1   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_EXC},    /* !                                  pos 2   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AT},     /* @                                  pos 3   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_POUND},  /* pound sign                         pos 4   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_DOLLAR}, /* $                                  pos 5   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PERCENT},/* %                                  pos 6   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AND},    /* ^ (circumflex)                     pos 7   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AMP},    /* &                                  pos 8   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AST},    /* asterisk                           pos 9   */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LPAR},   /* left paren                         pos 10  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_RPAR},   /* right paren                        pos 11  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC__},      /* _ (underscore)                     pos 12  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PLUS},   /* Plus                               pos 13  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 14  */
{FLAG_SINGLE_CONTROL,CODE_STAT_CURSOR,0,         0,IC_BS},     /* Back Space                         pos 15  */
{FLAG_CNTL_FUNCTION, CODE_STAT_CURSOR,0,         1,KF_CBT},    /* cursor Back Horizontal Tab         pos 16  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCQ},    /* Q                                  pos 17  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCW},    /* W                                  pos 18  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCE},    /* E                                  pos 19  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCR},    /* R                                  pos 20  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCT},    /* T                                  pos 21  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCY},    /* Y                                  pos 22  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCU},    /* U                                  pos 23  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCI},    /* I                                  pos 24  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCO},    /* O                                  pos 25  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCP},    /* P                                  pos 26  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LBRACE}, /* Left curly Brace                   pos 27  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_RBRACE}, /* Right curly Brace                  pos 28  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_OR},     /* Pipe Symbol                        pos 29  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* caps Lock key; do not alter        pos 30  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCA},    /* A                                  pos 31  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCS},    /* S                                  pos 32  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCD},    /* D                                  pos 33  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCF},    /* F                                  pos 34  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCG},    /* G                                  pos 35  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCH},    /* H                                  pos 36  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCJ},    /* J                                  pos 37  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCK},    /* K                                  pos 38  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCL},    /* L                                  pos 39  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_COLON},  /* :                                  pos 40  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_2QUOT},  /* "                                  pos 41  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 42  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0,         0,IC_CR},     /* carriage Return                    pos 43  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 44  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 45  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCZ},    /* Z                                  pos 46  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCX},    /* X                                  pos 47  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCC},    /* C                                  pos 48  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCV},    /* V                                  pos 49  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCB},    /* B                                  pos 50  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCN},    /* N                                  pos 51  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_UCM},    /* M                                  pos 52  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_LT},     /* <                                  pos 53  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_GT},     /* >                                  pos 54  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_QUES},   /* ?                                  pos 55  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 56  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Right Shift key; do not alter      pos 57  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 58  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 59  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Left Alternate key; do not alter   pos 60  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},     /* Space Bar                          pos 61  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Right Alternate key; do not alter  pos 62  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 63  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* VTRM Previous Window key; no alter pos 64  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 65  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 66  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 67  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 68  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 69  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 70  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 71  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 72  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 73  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 74  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF139},  /* PFK 139                            pos 75  */
{FLAG_CNTL_FUNCTION, CODE_STAT_NONE,  0,         1,KF_DCH},    /* Delete character                   pos 76  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 77  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 78  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF158},  /* PFK 158                            pos 79  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF143},  /* PFK 143                            pos 80  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF147},  /* PFK 147                            pos 81  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 82  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF161},  /* PFK 161                            pos 83  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF164},  /* PFK 164                            pos 84  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF151},  /* PFK 151                            pos 85  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF155},  /* PFK 155                            pos 86  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 87  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available                      pos 88  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF167},  /* PFK 167                            pos 89  */
  /*********   SHIFT KEY ASSIGNMENTS  ----  Numeric keypad bank  ********/
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* Num Lock key; do not alter pos 90  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_7},      /* 7                          pos 91  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_4},      /* 4                          pos 92  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_1},      /* 1                          pos 93  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available              pos 94  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SLASH},  /* /                          pos 95  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_8},      /* 8                          pos 96  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_5},      /* 5                          pos 97  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_2},      /* 2                          pos 98  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_0},      /* 0                          pos 99  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_AST},    /* *                          pos 100 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_9},      /* 9                          pos 101 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_6},      /* 6                          pos 102 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_3},      /* 3                          pos 103 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PERIOD}, /* .                          pos 104 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_DASH},   /* -                          pos 105 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_PLUS},   /* +                          pos 106 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available              pos 107 */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0,         0,IC_CR},     /* carriage Return            pos 108 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available              pos 109 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF120},  /* PFK 120                    pos 110 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0,         1,KF_IGNORE}, /* not available              pos 111 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF13},   /* PFK 13                     pos 112 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF14},   /* PFK 14                     pos 113 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF15},   /* PFK 15                     pos 114 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF16},   /* PFK 16                     pos 115 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF17},   /* PFK 17                     pos 116 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF18},   /* PFK 18                     pos 117 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF19},   /* PFK 19                     pos 118 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF20},   /* PFK 20                     pos 119 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF21},   /* PFK 21                     pos 120 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF22},   /* PFK 22                     pos 121 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF23},   /* PFK 23                     pos 122 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF24},   /* PFK 24                     pos 123 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF210},  /* PFK 210                    pos 124 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF214},  /* PFK 214                    pos 125 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0,         0,KF_PF218},  /* PFK 218                    pos 126 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 127 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 128 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 129 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 130 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 131 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}, /* not available              pos 132 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,   0,         1,KF_IGNORE}  /* not available              pos 133 */
                },





                {
/*---------------------------------------------------------------------------
          CONTROL KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 0   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF57},   /* PFK 57                            pos 1   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF49},   /* PFK 49                            pos 2   */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_NUL},    /* Null                              pos 3   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF50},   /* PFK 50                            pos 4   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF51},   /* PFK 51                            pos 5   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF52},   /* PFK 52                            pos 6   */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SS2},    /* Single Shift 2                    pos 7   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF53},   /* PFK 53                            pos 8   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF54},   /* PFK 54                            pos 9   */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF55},   /* PFK 55                            pos 10  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF56},   /* PFK 56                            pos 11  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SS1},    /* Single Shift 1                    pos 12  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF69},   /* PFK 69                            pos 13  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 14  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DEL},    /* Delete                            pos 15  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF72},   /* PFK 72                            pos 16  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DC1},    /* Device control 1                  pos 17  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_ETB},    /* End of Block                      pos 18  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_ENQ},    /* Enquiry                           pos 19  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DC2},    /* Device control 2                  pos 20  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DC4},    /* Device control 4                  pos 21  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_EM},     /* End of Medium                     pos 22  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_NAK},    /* Negative Acknowledgment           pos 23  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_HT},     /* Horizontal Tab                    pos 24  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SI},     /* Shift In                          pos 25  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DLE},    /* Data Link Escape                  pos 26  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_ESC},    /* Escape                            pos 27  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SS3},    /* Single Shift 3                    pos 28  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SS4},    /* Single Shift 4                    pos 29  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* caps Lock key; do not alter       pos 30  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SOH},    /* Start of Header                   pos 31  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DC3},    /* Device control 3                  pos 32  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_EOT},    /* End of Transmission               pos 33  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_ACK},    /* Acknowledgment                    pos 34  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_BEL},    /* Bell                              pos 35  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_BS},     /* Backspace                         pos 36  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_LF},     /* Line Feed                         pos 37  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_VT},     /* Vertical Tab                      pos 38  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_FF},     /* Form Feed                         pos 39  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF96},   /* PFK 96                            pos 40  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF98},   /* PFK 98                            pos 41  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 42  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_CR},     /* carriage Return                   pos 43  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Shift key; do not alter      pos 44  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 45  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SUB},    /* Substitute                        pos 46  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_CAN},    /* cancel                            pos 47  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_ETX},    /* End of Text                       pos 48  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SYN},    /* Synchronous                       pos 49  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_STX},    /* Start of Text                     pos 50  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_SO},     /* Shift Out                         pos 51  */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_CR},     /* carriage Return                   pos 52  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF108},  /* PFK 108                           pos 53  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF110},  /* PFK 110                           pos 54  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF112},  /* PFK 112                           pos 55  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 56  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Shift key; do not alter     pos 57  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* control key; do not alter         pos 58  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 59  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Alternate key; do not alter  pos 60  */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},     /* Space Bar                 pos 61  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Alternate key; do not alter pos 62  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                     pos 63  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* VTRM Windows key; do not alter    pos 64  */
/*********  CONTROL KEY ASSIGNMENTS  ----  Function keys bank **************/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 74  */
 /*********** CONTROL KEY ASSIGNMENTS  ----  cursor keys bank **********/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF140},  /* PFK 140                   pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF142},  /* PFK 142                   pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF159},  /* PFK 159                   pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF144},  /* PFK 144                   pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF148},  /* PFK 148                   pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF162},  /* PFK 162                   pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF165},  /* PFK 165                   pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF152},  /* PFK 152                   pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF156},  /* PFK 156                   pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF168},  /* PFK 168                   pos 89  */
 /******************* CONTROL KEY ASSIGNMENTS  ----  Numeric keypad bank ******/
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_DC3},    /* Device control 3          pos 90  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF172},  /* PFK 172                   pos 91  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF174},  /* PFK 174                   pos 92  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF176},  /* PFK 176                   pos 93  */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 94  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF179},  /* PFK 179                   pos 95  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF182},  /* PFK 182                   pos 96  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF184},  /* PFK 184                   pos 97  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF186},  /* PFK 186                   pos 98  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF178},  /* PFK 178                   pos 99  */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF187},  /* PFK 187                   pos 100 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF190},  /* PFK 190                   pos 101 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF192},  /* PFK 192                   pos 102 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF194},  /* PFK 194                   pos 103 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF196},  /* PFK 196                   pos 104 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF198},  /* PFK 198                   pos 105 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF200},  /* PFK 200                   pos 106 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 107 */
{FLAG_SINGLE_CONTROL,CODE_STAT_NONE,  0, 0,IC_CR},     /* carriage Return           pos 108 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 109 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF121},  /* PFK 121                   pos 110 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 111 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF25},   /* PFK 25                    pos 112 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF26},   /* PFK 26                    pos 113 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF27},   /* PFK 27                    pos 114 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF28},   /* PFK 28                    pos 115 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF29},   /* PFK 29                    pos 116 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF30},   /* PFK 30                    pos 117 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF31},   /* PFK 31                    pos 118 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF32},   /* PFK 32                    pos 119 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF33},   /* PFK 33                    pos 120 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF34},   /* PFK 34                    pos 121 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF35},   /* PFK 35                    pos 122 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF36},   /* PFK 36                    pos 123 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF211},  /* PFK 211                   pos 124 */
{FLAG_CNTL_FUNCTION, CODE_STAT_PF_KEY,0, 0,KF_PF215},  /* PFK 215                   pos 125 */
{FLAG_GRAPHIC,       CODE_STAT_NONE,  0,CHARSET_P0,IC_DEL},    /* Delete                    pos 126 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 127 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 128 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 129 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 130 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 131 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available             pos 132 */
{FLAG_ESC_FUNCTION,  CODE_STAT_NONE,  0, 1,KF_IGNORE}  /* not available             pos 133 */
                },





                {
/*---------------------------------------------------------------------------
                 ALT KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 0   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF115},  /* PFK 115                            pos 1   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF58},   /* PFK 58                             pos 2   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF59},   /* PFK 59                             pos 3   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF60},   /* PFK 60                             pos 4   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF61},   /* PFK 61                             pos 5   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF62},   /* PFK 62                             pos 6   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF63},   /* PFK 63                             pos 7   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF64},   /* PFK 64                             pos 8   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF65},   /* PFK 65                             pos 9   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF66},   /* PFK 66                             pos 10  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF67},   /* PFK 67                             pos 11  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF68},   /* PFK 68                             pos 12  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF70},   /* PFK 70                             pos 13  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 14  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF71},   /* PFK 71                             pos 15  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF73},   /* PFK 73                             pos 16  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF74},   /* PFK 74                             pos 17  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF75},   /* PFK 75                             pos 18  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF76},   /* PFK 76                             pos 19  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF77},   /* PFK 77                             pos 20  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF78},   /* PFK 78                             pos 21  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF79},   /* PFK 79                             pos 22  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF80},   /* PFK 80                             pos 23  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF81},   /* PFK 81                             pos 24  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF82},   /* PFK 82                             pos 25  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF83},   /* PFK 83                             pos 26  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF84},   /* PFK 84                             pos 27  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF85},   /* PFK 85                             pos 28  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF86},   /* PFK 86                             pos 29  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* caps Lock key; do not alter        pos 30  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF87},   /* PFK 87                             pos 31  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF88},   /* PFK 88                             pos 32  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF89},   /* PFK 89                             pos 33  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF90},   /* PFK 90                             pos 34  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF91},   /* PFK 91                             pos 35  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF92},   /* PFK 92                             pos 36  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF93},   /* PFK 93                             pos 37  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF94},   /* PFK 94                             pos 38  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF95},   /* PFK 95                             pos 39  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF97},   /* PFK 97                             pos 40  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF99},   /* PFK 99                             pos 41  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 42  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                            pos 43  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Shift key; do not alter       pos 44  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 45  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF101},  /* PFK 101                            pos 46  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF102},  /* PFK 102                            pos 47  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF103},  /* PFK 103                            pos 48  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF104},  /* PFK 104                            pos 49  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF105},  /* PFK 105                            pos 50  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF106},  /* PFK 106                            pos 51  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF107},  /* PFK 107                            pos 52  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF109},  /* PFK 109                            pos 53  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF111},  /* PFK 111                            pos 54  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF113},  /* PFK 113                            pos 55  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 56  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Shift key; do not alter      pos 57  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* control key; do not alter          pos 58  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Alternate key; do not alter   pos 60  */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},     /* Space Bar                          pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Alternate key; do not alter  pos 62  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 63  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* VTRM Next Window key; do not alter pos 64  */
  /************ ALT KEY ASSIGNMENTS  ----  Function keys bank   **********/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available      pos 74  */
  /************* ALT KEY ASSIGNMENTS  ----  cursor keys bank  *************/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF141},  /* PFK 141               pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0, 1,KF_DL},     /* Delete Line           pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF160},  /* PFK 160               pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF145},  /* PFK 145               pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF149},  /* PFK 149               pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF163},  /* PFK 163               pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF166},  /* PFK 166               pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF153},  /* PFK 153               pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF157},  /* PFK 157               pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF169},  /* PFK 169               pos 89  */
 /*********** ALT KEY ASSIGNMENTS  ----  Numeric keypad bank  ***************/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF170},  /* PFK 170                            pos 90  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (7      pos 91  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (4      pos 92  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (1      pos 93  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 94  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF180},  /* PFK 180                            pos 95  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (8      pos 96  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (5      pos 97  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (2      pos 98  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (0      pos 99  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF188},  /* PFK 188                            pos 100 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (9      pos 101 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (6      pos 102 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (3      pos 103 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF197},  /* PFK 197                            pos 104 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF199},  /* PFK 199                            pos 105 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF201},  /* PFK 201                            pos 106 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 107 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                            pos 108 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 109 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF122},  /* PFK 122                            pos 110 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 111 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF37},   /* PFK 37                             pos 112 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF38},   /* PFK 38                             pos 113 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF39},   /* PFK 39                             pos 114 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF40},   /* PFK 40                             pos 115 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF41},   /* PFK 41                             pos 116 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF42},   /* PFK 42                             pos 117 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF43},   /* PFK 43                             pos 118 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF44},   /* PFK 44                             pos 119 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF45},   /* PFK 45                             pos 120 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF46},   /* PFK 46                             pos 121 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF47},   /* PFK 47                             pos 122 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF48},   /* PFK 48                             pos 123 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF212},  /* PFK 212                            pos 124 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF216},  /* PFK 216                            pos 125 */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_DEL},    /* Delete                             pos 126 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 127 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 128 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 129 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 130 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 131 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 132 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}  /* not available                      pos 133 */
                },



                {
/*---------------------------------------------------------------------------
                 ALT GRAPHIcS KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 0   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF115},  /* PFK 115                            pos 1   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF58},   /* PFK 58                             pos 2   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF59},   /* PFK 59                             pos 3   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF60},   /* PFK 60                             pos 4   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF61},   /* PFK 61                             pos 5   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF62},   /* PFK 62                             pos 6   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF63},   /* PFK 63                             pos 7   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF64},   /* PFK 64                             pos 8   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF65},   /* PFK 65                             pos 9   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF66},   /* PFK 66                             pos 10  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF67},   /* PFK 67                             pos 11  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF68},   /* PFK 68                             pos 12  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF70},   /* PFK 70                             pos 13  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 14  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF71},   /* PFK 71                             pos 15  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF73},   /* PFK 73                             pos 16  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF74},   /* PFK 74                             pos 17  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF75},   /* PFK 75                             pos 18  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF76},   /* PFK 76                             pos 19  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF77},   /* PFK 77                             pos 20  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF78},   /* PFK 78                             pos 21  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF79},   /* PFK 79                             pos 22  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF80},   /* PFK 80                             pos 23  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF81},   /* PFK 81                             pos 24  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF82},   /* PFK 82                             pos 25  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF83},   /* PFK 83                             pos 26  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF84},   /* PFK 84                             pos 27  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF85},   /* PFK 85                             pos 28  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF86},   /* PFK 86                             pos 29  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* caps Lock key; do not alter        pos 30  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF87},   /* PFK 87                             pos 31  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF88},   /* PFK 88                             pos 32  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF89},   /* PFK 89                             pos 33  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF90},   /* PFK 90                             pos 34  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF91},   /* PFK 91                             pos 35  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF92},   /* PFK 92                             pos 36  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF93},   /* PFK 93                             pos 37  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF94},   /* PFK 94                             pos 38  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF95},   /* PFK 95                             pos 39  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF97},   /* PFK 97                             pos 40  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF99},   /* PFK 99                             pos 41  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 42  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                            pos 43  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Shift key; do not alter       pos 44  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 45  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF101},  /* PFK 101                            pos 46  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF102},  /* PFK 102                            pos 47  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF103},  /* PFK 103                            pos 48  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF104},  /* PFK 104                            pos 49  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF105},  /* PFK 105                            pos 50  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF106},  /* PFK 106                            pos 51  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF107},  /* PFK 107                            pos 52  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF109},  /* PFK 109                            pos 53  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF111},  /* PFK 111                            pos 54  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF113},  /* PFK 113                            pos 55  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 56  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Shift key; do not alter      pos 57  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* control key; do not alter          pos 58  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Alternate key; do not alter   pos 60  */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},     /* Space Bar                          pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Alternate key; do not alter  pos 62  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 63  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* VTRM Next Window key; do not alter pos 64  */
  /************** ALT KEY ASSIGNMENTS  ----  Function keys bank  **********/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available                 pos 74  */
  /************** ALT KEY ASSIGNMENTS  ----  cursor keys bank   ***********/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF141},  /* PFK 141                       pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0, 1,KF_DL},     /* Delete Line                   pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF160},  /* PFK 160                       pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF145},  /* PFK 145                       pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF149},  /* PFK 149                       pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF163},  /* PFK 163                       pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF166},  /* PFK 166                       pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF153},  /* PFK 153                       pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF157},  /* PFK 157                       pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF169},  /* PFK 169                       pos 89  */
  /************** ALT KEY ASSIGNMENTS  ----  Numeric keypad bank  **********/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF170},  /* PFK 170                       pos 90  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (7 pos 91  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (4 pos 92  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (1 pos 93  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 94  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF180},  /* PFK 180                       pos 95  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (8 pos 96  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (5 pos 97  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (2 pos 98  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (0 pos 99  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF188},  /* PFK 188                       pos 100 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (9 pos 101 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (6 pos 102 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (3 pos 103 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF197},  /* PFK 197                       pos 104 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF199},  /* PFK 199                       pos 105 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF201},  /* PFK 201                       pos 106 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 107 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                       pos 108 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 109 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF122},  /* PFK 122                       pos 110 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 111 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF37},   /* PFK 37                        pos 112 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF38},   /* PFK 38                        pos 113 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF39},   /* PFK 39                        pos 114 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF40},   /* PFK 40                        pos 115 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF41},   /* PFK 41                        pos 116 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF42},   /* PFK 42                        pos 117 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF43},   /* PFK 43                        pos 118 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF44},   /* PFK 44                        pos 119 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF45},   /* PFK 45                        pos 120 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF46},   /* PFK 46                        pos 121 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF47},   /* PFK 47                        pos 122 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF48},   /* PFK 48                        pos 123 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF212},  /* PFK 212                       pos 124 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF216},  /* PFK 216                       pos 125 */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_DEL},    /* Delete                        pos 126 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 127 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 128 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 129 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 130 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 131 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 132 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}  /* not available                 pos 133 */
                },




                {
/*---------------------------------------------------------------------------
                 ALT KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 0   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF115},  /* PFK 115                            pos 1   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF58},   /* PFK 58                             pos 2   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF59},   /* PFK 59                             pos 3   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF60},   /* PFK 60                             pos 4   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF61},   /* PFK 61                             pos 5   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF62},   /* PFK 62                             pos 6   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF63},   /* PFK 63                             pos 7   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF64},   /* PFK 64                             pos 8   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF65},   /* PFK 65                             pos 9   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF66},   /* PFK 66                             pos 10  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF67},   /* PFK 67                             pos 11  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF68},   /* PFK 68                             pos 12  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF70},   /* PFK 70                             pos 13  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 14  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF71},   /* PFK 71                             pos 15  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF73},   /* PFK 73                             pos 16  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF74},   /* PFK 74                             pos 17  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF75},   /* PFK 75                             pos 18  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF76},   /* PFK 76                             pos 19  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF77},   /* PFK 77                             pos 20  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF78},   /* PFK 78                             pos 21  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF79},   /* PFK 79                             pos 22  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF80},   /* PFK 80                             pos 23  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF81},   /* PFK 81                             pos 24  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF82},   /* PFK 82                             pos 25  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF83},   /* PFK 83                             pos 26  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF84},   /* PFK 84                             pos 27  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF85},   /* PFK 85                             pos 28  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF86},   /* PFK 86                             pos 29  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* caps Lock key; do not alter        pos 30  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF87},   /* PFK 87                             pos 31  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF88},   /* PFK 88                             pos 32  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF89},   /* PFK 89                             pos 33  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF90},   /* PFK 90                             pos 34  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF91},   /* PFK 91                             pos 35  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF92},   /* PFK 92                             pos 36  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF93},   /* PFK 93                             pos 37  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF94},   /* PFK 94                             pos 38  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF95},   /* PFK 95                             pos 39  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF97},   /* PFK 97                             pos 40  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF99},   /* PFK 99                             pos 41  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 42  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF100},  /* PFK 100                            pos 43  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* Left Shift key; do not alter       pos 44  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 45  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF101},  /* PFK 101                            pos 46  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF102},  /* PFK 102                            pos 47  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF103},  /* PFK 103                            pos 48  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF104},  /* PFK 104                            pos 49  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF105},  /* PFK 105                            pos 50  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF106},  /* PFK 106                            pos 51  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF107},  /* PFK 107                            pos 52  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF109},  /* PFK 109                            pos 53  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF111},  /* PFK 111                            pos 54  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF113},  /* PFK 113                            pos 55  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 56  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* Right Shift key; do not alter      pos 57  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* control key; do not alter          pos 58  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* Left Alternate key; do not alter   pos 60  */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},  /* Space Bar                          pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* Right Alternate key; do not alter  pos 62  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                      pos 63  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* VTRM Next Window key; do not alter pos 64  */
  /*************** ALT KEY ASSIGNMENTS  ----  Function keys bank   ********/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0,1,KF_IGNORE}, /* not available         pos 74  */
  /************ ALT KEY ASSIGNMENTS  ----  cursor keys bank  *********/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF141},  /* PFK 141               pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0,1,KF_DL},     /* Delete Line           pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available         pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available         pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF160},  /* PFK 160               pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF145},  /* PFK 145               pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF149},  /* PFK 149               pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available         pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF163},  /* PFK 163               pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF166},  /* PFK 166               pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF153},  /* PFK 153               pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF157},  /* PFK 157               pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available         pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available         pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF169},  /* PFK 169               pos 89  */
  /************** ALT KEY ASSIGNMENTS  ----  Numeric keypad bank  *******/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF170},  /* PFK 170                       pos 90  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (7 pos 91  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (4 pos 92  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (1 pos 93  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 94  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF180},  /* PFK 180                       pos 95  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (8 pos 96  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (5 pos 97  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (2 pos 98  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (0 pos 99  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF188},  /* PFK 188                       pos 100 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (9 pos 101 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (6 pos 102 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available; alt+num pad (3 pos 103 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF197},  /* PFK 197                       pos 104 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF199},  /* PFK 199                       pos 105 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF201},  /* PFK 201                       pos 106 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 107 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF100},  /* PFK 100                       pos 108 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 109 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF122},  /* PFK 122                       pos 110 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 111 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF37},   /* PFK 37                        pos 112 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF38},   /* PFK 38                        pos 113 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF39},   /* PFK 39                        pos 114 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF40},   /* PFK 40                        pos 115 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF41},   /* PFK 41                        pos 116 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF42},   /* PFK 42                        pos 117 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF43},   /* PFK 43                        pos 118 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF44},   /* PFK 44                        pos 119 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF45},   /* PFK 45                        pos 120 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF46},   /* PFK 46                        pos 121 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF47},   /* PFK 47                        pos 122 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF48},   /* PFK 48                        pos 123 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF212},  /* PFK 212                       pos 124 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0,0,KF_PF216},  /* PFK 216                       pos 125 */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_DEL}, /* Delete                        pos 126 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 127 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 128 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 129 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 130 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 131 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}, /* not available                 pos 132 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0,1,KF_IGNORE}  /* not available                 pos 133 */
                },





                {
/*---------------------------------------------------------------------------
                 ALT KEY ASSIGNMENTS  ----  Main key bank
---------------------------------------------------------------------------*/
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 0   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF115},  /* PFK 115                            pos 1   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF58},   /* PFK 58                             pos 2   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF59},   /* PFK 59                             pos 3   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF60},   /* PFK 60                             pos 4   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF61},   /* PFK 61                             pos 5   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF62},   /* PFK 62                             pos 6   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF63},   /* PFK 63                             pos 7   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF64},   /* PFK 64                             pos 8   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF65},   /* PFK 65                             pos 9   */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF66},   /* PFK 66                             pos 10  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF67},   /* PFK 67                             pos 11  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF68},   /* PFK 68                             pos 12  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF70},   /* PFK 70                             pos 13  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 14  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF71},   /* PFK 71                             pos 15  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF73},   /* PFK 73                             pos 16  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF74},   /* PFK 74                             pos 17  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF75},   /* PFK 75                             pos 18  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF76},   /* PFK 76                             pos 19  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF77},   /* PFK 77                             pos 20  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF78},   /* PFK 78                             pos 21  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF79},   /* PFK 79                             pos 22  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF80},   /* PFK 80                             pos 23  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF81},   /* PFK 81                             pos 24  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF82},   /* PFK 82                             pos 25  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF83},   /* PFK 83                             pos 26  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF84},   /* PFK 84                             pos 27  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF85},   /* PFK 85                             pos 28  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF86},   /* PFK 86                             pos 29  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* caps Lock key; do not alter        pos 30  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF87},   /* PFK 87                             pos 31  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF88},   /* PFK 88                             pos 32  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF89},   /* PFK 89                             pos 33  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF90},   /* PFK 90                             pos 34  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF91},   /* PFK 91                             pos 35  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF92},   /* PFK 92                             pos 36  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF93},   /* PFK 93                             pos 37  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF94},   /* PFK 94                             pos 38  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF95},   /* PFK 95                             pos 39  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF97},   /* PFK 97                             pos 40  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF99},   /* PFK 99                             pos 41  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 42  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                            pos 43  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Shift key; do not alter       pos 44  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 45  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF101},  /* PFK 101                            pos 46  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF102},  /* PFK 102                            pos 47  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF103},  /* PFK 103                            pos 48  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF104},  /* PFK 104                            pos 49  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF105},  /* PFK 105                            pos 50  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF106},  /* PFK 106                            pos 51  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF107},  /* PFK 107                            pos 52  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF109},  /* PFK 109                            pos 53  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF111},  /* PFK 111                            pos 54  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF113},  /* PFK 113                            pos 55  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 56  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Shift key; do not alter      pos 57  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* control key; do not alter          pos 58  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Left Alternate key; do not alter   pos 60  */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_SP},     /* Space Bar                          pos 59  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* Right Alternate key; do not alter  pos 62  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                      pos 63  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* VTRM Next Window key; do not alter pos 64  */
  /************ ALT KEY ASSIGNMENTS  ----  Function keys bank   ***********/
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 65  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 66  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 67  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 68  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 69  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 70  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 71  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 72  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 73  */
{FLAG_ESC_FUNCTION,CODE_STAT_NONE,0, 1,KF_IGNORE}, /* not available        pos 74  */
  /****************** ALT KEY ASSIGNMENTS  ----  cursor keys bank  ********/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF141},  /* PFK 141               pos 75  */
{FLAG_CNTL_FUNCTION,CODE_STAT_NONE,  0, 1,KF_DL},     /* Delete Line           pos 76  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 77  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 78  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF160},  /* PFK 160               pos 79  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF145},  /* PFK 145               pos 80  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF149},  /* PFK 149               pos 81  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 82  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF163},  /* PFK 163               pos 83  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF166},  /* PFK 166               pos 84  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF153},  /* PFK 153               pos 85  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF157},  /* PFK 157               pos 86  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 87  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available         pos 88  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF169},  /* PFK 169               pos 89  */
  /**************** ALT KEY ASSIGNMENTS  ----  Numeric keypad bank *******/
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF170},  /* PFK 170                       pos 90  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (7 pos 91  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (4 pos 92  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (1 pos 93  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 94  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF180},  /* PFK 180                       pos 95  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (8 pos 96  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (5 pos 97  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (2 pos 98  */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (0 pos 99  */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF188},  /* PFK 188                       pos 100 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (9 pos 101 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (6 pos 102 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available; alt+num pad (3 pos 103 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF197},  /* PFK 197                       pos 104 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF199},  /* PFK 199                       pos 105 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF201},  /* PFK 201                       pos 106 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 107 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF100},  /* PFK 100                       pos 108 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 109 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF122},  /* PFK 122                       pos 110 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 111 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF37},   /* PFK 37                        pos 112 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF38},   /* PFK 38                        pos 113 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF39},   /* PFK 39                        pos 114 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF40},   /* PFK 40                        pos 115 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF41},   /* PFK 41                        pos 116 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF42},   /* PFK 42                        pos 117 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF43},   /* PFK 43                        pos 118 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF44},   /* PFK 44                        pos 119 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF45},   /* PFK 45                        pos 120 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF46},   /* PFK 46                        pos 121 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF47},   /* PFK 47                        pos 122 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF48},   /* PFK 48                        pos 123 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF212},  /* PFK 212                       pos 124 */
{FLAG_CNTL_FUNCTION,CODE_STAT_PF_KEY,0, 0,KF_PF216},  /* PFK 216                       pos 125 */
{FLAG_GRAPHIC,      CODE_STAT_NONE,  0,CHARSET_P0,IC_DEL},    /* Delete                        pos 126 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 127 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 128 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 129 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 130 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 131 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}, /* not available                 pos 132 */
{FLAG_ESC_FUNCTION, CODE_STAT_NONE,  0, 1,KF_IGNORE}  /* not available                 pos 133 */
                }


        },
        {                               /* capslock           */
         0x00007fe1,
         0xff03f800,
         0x00000000,
         0x00000000,
         0x00000000,
        },
};


main ()
{
    int fd;             /* file descriptor */
    int rc;             /* return code */

    fd = open("zh_CN.IBM-eucCN.lftkeymap",O_CREAT|O_RDWR, 0777);
    if (fd == -1) {
        printf("ERROR:  bad open of software keyboard output file.\n");
        printf("        errno=%d\n",errno);
        exit(-1);
    }

    rc = write(fd,&usa_key_map,sizeof(lft_swkbd_t));
    if (rc != sizeof(lft_swkbd_t)) {
        printf("ERROR:  bad write to software keyboard output file.\n");
        printf("        errno=%d\n",errno);
        exit(-1);
    }

    rc = close(fd);
    if (rc != 0) {
        printf("ERROR:  bad close of software keyboard output file.\n");
        printf("        errno=%d\n", errno);
        exit(-1);
    }

    exit(0);
}
