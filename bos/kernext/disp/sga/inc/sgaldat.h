/* @(#)30       1.2  src/bos/kernext/disp/sga/inc/sgaldat.h, sgadd, bos411, 9428A410j 10/29/93 10:46:59 */
/*
 * COMPONENT_NAME: (SGADD) Salmon Graphics Adapter Device Driver
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


#ifndef _SGA_LDAT
#define _SGA_LDAT

/******************************************************************
*   IDENTIFICATION: SGALDAT.H                                     *
*   DESCRIPTIVE name: Virtual Display Driver (VDD) global internal*
*                     data structures for the Simple adapter      *
*                                                                 *
*   FUNCTION: Declare and initialize data structures that are     *
*             global to all Matrox VDD procedures.                *
*                                                                 *
******************************************************************/

#include "sgaenv.h"


struct sgapal
{
  short  num_entries;
  long  rgbval[16];
};


struct sga_data {

   struct sgapal ksr_color_table;

   long  Scroll_offset;  /* number of cells into the presentation */

   char volatile * sgamem;    /* Pointer to VRAM memory address */

   ushort reverse_video;    /* TRUE for reverse;False for Normal */
																											/* set in SET_ATTRIBUTES */
   ushort fgcol;           /* foreground color            */
   ushort bgcol;           /* background color            */

   aixFontInfoPtr fonthptr;           /* Pointer to font head        */
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

   long    charwd2;
   struct vttenv Vttenv;
   ulong bpp;                /* bits per pel (vram dependent) */
   uchar num_vram_modules;   /* number of vram modules on SGA */
   uchar monitor_type;       /* monitor type connected to SGA */

};

#endif /* _SGA_LDAT */
