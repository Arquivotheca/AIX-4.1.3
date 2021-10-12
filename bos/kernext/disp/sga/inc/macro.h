/* @(#)27       1.4  src/bos/kernext/disp/sga/inc/macro.h, sgadd, bos411, 9428A410j 10/29/93 10:45:00 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
 *
 *   FUNCTIONS: SET_CHAR_ATTRIB
 *		SELECT_NEW_FONT
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

#ifndef _SGA_MACROS
#define _SGA_MACROS

#include "sgamisc.h"
/*************************************************************
*  Sets up a change in attributes by:                        *
*     - loading the ld->fgcol and ld->bgcol variables with   *
*       the new foreground and background colors             *
*     - For 1 Bit/Pel mode, the hardware palette positions   *
*       1 and 0 are set to fgcol and bgcol for Normal Video  *
*       0 and 1 are set to fgcol and bgcol for ReverseVideo  *
*       This way colex() is not affected                     *
*     - A reverse video flag is set for clear_lines()        *
*                                                            *
*                                                            *
*************************************************************/
#define SET_CHAR_ATTRIB(ld,new,tmp)                          \
{                                                            \
  SELECT_NEW_FONT(ld,new,tmp);                               \
  SELECT_NEW_BGCOL(ld,new);                                  \
  SELECT_NEW_FGCOL(ld,new);                                  \
  ld->Vttenv.current_attr=new;                               \
  if (ld->bpp == BitsPP1)                                    \
  { if ((new & 0x02) == 0x02)                                \
    { /* REVERSE VIDEO */                                    \
      /* used in clear_lines() and colex() */                \
      ld->reverse_video = TRUE;                              \
    }                                                        \
    else                                                     \
    { /* NORMAL  VIDEO */                                    \
      /* used in clear_lines() and colex() */                \
      ld->reverse_video = FALSE;                             \
    }                                                        \
  }                                                          \
}                                                         

#define SELECT_NEW_FGCOL(ld,char_attr)	              	     \
 ld->colset = FALSE;                                         \
 ld->fgcol = ((char_attr << 16) >> 28);

#define SELECT_NEW_BGCOL(ld,char_attr)		             \
 ld->colset = FALSE;                                         \
 ld->old_bgcol = ld->bgcol;			             \
 ld->bgcol = ((char_attr << 20) >> 28);

#define SELECT_NEW_FONT(ld,char_attr,tmp)                    \
 tmp = ((char_attr << 24) >> 29);	                     \
 tmp = tmp & 0x00000007;                                     \
 if (tmp != ld->cur_font)                                    \
 {	                                                     \
   load_font(vp,tmp);                                        \
   ld->cur_font = tmp;	                                     \
 }

#endif /* _SGA_MACROS */
