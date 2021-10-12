/* @(#)11	1.1  src/bos/usr/bin/bprt/5202.h, libbidi, bos411, 9428A410j 8/27/93 09:56:34 */
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
/* char            PrinterName[14]   = */ { "5202.CFG" },
/* unsigned short  PRT_CHAR2_ATTR1   = */   0x8000 ,
/* unsigned short  PRT_LAST          = */   80     ,
/* unsigned short  PRT_DFLT_LENGTH   = */   16320  ,
/* unsigned short  PRT_L_MARGIN      = */   0      ,
/* unsigned short  PRT_R_MARGIN      = */   80     ,
/* unsigned char   PRT_D_PITCH       = */   10     ,
/* unsigned short  PRT_OLD_CWIDTH    = */   204    ,
/* unsigned char   PRT_SPACE_FOR[11] = */   {17,0x80+0x40+4,5,27,
                                             100,0,0,0,0,0,0},
/* unsigned short  PRT_SPEC_PITCH    = */   119    ,
/* unsigned short  RESERVED[2]       = */   {0,0}  ,
/* unsigned char   EXPANSION_CODE    = */   1      ,
/* unsigned char   TAB_FLAG          = */   3,
/* unsigned char   PRT_HOME_HEAD[8]  = */   {1,13,0,0,0,0,0,0},
/* unsigned short  PRT_MIN_PNTER     = */   0,
/* unsigned short  PRT_MIN_CWIDTH    = */   119,
/* unsigned short  PRT_MIN_PITCH     = */   17,
/* unsigned short  PRT_HT_ATTR_1     = */   0xFBFF,
/* unsigned short  PRT_HT_ATTR_2     = */   0xFFFF,
/* unsigned short  PRT_END_ATTR_1    = */   0X0383,
/* unsigned short  PRT_END_ATTR_2    = */   0x8000,  /* 0 */
/* unsigned char   TAIL_FLAG         = */   0x9F,
/* unsigned short  PRT_ATTR1         = */   0x8002,
/* unsigned short  PRT_ATTR2         = */   0x0000,
/* unsigned short  PRT_PSM_ATTR_1    = */   0x0001,
/* unsigned short  PRT_PSM_ATTR_2    = */   0x0000,
/* unsigned char   DESELECT_n        = */   21,
/* unsigned char   PRT_420X          = */   0,
/* unsigned char   PRT_DEF_TABS[28]  = */         /* Default Tab Stops */
                                           {8*1,8*2,8*3,8*4,8*5,8*6,8*7        ,
                                            8*8,8*9,8*10,8*11,8*12,8*13,8*14   ,
                                            8*15,8*16,0*17,0*18,0*19,0*20,0*21 ,
                                            0*22,0*23,0*24,0*25,0*26,0*27,0*28},

/* NORMAL_Elements  NORMAL_CHARS_TAB[33] = */

           {   /* 0 */   0x00  , PASS_THROUGH,
               /* 1 */   0x00  , PASS_THROUGH,
               /* 2 */   0x00  , PASS_THROUGH,
               /* 3 */   0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , PRINT_SERVICE_1,
                         0x00  , DESTRUCT_BS,
                         0x00  , HT_FOUND,
                         /*0x40+7, FLUSH_BUFFER,*/
                         0x40+7, PRINT_BUFF,
                         0x40+7, FLUSH_BUFFER,
                         0x40+7, PRINT_BUFF,
                         0x40+7, PRINT_BUFF,
                         0x40+8, SO_SI,
                         0x40+4, RESERVED_2,
                         0x00  , PASS_THROUGH,
                         0x00  , PRT_SELECT,
                         0x40+3, RESERVED_2,
                         0x00  , PASS_THROUGH,
                         0x40+7, SO_SI,
                         0x00  , SPECIAL_ONE,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , ESC_FOUND,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , PASS_THROUGH,
                         0x00  , DESTRUCT_BS
                         },

/* ESC_Elements  ESC_CHARS_TAB[45] = */
                        { 
	                       	    /*****************************************/
             '-',0x80+26,           /*                                       */
            ESC_SINGLE_0_1 ,        /*  UNDERLINE MODE                       */
            '/',0,                  /*                                       */
            IGNORE_ESC_n  ,         /*  VFU CHA SELECTION                    */
            '3',0,                  /*                                       */
            IGNORE_ESC_n ,          /*                                       */
            '5',0,                  /*                                       */
            IGNORE_ESC_n ,          /*                                       */
            '6',0x80+31 ,           /*                                       */
             ESC_SINGLE ,           /*                                       */
            '7',00+31  ,            /*                                       */
             ESC_SINGLE ,           /*                                       */
            'A',0,                  /*                                       */
             IGNORE_ESC_n,          /*  N/72 LINE SPACING                    */
            'B',0,                  /*                                       */
             ZERO_TERMINATOR,       /*  VERTICAL TAB                         */
            'C',0,                  /*  LINE LENGTH                          */
             ESC_C_FOUND,           /*                                       */
            'D',0 ,                 /*  HORIZONTAL TAB SETTING               */
             TAB_SET_FOUND ,        /*                                       */
            'E',0x80+29,            /*  EMPHASIZE ON                         */
             ESC_SINGLE,            /*                                       */
            'F',0x00+29,            /*  EMPHASIZE OFF                        */
             ESC_SINGLE,            /*                                       */
            'G',0x80+30 ,           /* DOUBLE STRIKE ON                      */
             ESC_SINGLE ,           /*                                       */
            'H',0x00+30 ,           /* DOUBLE STRIKE OFF                     */
             ESC_SINGLE ,           /*                                       */
            'I',0 ,                 /* CHANGE FONT                           */
             ESC_I        ,         /*                                       */
            'J',0 ,                 /*                                       */
             REVERSE_LF_n ,         /*                                       */
            'K',0 ,                 /*                                       */
            GRAPHICS,               /*                                       */
            'L',0,                  /*                                       */
             GRAPHICS       ,       /*                                       */
            'N',0,                  /*                                       */
            IGNORE_ESC_n   ,        /*                                       */
            'P',0x80+16,            /*                                       */
            ESC_SINGLE_0_1  ,       /* Set PSM MODE ON/OFF                   */
            'Q',0,                  /*                                       */
            PRT_DESELECT_n ,        /*  DESELECT                             */
            'R',0,                  /*                                       */
            RESET_TAB_SETTINGS,     /*                                       */
            'S',0x80+28,            /*                                       */
            ESC_SUB_SUPER   ,       /*                                       */
            'T',0x20+0,             /*                                       */
            ESC_SINGLE     ,        /*                                       */
            'W',0x40+1,             /*                                       */
            ESC_SUB_SUPER   ,       /*  DOUBLE WIDTH ON/OFF                  */
            'X',0,                  /*                                       */
            SET_HOR_MARGINS ,       /*  LEFT/RIGHT MARGIN SETTINGS           */
            'Y',0,                  /*                                       */
            GRAPHICS        ,       /*                                       */
            'Z',0,                  /*                                       */
            GRAPHICS        ,       /*                                       */
            'd',0,                  /*                                       */
            SPACE_FOR_BAK   ,       /* SPACE FORWARD                         */
            'e',0,                  /*                                       */
            SPACE_FOR_BAK   ,       /* SPACE BACKWARD                        */
            'n',0 ,                 /*                                       */
            IGNORE_ESC_n    ,       /*                                       */
            '[',0,                  /*                                       */
            /*PMP ,   */    /*  CONTROL SEQUENCE FUNCTION CMDS       */
            HANDLE_ESC_BRKT ,       /*  CONTROL SEQUENCE FUNCTION CMDS       */
            ']',0,                  /*                                       */
            REVERSE_LF      ,       /*  REVERSE LINE FEED                    */
            '^',0x80+0,             /* PRINT NEXT CHARACTER                  */
            PRT_NEXT        ,       /*                                       */
            '\\',0x80+0,            /* PRINT ALL CHARACTER                   */
            PRT_ALL         ,       /*                                       */
            '=',0,                  /*                                       */
            GRAPHICS        ,       /* DOWNLOAD FONT                         */
            ':',0x40+2,             /*                                       */
            RESERVED_2      ,       /* SELECT 12 PITCH                       */
            '',0x40+8,            /*                                       */
            ESC_SINGLE      ,       /* DOUBLE WIDTH                          */
            '',0x40+4,            /*                                       */
            RESERVED_2      ,       /* CONDENSED PRINTING                    */
            0,0,                    /*                                       */
            PASS_THROUGH    ,       /*                                       */
            0,0,                    /*                                       */
            PASS_THROUGH    ,       /*                                       */
           0,0,                     /*                                       */
           PASS_THROUGH     ,       /*                                       */
            0,0,                    /*                                       */
           PASS_THROUGH     ,       /*                                       */
            0,0,                    /*                                       */
           PASS_THROUGH     ,       /*                                       */
           0,0,                     /*                                       */
           PASS_THROUGH     ,       /*                                       */
                                    /*                                       */
           },                       /*     PITCH,ATTR                        */

/* WIDTH_Elements WIDTH_TABLE[16] = */

   {                                   /*************************************/
      5,0x00+23,                       /* 0 - DW CONTINUOUS OFF             */
      5,0x80+23,                       /* 1 - DW CONTINUOUS ON              */
      12,0x80+24,                      /* 2 - 12 PITCH ON                   */
      17,0X00+25,                      /* 3 - 17 PITCH OFF                  */
      10,0x00+16,                      /* 4 - PSM OFF                       */
      12,0x00+24,                      /* 5 - 12 PITCH OFF                  */
      17,0x80+25,                      /* 6 - 17 PITCH ON                   */
      5,0x00+22,                       /* TEMP DW OFF                       */
      5,0x80+22,                       /* TEMP DW ON                        */
      0,0,                             /* 9                                 */
      0,0,                             /* 10                                */
      0,0,                             /* 11                                */
      0,0,                             /* 12                                */
      0,0,                             /* 13                                */
      0,0,                             /* 14                                */
      0,0,                             /* 15                                */
     },                                /*************************************/
/* unsigned long  SPECIAL_TABLE[16] = */

   {                                /***************************************/
          0x18000000,               /*   0    ESC T                        */
          0,                        /*   1    User programmable location   */
          0,                        /*   2    User programmable location   */
          0,                        /*   3    User programmable location   */
          0,                        /*   4    User programmable location   */
          0,                        /*   5    User programmable location   */
          0,                        /*   6    User programmable location   */
          0,                        /*   7    User programmable location   */
          0,                        /*   8    User programmable location   */
          0,                        /*   9    User programmable location   */
          0,                        /*  10    User programmable location   */
          0,                        /*  11    User programmable location   */
          0,                        /*  12    User programmable location   */
          0,                        /*  13    User programmable location   */
          0,                        /*  14    User programmable location   */
          0,                        /*  15    User programmable location   */
          },                        /***************************************/
/* CODE_ON_TABLE_Elements  CODE_ON_TAB[32] = */

        {                          /*************************************/
        0, 2, 27,'6',0,0,0,0,0,    /*  31 - Sel Char Set 2              */
        0, 2, 27,'G',0,0,0,0,0,    /*  30 - Double Strike on            */
        0, 2, 27,'E',0,0,0,0,0,    /*  29 - Emphasis on                 */
        0, 3, 27,'S',1,0,0,0,0,    /*  28 - Subscript on                */
        0, 3, 27,'S',0,0,0,0,0,    /*  27 - Superscript on              */
        0, 3, 27,'-',1,0,0,0,0,    /*  26 - Underscore on               */
       17, 1, 15,  0,0,0,0,0,0,    /*  25 - 17 PITCH ON                 */
       12, 2, 27,':',0,0,0,0,0,    /*  24 - 12 Pitch on                 */
        5, 3, 27,'W',1,0,0,0,0,    /*  23 - DW Continuous on            */
        5, 1, 14,  0,0,0,0,0,0,    /*  22 - Temp DW on                  */
        0, 0, 0, 0, 0, 0,0,0,0,    /*  21                               */
        0, 0,  0,  0,0,0,0,0,0,    /*  20                               */
        0, 0,  0,  0,0,0,0,0,0,    /*  19                               */
        0, 0,  0,  0,0,0,0,0,0,    /*  18                               */
        0, 1,  0,  0,0,0,0,0,0,    /*  17 Special EOL SUPPORT           */
       10, 3, 27,'P',1,0,0,0,0,    /*  16 PSM on                        */
        0, 0,  0,  0,0,0,0,0,0,    /*  15                               */
        0, 0,  0,  0,0,0,0,0,0,    /*  14                               */
        0, 0,  0,  0,0,0,0,0,0,    /*  13 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  12 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  11 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*  10 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   9 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   8 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   7 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   6 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,    /*   5 User programmable location    */
        0, 0,  0,  0,0,0,0,0,0,   /*    4 User programmable location   */
        0, 0,  0,  0,0,0,0,0,0,   /*    3 User programmable location   */
        0, 0,  0,  0,0,0,0,0,0,   /*    2 User programmable location   */
        0, 0,  0,  0,0,0,0,0,0,   /*    1 User programmable location   */
        0, 2, 27,'^',0,0,0,0,0,   /*    0 PRINT NEXT CHAR              */
                                   /*************************************/
        },
/* CODE_OFF_TABLE_Elements  CODE_OFF_TAB[32] = */

     {                            /***************************************/
        0, 2, 27,'7',0,0,0,0,0,   /*   31 - Sel Char Set 1               */
        0, 2, 27,'H',0,0,0,0,0,   /*   30 - Double Strike off            */
        0, 2, 27,'F',0,0,0,0,0,   /*   29 - Emphasis off                 */
        0, 2, 27,'T',0,0,0,0,0,   /*   28 - Subscript off                */
        0, 2, 27,'T',0,0,0,0,0,   /*   27 - Superscript off              */
        0, 3, 27,'-',0,0,0,0,0,   /*   26 - Underscore off               */
      119, 1, 18,  0,0,0,0,0,0,   /*   25 - 17 PITCH OFF                 */
      170, 1, 18,  0,0,0,0,0,0,   /*   24 - 12 Pitch off (10 Pitch)      */
        0, 3, 27,'W',0,0,0,0,0,   /*   23 - DW Continuous off            */
        0, 1, 20,  0,0,0,0,0,0,   /*   22 - Temp DW off                  */
        0, 0,  0, 0, 0,0,0,0,0,   /*   21                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   20                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   19                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   18                                */
        0, 1, 18,  0,0,0,0,0,0,   /*   17 SPECIAL EOL SUPPORT            */
        0, 3, 27,'P',0,0,0,0,0,   /*   16 PSM OFF                        */
        0, 0,  0,  0,0,0,0,0,0,   /*   15                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   14                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   13                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   12                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   11                                */
        0, 0,  0,  0,0,0,0,0,0,   /*   10                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    9                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    8                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    7                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    6                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    5                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    4                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    3                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    2                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    1                                */
        0, 0,  0,  0,0,0,0,0,0,   /*    0                                */
        },                        /***************************************/

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
