/* @(#)38       1.3  src/bos/diag/tu/mps/mps_macros.h, tu_mps, bos411, 9440E411e 10/11/94 16:57:17 */
/*****************************************************************************
 * COMPONENT_NAME: (tu_mps)  Wildwood LAN adapter test units
 *
 * FUNCTIONS: MPS Macros Header File
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
 *****************************************************************************/
#include <stdio.h>
#ifdef MFGPATH
      #define INTERRUPT_HANDLER_PATH  "/usr/lpp/diagnostics/slih/tok32_intr"
      #define KERNEXT_PATH            "/usr/lib/drivers/dtok32_kext"
#else
      #define INTERRUPT_HANDLER_PATH  "/usr/lpp/htx/etc/kernext/mps_intr"
      #define KERNEXT_PATH            "/usr/lpp/htx/etc/kernext/mpskex"
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
#define DEBUGELSE                else
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

