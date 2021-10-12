/* @(#)10	1.1  src/bos/usr/bin/bprt/5201.h, libbidi, bos411, 9428A410j 8/27/93 09:56:31 */
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
/* char            PrinterName[14]   = */ { "5201.CFG" },
/* unsigned short  PRT_CHAR2_ATTR1   = */   0x8000 ,
/* unsigned short  PRT_LAST          = */   96     ,
/* unsigned short  PRT_DFLT_LENGTH   = */   960    ,
/* unsigned short  PRT_L_MARGIN      = */   0      ,
/* unsigned short  PRT_R_MARGIN      = */   96     ,
/* unsigned char   PRT_D_PITCH       = */   10     ,
/* unsigned short  PRT_OLD_CWIDTH    = */   10     ,
/* unsigned char   PRT_SPACE_FOR[11] = */   {1,0x80+4,5,27,100,0,0,0,0,0,0},
/* unsigned short  PRT_SPEC_PITCH    = */   0XFFFF ,
/* unsigned short  RESERVED[2]       = */   {0,0}  ,
/* unsigned char   EXPANSION_CODE    = */   0      ,
/* unsigned char   TAB_FLAG          = */   3,
/* unsigned char   PRT_HOME_HEAD[8]  = */   {1,13,0,0,0,0,0,0},
/* unsigned short  PRT_MIN_PNTER     = */   6*9,
/* unsigned short  PRT_MIN_CWIDTH    = */   10,
/* unsigned short  PRT_MIN_PITCH     = */   10,
/* unsigned short  PRT_HT_ATTR_1     = */   0xFBDF,
/* unsigned short  PRT_HT_ATTR_2     = */   0xFFFF,
/* unsigned short  PRT_END_ATTR_1    = */   0x0380,
/* unsigned short  PRT_END_ATTR_2    = */   0,
/* unsigned char   TAIL_FLAG         = */   0x20,
/* unsigned short  PRT_ATTR1         = */   0x8001,
/* unsigned short  PRT_ATTR2         = */   0x0000,
/* unsigned short  PRT_PSM_ATTR_1    = */   0x0001,
/* unsigned short  PRT_PSM_ATTR_2    = */   0x0000,
/* unsigned char   DESELECT_n        = */   1,
/* unsigned char   PRT_420X          = */   0,
/* unsigned char   PRT_DEF_TABS[28]  = */         /* Default Tab Stops */
                                           {8*1,8*2,8*3,8*4,8*5,8*6,8*7        ,
                                            8*8,8*9,8*10,8*11,8*12,8*13,8*14   ,
                                            8*15,8*16,0*17,0*18,0*19,0*20,0*21 ,
                                            0*22,0*23,0*24,0*25,0*26,0*27,0*28},

/* NORMAL_Elements  NORMAL_CHARS_TAB[33] = */
                   {     0x00,PASS_THROUGH,
                         0x00, PASS_THROUGH,
                         0x00, PASS_THROUGH,
                         0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , SPECIAL_ONE,
                         0x00  , PRINT_SERVICE_1,
                         0x00  , DESTRUCT_BS,
                         0x00  , HT_FOUND,
                         /*0x40+4, FLUSH_BUFFER,*/
                         0x40+4, PRINT_BUFF,
                         0x40+4, FLUSH_BUFFER,
                         0x40+4, PRINT_BUFF,
                         0x40+4, PRINT_BUFF,
                         0x80+0x40+4, SO_SI,
                         0, CHANGE_LL_OUT,
                         0  , PASS_THROUGH,
                         0  , PRT_SELECT,
                         0, CHANGE_LL_IN,
                         0  , PASS_THROUGH,
                         0x40+4, SO_SI,
                         0  , SPECIAL_ONE,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , ESC_FOUND,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , PASS_THROUGH,
                         0  , DESTRUCT_BS
                         },

/* ESC_Elements  ESC_CHARS_TAB[45] = */

                       {             /*****************************************/
              '-',0x80+26,           /*                                       */
             ESC_SINGLE_0_1 ,        /*  UNDERLINE MODE                       */
             '/',0,                  /*                                       */
             IGNORE_ESC_n  ,         /*  VFU CHA SELECTION                    */
             '5',0,                  /*                                       */
             IGNORE_ESC_n ,          /*                                       */
             '6',0x80+31,            /*                                       */
             ESC_SINGLE ,            /*                                       */
             '7',0x00+31 ,           /*                                       */
              ESC_SINGLE ,           /*                                       */
             'A',0,                  /*                                       */
              IGNORE_ESC_n,          /*  N/72 LINE SPACING                    */
             'B',0,                  /*                                       */
              ZERO_TERMINATOR,       /*  VERTICAL TAB                         */
             'C',0,                  /*  LINE LENGTH                          */
              ESC_C_FOUND,           /*                                       */
             'D',0 ,                 /*  HORIZONTAL TAB SETTING               */
              TAB_SET_FOUND ,        /*                                       */
             'I',0,                  /*  CHANGE FONT                          */
              IGNORE_ESC_n,          /*                                       */
              'N',0,                 /*                                       */
              IGNORE_ESC_n,          /*                                       */
             'Q',0,                  /*                                       */
              PRT_DESELECT_n,        /*                                       */
             'R',0,                  /*                                       */
             RESET_TAB_SETTINGS,     /*                                       */
            'S',0x80+28 ,            /*                                       */
             ESC_SUB_SUPER,          /*                                       */
             'T',0x20+0,             /*                                       */
              ESC_SINGLE ,           /*                                       */
             'W',0X80+0X40+0 ,       /*                                       */
              ESC_SINGLE_0_1,        /*                                       */
             'X',0,                  /*                                       */
              SET_HOR_MARGINS,       /*                                       */
             '[',0,                  /*                                       */
             IGNORE_ESC_n_000,       /* SET VERTICAL UNITS                    */
             ']',0,                  /*                                       */
             REVERSE_LF,             /* REVERSE LINE FEED                     */
             'd',0,                  /*                                       */
              SPACE_FOR_BAK,         /*  SPACE FORWARD                        */
             'e',0,                  /*                                       */
              SPACE_FOR_BAK,         /* SPACE BACKWARD                        */
             '^',0x80+0,             /*                                       */
             PRT_NEXT ,              /* PRINT NEXT CHARACTER                  */
             '\\',0x80+0,            /* PRINT ALL CHARACTERS                  */
             PRT_ALL,                /*                                       */
             '3',0,                  /*                                       */
             IGNORE_ESC_n ,          /*                                       */
             'K',0,                  /*                                       */
             GRAPHICS,               /*                                       */
             'L',0,                  /*                                       */
             GRAPHICS        ,       /*                                       */
             'Y',0,                  /*                                       */
             GRAPHICS        ,       /*                                       */
             'Z',0,                  /*                                       */
             GRAPHICS        ,       /*                                       */
             'n',0 ,                 /*                                       */
             IGNORE_ESC_n    ,       /*                                       */
             0,0,                    /*                                       */
             PASS_THROUGH ,          /*                                       */
             0,0,                    /*                                       */
             PASS_THROUGH ,          /*                                       */
             0,0,                    /*                                       */
             PASS_THROUGH  ,         /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH      ,      /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH      ,      /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0 ,0,                   /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0,0,                    /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0,0,                    /*                                       */
             PASS_THROUGH    ,       /*                                       */
            0,0,                     /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0,0,                    /*                                       */
            PASS_THROUGH     ,       /*                                       */
             0,0,                    /*                                       */
            PASS_THROUGH     ,       /*                                       */
            0,0,                     /*                                       */
            PASS_THROUGH    ,        /*                                       */
            0,0,                     /*                                       */
            PASS_THROUGH    ,        /*                                       */
            },                       /*                                       */

/* WIDTH_Elements WIDTH_TABLE[16] = */
   {                                   /*************************************/
      5,0x80+23,                       /* 1                                 */
      0,0,                             /* 2 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 3 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 4 - ENTRY (WIDTH,ATTR)            */
      5,0x80+22,                       /* 5 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 6 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 7 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 8 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 9 - ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 10- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 11- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 12- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 13- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 14- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 15- ENTRY (WIDTH,ATTR)            */
      0,0,                             /* 16- ENTRY (WIDTH,ATTR)            */
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
        0, 2, 27,'6',0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-31  */
        0, 0,  0, 0, 0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-30  */
        0, 0,  0, 0, 0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-29  */
        0, 3, 27,'S',1,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-28  */
        0, 3, 27,'S',0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-27  */
        0, 3, 27,'-',1,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-26  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-25  */
        0, 0,  0, 0, 0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-24  */
        5, 3, 27,'W',1,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-23  */
        5, 1, 14,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-22  */
        0, 0, 0, 0, 0, 0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-21  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-20  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-19  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-18  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-17  */
        0, 0,  0, 0, 0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-16  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-15  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-14  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-13  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-12  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-11  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-10  */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-9   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-8   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-7   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-6   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-5   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-4   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-3   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-2   */
        0, 0,  0,  0,0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-1   */
        0, 2, 27,'^',0,0,0,0,0,    /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-0   */
                                   /*************************************/
        },
/* CODE_OFF_TABLE_Elements  CODE_OFF_TAB[32] = */
     {                              /*************************************/
        0, 2, 27,'7',0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-31  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-30  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-29  */
        0, 2, 27,'T',0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-28  */
        0, 2, 27,'T',0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-27  */
        0, 3, 27,'-',0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-26  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-25  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-24  */
       20, 3, 27,'W',0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-23  */
       20, 1, 20,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-22  */
        0, 0,  0, 0, 0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-21  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-20  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-19  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-18  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-17  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-16  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-15  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-14  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-13  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-12  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-11  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-10  */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-9   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-8   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-7   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-6   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-5   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-4   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-3   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-2   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-1   */
        0, 0,  0,  0,0,0,0,0,0,     /* WIDTH,#BYTES,ESC_VALUE(MAX 6)-0   */
        },                          /*************************************/

/* unsigned short PSM_TABLE_ORG_1046 [256] = */
                 {
            6,10,10,14,14,14,14,10,10,10,10,10,10,10,10,10,
           10,10, 6,10,10,10,12, 8,12,10,10,10,12,10,10,10,
           10,10,10,10,10,10,12, 6,10,10,10,10,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10,14,14,14,14,12,12,14,14, 8,10,14,12,14,14,14,
           12,14,14,12,14,14,14,14,14,14,12,10,10,10,10,10,
           10,10,12,10,12,10, 8,12,12, 6, 6,12, 6,14,12,10,
           12,12,10,10, 8,12,12,14,12,12,10,10,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10,10,14,10,10,10,10,14,14,10,10,10,10,10,10, 0,
           10,10, 8,10,10, 6,10,10, 6,12,12,12,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,12,10,14,14,14,10,
           10, 8, 8, 6, 8,10, 8, 6, 8, 8, 8, 8,10,10,10, 8,
            8, 8, 8,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10, 8, 8,10, 6, 8, 8,10, 8,12, 8,14,10,10,10, 8,
            6, 6, 8, 8, 8,12,12,10,12,12,12,10,12,12,10,10,
           },
/* unsigned short PSM_TABLE_1046 [256] = */
                 {
            0,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10,10,10,10,10,10,12, 6,10,10,10,10,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10,14,14,14,14,12,12,14,14, 8,10,14,12,14,14,14,
           12,14,14,12,14,14,14,14,14,14,12,10,10,10,10,10,
           10,10,12,10,12,10, 8,12,12, 6, 6,12, 6,14,12,10,
           12,12,10,10, 8,12,12,14,12,12,10,10,10,10,10,10,
            6,10,10,14,14,14,14,14,14,14,10,10,10,10,10,10,
           10,10,10,10,10,14,12, 8,12,10,10,10,12,12,12,12,
           14,16,16,16,10,14, 8, 8, 8, 8,10,10,10,10,10,10,
           10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
           10, 8, 8, 6, 8, 6, 8, 6,12, 8,12,12,10,10,10, 8,
            8, 8, 8,14,14,14,14,10,10,10,10,10, 8, 6, 6, 8,
           10,12,12,12,10, 8, 8,10, 8,12,12,10,10,10,10,10,
           10, 6,10, 8,10, 6, 0,12,10,10,10, 8, 8, 8, 8,10
           }
}
