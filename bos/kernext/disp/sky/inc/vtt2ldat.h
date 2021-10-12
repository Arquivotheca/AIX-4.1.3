/* @(#)14	1.12  src/bos/kernext/disp/sky/inc/vtt2ldat.h, sysxdispsky, bos411, 9428A410j 4/18/94 14:20:08 */
/*
 *   COMPONENT_NAME: SYSXDISPSKY
 *
 *   FUNCTIONS: 
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

/******************************************************************
*   IDENTIFICATION: VTT2LDAT.H                                    *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the Matrox adapter      *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all Matrox VDD procedures.                *
*                                                                 *
******************************************************************/

#include "vtt2env.h"
#include "vtt2regs.h"
#include "entdisp.h"


struct skypal
{
  short  nent;
  long  rgbval[16];
} ;


struct skyway_data {

   struct skypal ct_k;

   long  Scroll_offset;  /* number of cells into the presentation */

   struct sky_io_regs volatile * iop; /* ptr to io mapped registers   */
   struct sky_cop_regs volatile * cop; /* ptr to coprocessor registers */
   char volatile * skymem;    /* Pointer to VRAM memory address */

   ushort fgcol;           /* foreground color            */
   ushort bgcol;           /* background color            */

   aixFontInfoPtr fonthptr;            /* Pointer to font head        */
   aixCharInfoPtr fontcindex;         /* Pointer to char index       */
   aixGlyphPtr glyphptr;           /* Pointer to glyph pointer    */

   char * Font_map_start;           /* Pointer to font in memory   */
   ushort volatile *io_idata;       /* pointer to index and datab  */

   long center_x, center_y;        /* number of pels to center picture */
   long cursor_x, cursor_y;        /* values to center cursor */
   ulong old_bus;
   uchar colset,hwset,opdimset,resv; /* flags indicating cop status */
   ushort old_bgcol;           /* background color            */
   char cur_font;
   char disp_type;             /* Flag for mono or color */
   long poll_type;             /* Flag for old or new polling method */
   label_t jmpbuf;             /* jumpbuffer for exception handling */
   char    *comp_name;         /* Pointer to dds for error logging  */

   struct vttenv Vttenv;

};

