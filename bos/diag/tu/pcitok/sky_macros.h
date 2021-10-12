/* @(#)10	1.2  src/bos/diag/tu/pcitok/sky_macros.h, tu_pcitok, bos41J, 9514A_all 3/30/95 14:04:36  */
/*
 *   COMPONENT_NAME: tu_pcitok
 *
 *   FUNCTIONS: DEBUG_0
 *		DEBUG_1
 *		DEBUG_2
 *		DEBUG_3
 *		DEBUG_4
 *		DEBUG_5
 *		DEBUG_6
 *		DEBUG_7
 *		DEBUG_8
 *		DEBUG_9
 *		SWAP16
 *		SWAP32
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*****************************************************************************
 *****************************************************************************/
#include <stdio.h>
#ifdef DIAGPATH
      #define INTERRUPT_HANDLER_PATH  "/usr/lib/lpp/diagnostics/slih/sky_intr"
#else
      #define INTERRUPT_HANDLER_PATH  "/usr/lpp/htx/etc/kernext/sky_intr"
#endif
/******************************************************************************/
/*   SWAP16, SWAP32 -                                                         */
/*   Swap bytes for 16 and 32 bit entities.                                   */
/******************************************************************************/
#define SWAP16(v)                                     \
  ((unsigned)                                         \
    (((v & 0xff00) >> 8)      |                       \
      ((v & 0x00ff) <<  8)))

#define SWAP32(v)                                     \
  ((ulong)                                            \
    (((v & 0xff000000) >> 24) |                       \
      ((v & 0x00ff0000) >>  8)  |                     \
        ((v & 0x0000ff00) <<  8)  |                   \
          ((v & 0x000000ff) << 24)))

/**********************************/
/*        DEBUGGING AIDS          */
/******************************** */
#ifdef MPS_DEBUG
#define DEBUG_0(A)               {fprintf(tucb_ptr->msg_file,A);}
#define DEBUG_1(A,B)             {fprintf(tucb_ptr->msg_file,A,B);}
#define DEBUG_2(A,B,C)           {fprintf(tucb_ptr->msg_file,A,B,C);}
#define DEBUG_3(A,B,C,D)         {fprintf(tucb_ptr->msg_file,A,B,C,D);}
#define DEBUG_4(A,B,C,D,E)       {fprintf(tucb_ptr->msg_file,A,B,C,D,E);}
#define DEBUG_5(A,B,C,D,E,F)     {fprintf(tucb_ptr->msg_file,A,B,C,D,E,F);}
#define DEBUG_6(A,B,C,D,E,F,G)   {fprintf(tucb_ptr->msg_file,A,B,C,D,E,F,G);}
#define DEBUG_7(A)        
#define DEBUG_8(A,B)     
#define DEBUG_9(A,B,C)  
#define DEBUGELSE                else
#elif DEBUG
#define DEBUG_0(A)                      {fprintf(stdout,A);fflush(stdout);}
#define DEBUG_1(A,B)                    {fprintf(stdout,A,B);fflush(stdout);}
#define DEBUG_2(A,B,C)                  {fprintf(stdout,A,B,C);fflush(stdout);}
#define DEBUG_3(A,B,C,D)                {fprintf(stdout,A,B,C,D);fflush(stdout);}
#define DEBUG_4(A,B,C,D,E)              {fprintf(stdout,A,B,C,D,E);fflush(stdout);}
#define DEBUG_5(A,B,C,D,E,F)            {fprintf(stdout,A,B,C,D,E,F);fflush(stdout);}
#define DEBUG_6(A,B,C,D,E,F,G)          {fprintf(stdout,A,B,C,D,E,F,G);fflush(stdout);}
#define DEBUG_7(A)                      {printf(A);}
#define DEBUG_8(A,B)                    {printf(A,B);}
#define DEBUG_9(A,B,C)                  {printf(A,B,C);}
#define DEBUGELSE                       else
#else
#define DEBUG_0(A)
#define DEBUG_1(A,B)
#define DEBUG_2(A,B,C)
#define DEBUG_3(A,B,C,D)
#define DEBUG_4(A,B,C,D,E)
#define DEBUG_5(A,B,C,D,E,F)
#define DEBUG_6(A,B,C,D,E,F,G)
#define DEBUG_7(A)
#define DEBUG_8(A,B)
#define DEBUG_9(A,B,C)
#define DEBUGELSE
#endif

struct attr {
        char *attribute;
        char *value;
};

