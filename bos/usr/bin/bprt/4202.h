/* @(#)04	1.1  src/bos/usr/bin/bprt/4202.h, libbidi, bos411, 9428A410j 8/27/93 09:56:15 */
/*
 *   COMPONENT_NAME: LIBBIDI
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
{
/***************************************/
/* char            PrinterName[14]   = */ { "4202.CFG" },
/* unsigned short  PRT_CHAR2_ATTR1   = */   0x8000 ,
/* unsigned short  PRT_LAST          = */   80     ,
/* unsigned short  PRT_DFLT_LENGTH   = */   16320  ,
/* unsigned short  PRT_L_MARGIN      = */   0      ,
/* unsigned short  PRT_R_MARGIN      = */   80     ,
/* unsigned char   PRT_D_PITCH       = */   10     ,
/* unsigned short  PRT_OLD_CWIDTH    = */   204    ,
/* unsigned char   PRT_SPACE_FOR[11] = */   {0,0,0,0,0,0,0,0,0,0,0},
/* unsigned short  PRT_SPEC_PITCH    = */   119    ,
/* unsigned short  RESERVED[2]       = */   {0,0}  ,
/* unsigned char   EXPANSION_CODE    = */   0      ,
/* unsigned char   TAB_FLAG          = */   1,
/* unsigned char   PRT_HOME_HEAD[8]  = */   {0,0,0,0,0,0,0,0},
/* unsigned short  PRT_MIN_PNTER     = */   6*9,
/* unsigned short  PRT_MIN_CWIDTH    = */   119,
/* unsigned short  PRT_MIN_PITCH     = */   17,
/* unsigned short  PRT_HT_ATTR_1     = */   0xFBDF,
/* unsigned short  PRT_HT_ATTR_2     = */   0xFFFF,
/* unsigned short  PRT_END_ATTR_1    = */   0,
/* unsigned short  PRT_END_ATTR_2    = */   0,
/* unsigned char   TAIL_FLAG         = */   0x9F,
/* unsigned short  PRT_ATTR1         = */   0x8000,
/* unsigned short  PRT_ATTR2         = */   0x0000,
/* unsigned short  PRT_PSM_ATTR_1    = */   0x0000,
/* unsigned short  PRT_PSM_ATTR_2    = */   0x0000,
/* unsigned char   DESELECT_n        = */   3,
/* unsigned char   PRT_420X          = */   1,
/* unsigned char   PRT_DEF_TABS[28]  = */         /* Default Tab Stops */
                                           {8*1,8*2,8*3,8*4,8*5,8*6,8*7        ,
                                            8*8,8*9,8*10,8*11,8*12,8*13,8*14   ,
                                            8*15,8*16,0*17,0*18,0*19,0*20,0*21 ,
                                            0*22,0*23,0*24,0*25,0*26,0*27,0*28},

/* NORMAL_Elements  NORMAL_CHARS_TAB[33] = */
                  {      0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0X00  , SPECIAL_ONE,
                         0X00  , SPECIAL_ONE,
                         0X00  , SPECIAL_ONE,
                         0X00  , SPECIAL_ONE,
                         0X00  , PRINT_SERVICE_1,
                         0X00  , DESTRUCT_BS,
                         0X00  , HT_FOUND,
                         /*0X40+4, FLUSH_BUFFER,*/
                         0X40+4, PRINT_BUFF,
                         0X40+4, FLUSH_BUFFER,
                         0X40+4, PRINT_BUFF,
                         0X40+4, PRINT_BUFF,
                    0X80+0X40+4, SO_SI,
                    0X80+0X40+1, SO_SI,
                         0X00  , PASS_THROUGH,
                         0X00  , PRT_SELECT,
                         0X40+5, RESERVED_1,
                         0X00  , PASS_THROUGH,
                         0X40+4, SO_SI,
                         0X00  , SPECIAL_ONE,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , CAN_FOUND,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , ESC_FOUND,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , DESTRUCT_BS
                         },

/* ESC_Elements  ESC_CHARS_TAB[45] = */
      {                           /*******************************************/
       'A',0,                     /*   ESC A                                 */
       IGNORE_ESC_n        ,      /*   N/72 Line Spacing                     */
       'B',0,                     /*   ESC B                                 */
        ZERO_TERMINATOR    ,      /*   Vertical Tab Settings                 */
       'C',0,                     /*   ESC C                                 */
        ESC_C_FOUND        ,      /*   Set Form Length                       */
       'D',0,                     /*   ESC D                                 */
        TAB_SET_FOUND      ,      /*   Set Horizontal Tabs                   */
       'E',0x80+29,               /*   ESC E                                 */
        ESC_SINGLE         ,      /*   Emphasized Printing                   */
       'F',0x00+29,               /*   ESC F                                 */
        ESC_SINGLE         ,      /*   Cancel Emphasized Printing            */
       'G',0x80+30,               /*   ESC G                                 */
        ESC_SINGLE         ,      /*   Double Strike ( NLQ ) Printing        */
       'H',0x00+30,               /*   ESC H                                 */
        ESC_SINGLE         ,      /*   Cancel Double Strike ( NLQ ) Printing */
       'I',0,                     /*   ESC I                                 */
        ESC_I              ,      /*   Select Print Mode                     */
       'J',0,                     /*   ESC J                                 */
        REVERSE_LF_n       ,      /*   Variable Line Feed                    */
       'K',0,                     /*   ESC K                                 */
        GRAPHICS           ,      /*   480 Bit-Image Graphics                */
       'L',0,                     /*   ESC L                                 */
        GRAPHICS           ,      /*   960 Bit-Image Graphics ( half speed ) */
       'N',0,                     /*   ESC N                                 */
        IGNORE_ESC_n       ,      /*   Set Automatic Perforation Skip        */
       'Q',0,                     /*   ESC Q                                 */
        PRT_DESELECT_n     ,      /*   Deselect IBM Proprinter               */
       'R',0,                     /*   ESC R                                 */
        RESET_TAB_SETTINGS ,      /*   Set all Tabs to Power on Settings     */
       'S',0x80+28,               /*   ESC S                                 */
        ESC_SUB_SUPER      ,      /*   Subscript or Superscript Printing     */
       'T',0x20+0,                /*   ESC T                                 */
        ESC_SINGLE         ,      /*   Cancel Subscript or Superscript       */
       'U',0,                     /*   ESC U                                 */
        IGNORE_ESC_n       ,      /*   Print in One Direction                */
       'W',0x80+0x40+0,           /*   ESC W                                 */
        ESC_SINGLE_0_1     ,      /*   Continuous Double wide Printing       */
       'X', 0,                    /*   ESC X                                 */
        SET_HOR_MARGINS,          /*   Set horizontal margins                */
       'Y',0,                     /*   ESC Y                                 */
        GRAPHICS           ,      /*   960 Bit-Image Graphics                */
       'Z',0,                     /*   ESC Z                                 */
        GRAPHICS           ,      /*   1920 Bit-Image Graphics               */
       '3',0,                     /*   ESC 3                                 */
        IGNORE_ESC_n       ,      /*   Graphics Line Spacing                 */
       '5',0,                     /*   ESC 5                                 */
        IGNORE_ESC_n       ,      /*   Automatic Line Feed                   */
       '6',0x80+31,               /*   ESC 6                                 */
        ESC_SINGLE         ,      /*   Select Character Set 2                */
       '7',0x00+31,               /*   ESC 7                                 */
        ESC_SINGLE         ,      /*   Select Character Set 1                */
       '_',0x80+21,               /*   ESC _                                 */
        ESC_SINGLE_0_1     ,      /*   Continuous Overscore                  */
       '-',0x80+26,               /*   ESC -                                 */
        ESC_SINGLE_0_1     ,      /*   Continuous Underscore                 */
       ':',0x80+0x40+2,           /*   ESC :                                 */
        ESC_SINGLE         ,      /*   12 Characters per Inch Printing       */
       '=',0,                     /*   ESC =                                 */
        DOWNLOAD_PROPRINT  ,      /*   Character Font Image Download         */
       14, 0X80+0X40+4,           /*   ESC #14                               */
        ESC_SINGLE         ,      /*   Double Width By Line                  */
       15, 0X80+0X40+1,           /*   ESC #15                               */
        ESC_SINGLE         ,      /*   Double Width By Line                  */
        '^',0X80+0    ,           /*   ESC ^ (Must be Index 0)               */
        PRT_NEXT           ,      /*   Print Single Character from chart     */
        '\\',0X80+0,              /*   ESC \ (Must be Index 0)               */
        PRT_ALL            ,      /*   Print Continuously from all chart     */
        '[',0,                    /*                                         */
        HANDLE_ESC_BRKT    ,      /*                                         */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
                                  /*******************************************/
     },

/* WIDTH_Elements WIDTH_TABLE[16] = */
     {                         /******************************************/
       5,0x80+23,              /*   0    ESC W                           */
      17,0x80+25,              /*   1    ESC , SI                       */
      12,0x80+24,              /*   2    ESC :                           */
       0,0      ,              /*   3    User programmable location      */
       5,0x80+22,              /*   4    ESC , LF, VT, FF, CR, SO, DC4  */
      12,0x80+24,              /*   5    DC2                             */
      17,0x80+25,              /*   6    DC2                             */
      0,0       ,              /*   7    User programmable location      */
      0,0       ,              /*   8    User programmable location      */
      0,0       ,              /*   9    User programmable location      */
      0,0       ,              /*  10    User programmable location      */
      0,0       ,              /*  11    User programmable location      */
      0,0       ,              /*  12    User programmable location      */
      0,0       ,              /*  13    User programmable location      */
      0,0       ,              /*  14    User programmable location      */
      0,0                      /*  15    User programmable location      */
                               /******************************************/
     },

/* unsigned long  SPECIAL_TABLE[16] = */
     {                               /***************************************/
          0x18000000L,               /*   0    ESC T                        */
              0      ,               /*   1    User programmable location   */
              0      ,               /*   2    User programmable location   */
              0      ,               /*   3    User programmable location   */
              0      ,               /*   4    User programmable location   */
              0      ,               /*   5    User programmable location   */
              0      ,               /*   6    User programmable location   */
              0      ,               /*   7    User programmable location   */
              0      ,               /*   8    User programmable location   */
              0      ,               /*   9    User programmable location   */
              0      ,               /*  10    User programmable location   */
              0      ,               /*  11    User programmable location   */
              0      ,               /*  12    User programmable location   */
              0      ,               /*  13    User programmable location   */
              0      ,               /*  14    User programmable location   */
              0                      /*  15    User programmable location   */
                                     /***************************************/
        },
/* CODE_ON_TABLE_Elements  CODE_ON_TAB[32] = */
        {                          /*************************************/
        0, 2, 27,'6',0,0,0,0,0,    /*  31 - Sel Char Set 2              */
        0, 2, 27,'G',0,0,0,0,0,    /*  30 - Double Strike on            */
        0, 2, 27,'E',0,0,0,0,0,    /*  29 - Emphasis on                 */
        0, 3, 27,'S',1,0,0,0,0,    /*  28 - Subscript on                */
        0, 3, 27,'S',0,0,0,0,0,    /*  27 - Superscript on              */
        0, 3, 27,'-',1,0,0,0,0,    /*  26 - Underscore on               */
       17, 1, 15,  0,0,0,0,0,0,    /*  25 - Condense mode on            */
       12, 2, 27,':',0,0,0,0,0,    /*  24 - 12 Pitch on                 */
        5, 3, 27,'W',1,0,0,0,0,    /*  23 - DW Continuous on            */
        5, 1, 14,  0,0,0,0,0,0,    /*  22 - Temp DW on                  */
        0, 3, 27,'_',1,0,0,0,0,    /*  21 - Overscore on                */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  19 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  18 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  17 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  16 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  15 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  14 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  13 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  12 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  11 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  10 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   9 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   8 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   7 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   6 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   5 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   4 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   3 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   2 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   1 User programmable location    */
        0, 2, 27,'^',0,0,0,0,0     /*   0 Print Next Char.  (Must       */
                                   /*     be Index 0, do not Change)    */
                                   /*************************************/
        },

/* CODE_OFF_TABLE_Elements  CODE_OFF_TAB[32] = */

        {                          /***************************************/
         0, 2, 27,'7',0,0,0,0,0,   /*   31 - Sel Char Set 1               */
         0, 2, 27,'H',0,0,0,0,0,   /*   30 - Double Strike off            */
         0, 2, 27,'F',0,0,0,0,0,   /*   29 - Emphasis off                 */
         0, 2, 27,'T',0,0,0,0,0,   /*   28 - Subscript off                */
         0, 2, 27,'T',0,0,0,0,0,   /*   27 - Superscript off              */
         0, 3, 27,'-',0,0,0,0,0,   /*   26 - Underscore off               */
       119, 1, 18,  0,0,0,0,0,0,   /*   25 - Condense mode off (10 Pitch) */
       170, 1, 18,  0,0,0,0,0,0,   /*   24 - 12 Pitch off (10 Pitch)      */
       102, 3, 27,'W',0,0,0,0,0,   /*   23 - DW Continuous off            */
       102, 1, 20,  0,0,0,0,0,0,   /*   22 - Temp DW off                  */
         0, 3, 27,'_',0,0,0,0,0,   /*   21 - Overscore off                */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   19 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   18 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   17 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   16 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   15 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   14 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   13 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   12 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   11 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   10 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    9 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    8 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    7 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    6 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    5 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    4 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    3 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    2 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    1 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*    0 Reserved                       */
                                   /***************************************/
        },
/* unsigned short PSM_TABLE_ORG_1046 [256] = */
       {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
/* unsigned short PSM_TABLE_1046 [256] = */
       {
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
}
