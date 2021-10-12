/* @(#)17	1.1  src/bos/usr/bin/bprt/bdprtdos.h, libbidi, bos411, 9428A410j 8/27/93 09:56:48 */
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
/* #pragma pack(1)  check */
/***************************************************************************/
/*                           BDPRTDOS.H                                    */
/*                Bidirectional Printer module ( AIX version )             */
/*                           Header File                                   */
/***************************************************************************/

typedef struct BidirectionalInterface
       {
         char *            in_buff;      /* pointer to buffere               */
         unsigned long     in_buff_len;  /* length of in_buff                */
         unsigned long     BDAtts;       /* bidi attributes                  */
         unsigned char     CFG;          /* contains printer model number    */
                                         /* 0-16                             */
         unsigned char     Mode;         /* 0=Initialize and LoadCFG         */
                                         /* 1=Process Input Data             */
	 unsigned char     PrtCodePage;  /* code page used by printer        */
         unsigned char     prt_len;
       } BDInterface;

typedef BDInterface *PBDInterface;     /* removed far before * */


/* Length of  output buffer */
   #define OBufSize     128

/* BDInterface.Orient */
   #define _LTR         0
   #define _RTL         1

/* BDInterface.Shape */
   #define _SHAPED      0
   #define _PASSTHRU    1

/* BDInterface.CFG   */
   #define _4019        0
   #define _4201        1
   #define _4202        2
   #define _4207        3
   #define _4208        4
   #define _4216        5
   #define _4234        6
   #define _5201        7
   #define _5202        8
   #define _5204        9
   #define _3812        10
   #define _4224        11
   #define MaxCFG       12

/* BDInterface.Mode */
   #define _OPEN        0
   #define _PROCESS     1

                                        /************************************/
 extern PBDInterface BDI;               /* Bidirectional Printing    -      */
                                        /*     -     Communication Area     */
                                        /************************************/

 extern unsigned  BidiPrint ( PBDInterface FBDI );
      /*************************************************/
      /* This function is the bidirectional printing   */
      /* only entry point. It uses the global variable */
      /* BDI to do its job.                            */
      /*************************************************/

/******************************************************************************/
/*               BDPRTDOS           RETURN CODES:                             */
/******************************************************************************/

        #define  RC_OK                 0
                   /****************************************/
                   /* EveryThing is OK, Normal Termination */
                   /****************************************/
        #define  RC_UNKNOWN_COMMAND    1
                   /****************************************/
                   /* Invalid BDI->Mode was received       */
                   /****************************************/
        #define  RC_CFG_NOT_FOUND      2
                   /****************************************/
                   /* Invalid BDI->CFG was received        */
                   /****************************************/
        #define  RC_CONTINUE           3
                   /****************************************/
                   /* Returning for buffer flush ONLY.     */
                   /****************************************/


