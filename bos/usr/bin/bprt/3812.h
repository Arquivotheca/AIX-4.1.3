/* @(#)01	1.1  src/bos/usr/bin/bprt/3812.h, libbidi, bos411, 9428A410j 8/27/93 09:56:05 */
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
/* char            PrinterName[14]   = */ { "3812.CFG" },
/* unsigned short  PRT_CHAR2_ATTR1   = */   0x8000 ,
/* unsigned short  PRT_LAST          = */   80     ,
/* unsigned short  PRT_DFLT_LENGTH   = */   16320  ,
/* unsigned short  PRT_L_MARGIN      = */   0      ,
/* unsigned short  PRT_R_MARGIN      = */   80     ,
/* unsigned char   PRT_D_PITCH       = */   10     ,
/* unsigned short  PRT_OLD_CWIDTH    = */   204    ,
/* unsigned char   PRT_SPACE_FOR[11] = */   {0x80+0x40,17,8,9,27,91,
                                             67,3,0xE2,0,0},
/* unsigned short  PRT_SPEC_PITCH    = */   119    ,
/* unsigned short  RESERVED[2]       = */   {255,306}  ,
/* unsigned char   EXPANSION_CODE    = */   0      ,
/* unsigned char   TAB_FLAG          = */   3,
/* unsigned char   PRT_HOME_HEAD[8]  = */   {0,0,0,0,0,0,0,0},
/* unsigned short  PRT_MIN_PNTER     = */   6*9,
/* unsigned short  PRT_MIN_CWIDTH    = */   119,
/* unsigned short  PRT_MIN_PITCH     = */   17,
/* unsigned short  PRT_HT_ATTR_1     = */   0xFBFF,
/* unsigned short  PRT_HT_ATTR_2     = */   0xFFFF,
/* unsigned short  PRT_END_ATTR_1    = */   0x2702,
/* unsigned short  PRT_END_ATTR_2    = */   0,
/* unsigned char   TAIL_FLAG         = */   0x9F,
/* unsigned short  PRT_ATTR1         = */   0x8000,
/* unsigned short  PRT_ATTR2         = */   0x0000,
/* unsigned short  PRT_PSM_ATTR_1    = */   0x0003,
/* unsigned short  PRT_PSM_ATTR_2    = */   0x0000,
/* unsigned char   DESELECT_n        = */   1,
/* unsigned char   PRT_420X          = */   1,
/* unsigned char   PRT_DEF_TABS[28]  = */         /* Default Tab Stops */
                                           {8*1,8*2,8*3,8*4,8*5,8*6,8*7        ,
                                            8*8,8*9,8*10,8*11,8*12,8*13,8*14   ,
                                            8*15,8*16,0*17,0*18,0*19,0*20,0*21 ,
                                            0*22,0*23,0*24,0*25,0*26,0*27,0*28},

/*   NORMAL_CHARS_TAB[33] =     */
                  {      0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0X00  , SPECIAL_ONE,
                         0X00  , SPECIAL_ONE,
             /* 5 */     0X00  , SPECIAL_ONE,
                         0X00  , SPECIAL_ONE,
                         0X00  , PRINT_SERVICE_1,
                         0X00  , DESTRUCT_BS,
                         0X00  , HT_FOUND,
           /* 10 */     0X00+17, FLUSH_BUFFER,
                        0X00+17, FLUSH_BUFFER,
                        0X00+17, PRINT_BUFF,
                        0X00+17, PRINT_BUFF,
                        0x80+17, SO_SI,
        /* 15 */    0X80+0X40+1, SO_SI,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X40+1, SO_SI,
                         0X00  , PASS_THROUGH,
                         0X00+17, SO_SI,
                         0X00  , SPECIAL_ONE,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , ESC_FOUND,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         0X00  , PASS_THROUGH,
                         },

/* ESC_CHARS_TAB[45] = */
      {                           /*******************************************/
       '-',0x80+26,               /*   ESC -                                 */
        ESC_SINGLE_0_1     ,      /*   Continuous Underscore                 */
       '3',0,                     /*   ESC 3                                 */
        IGNORE_ESC_n       ,      /*   Graphics Line Spacing                 */
       'A',0,                     /*   ESC A                                 */
       IGNORE_ESC_n        ,      /*   N/72 Line Spacing                     */
       'B',0,                     /*   ESC B                                 */
        ZERO_TERMINATOR    ,      /*   Vertical Tab Settings                 */
       'C',0,                     /*   ESC C                                 */
        ESC_C_FOUND        ,      /*   Set Form Length                       */
       'D',0,                     /*   ESC D                                 */
        TAB_SET_FOUND      ,      /*   Set Horizontal Tabs                   */
       'E',0x80+0x40+3,           /*   ESC E                                 */
        ESC_SINGLE         ,      /*   select 8 pitch KATEB!                 */
       'F',0x00+0x40+3,           /*   ESC F                                 */
        ESC_SINGLE         ,      /*   select 10cpi courier                  */
       'G',0x80+0x40+3,           /*   ESC G                                 */
        ESC_SINGLE         ,      /*   select 8 pitch KATEB!                 */
       'H',0x00+0x40+3,           /*   ESC H                                 */
        ESC_SINGLE         ,      /*   select 10cpi courier                  */
       'J',0,                     /*   ESC J                                 */
        REVERSE_LF_n       ,      /*   Variable Line Feed                    */
       'N',0,                     /*   ESC N                                 */
        IGNORE_ESC_n       ,      /*   Set Automatic Perforation Skip        */
       'S',0x80+28,               /*   ESC S                                 */
        ESC_SUB_SUPER      ,      /*   Subscript or Superscript Printing     */
       'T',0x20+0,                /*   ESC T                                 */
        ESC_SINGLE         ,      /*   Cancel Subscript or Superscript       */
       'W',0x80+16,               /*   ESC W                                 */
        ESC_SINGLE_0_1     ,      /*   Continuous Double wide Printing       */
       '6',0x80+31,               /*   ESC 6                                 */
        ESC_SINGLE         ,      /*   Select Character Set 2                */
       '7',0x00+31,               /*   ESC 7                                 */
        ESC_SINGLE         ,      /*   Select Character Set 1                */
        0,0,                      /*                                         */
        PASS_THROUGH       ,      /*                                         */
        'K',0,                    /*                                         */
        GRAPHICS           ,      /*                                         */
        'L',0,                    /*                                         */
        GRAPHICS           ,      /*                                         */
        'Y',0,                    /*                                         */
        GRAPHICS           ,      /*                                         */
        'Z',0,                    /*                                         */
        GRAPHICS           ,      /*                                         */
       '[',0,                     /*                                         */
        PMP                ,      /*                                         */
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
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
        0 ,0,                     /*   Reserved location in table for        */
       PASS_THROUGH        ,      /*   additional Escape Sequence            */
                                  /*******************************************/
     },

/*  WIDTH_TABLE[16] =  */
     {                         /******************************************/
      12,0x80+16,              /*   2    ESC :                           */
      17,0x80+25,              /*   1    ESC , SI                       */
       0,0      ,              /*   3    User programmable location      */
       8,0x80+29,              /*   4    ESC , LF, VT, FF, CR, SO, DC4  */
      0,0       ,              /*   7    User programmable location      */
      0,0       ,              /*   7    User programmable location      */
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

/* SPECIAL_TABLE[16] =  */
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

/*  CODE_ON_TAB[32] =   */
        {                          /*************************************/
        0, 2, 27,'6',0,0,0,0,0,    /*  31 - Sel Char Set 2              */
        0, 2, 27,'G',0,0,0,0,0,    /*  30 - Double Strike on            */
        8, 2, 27,'E',0,0,0,0,0,    /*  29 - Emphasis on                 */
        0, 3, 27,'S',1,0,0,0,0,    /*  28 - Subscript on                */
        0, 3, 27,'S',0,0,0,0,0,    /*  27 - Superscript on              */
        0, 3, 27,'-',1,0,0,0,0,    /*  26 - Underscore on               */
       17, 1, 15,  0,0,0,0,0,0,    /*  25 - Condense mode on            */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  20 User programmable location    */
        0, 1, 14,  0,0,0,0,0,0,    /*  22 - Temp DW on                  */
        0, 3, 27,'W',1,0,0,0,0,    /*  21 - Overscore on                */
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
                                   /*************************************/
        },

/*  CODE_OFF_TAB[32] =   */

        {                          /***************************************/
         0, 2, 27,'7',0,0,0,0,0,   /*   31 - Sel Char Set 1               */
         0, 2, 27,'H',0,0,0,0,0,   /*   30 - Double Strike off            */
       225, 2, 27,'F',0,0,0,0,0,   /*   29 - Emphasis off                 */
         0, 2, 27,'T',0,0,0,0,0,   /*   28 - Subscript off                */
         0, 2, 27,'T',0,0,0,0,0,   /*   27 - Superscript off              */
         0, 3, 27,'-',0,0,0,0,0,   /*   26 - Underscore off               */
       119, 1, 18,  0,0,0,0,0,0,   /*   25 - Condense mode off (10 Pitch) */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 0,  0,  0,0,0,0,0,0,   /*   20 User programmable location     */
         0, 1, 20,  0,0,0,0,0,0,   /*   24 - 12 Pitch off (10 Pitch)      */
         0, 3, 27,'W',0,0,0,0,0,   /*   23 - DW Continuous off            */
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
                                   /***************************************/
        },
/* unsigned short PSM_TABLE_ORG_1046 [256] = */
                 {
           0,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,204,102,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,238,238,238,238,204,204,238,238,136,170,238,204,238,238,238,
           204,238,238,204,238,238,238,238,238,238,204,170,170,170,170,170,
           170,170,204,170,204,170,136,204,204,102,102,204,102,238,204,170,
           204,204,170,170,136,204,204,238,204,204,170,170,170,170,170,170,

           102,170,170,238,238,238,238,0,0,0,170,170,170,170,170,170,
           0,0,102,0,0,0,204,136,204,170,170,170,204,204,204,204,
           0,0,0,0,170,0,136,136,136,136,170,170,170,0,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,136,136,102,136,170,136,102,204,136,204,204,170,170,170,136,
           136,136,136,238,238,238,238,170,170,170,170,170,136,102,102,136,
           170,204,204,204,170,136,136,170,136,204,204,0,0,0,0,0,
           0,102,0,136,170,102,  0,204,170,170,170,136,136,136,136,170,
           },
/* unsigned short PSM_TABLE_1046 [256] = */
                 {
           0,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,204,102,170,170,170,170,170,170,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,238,238,238,238,204,204,238,238,136,170,238,204,238,238,238,
           204,238,238,204,238,238,238,238,238,238,204,170,170,170,170,170,
           170,170,204,170,204,170,136,204,204,102,102,204,102,238,204,170,
           204,204,170,170,136,204,204,238,204,204,170,170,170,170,170,170,

           102,170,170,238,238,238,238,170,238,238,170,170,170,170,170,170,
           170,170,102,170,170,170,204,136,204,170,170,170,204,170,170,170,
           238,272,238,238,170,238,136,136,136,136,170,170,170,170,170,170,
           170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,170,
           170,136,136,102,136,102,170,102,204,136,204,204,170,170,170,136,
           136,136,136,238,238,238,238,170,170,170,170,170,136,102,102,136,
           170,204,204,204,170,136,136,170,136,204,204,170,170,170,170,170,
           170,102,170,136,170,102,  0,204,170,170,170,136,136,136,136,170
           }
}
