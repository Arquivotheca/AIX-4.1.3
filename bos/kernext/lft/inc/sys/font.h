/* @(#)09	1.8  src/bos/kernext/lft/inc/sys/font.h, lftdd, bos411, 9428A410j 7/5/94 11:30:26 */
/*
 *
 * COMPONENT_NAME: LFTDD
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef _L_FONT
#define _L_FONT


#ifndef XTRN
#   define XTRN extern
#endif

struct font_data {
           ulong  font_id;          /* font_data index (0 based)   */
           char   font_name[16];    /* filename for now            */
           char   font_weight[8];   /* boldness                    */
           char   font_slant[8];    /* Roman, italic etc.          */
           char   font_page[8];     /* code page this font renders */
           ulong  font_style;       /* 0=apa,                      */
           long   font_width;       /* charbox width  in pels      */
           long   font_height;      /* charbox height in pels      */
           long   *font_ptr;        /* pointer to rtfont file      */
           ulong  font_size;        /* size of font structure      */
};

XTRN struct font_data fonts[32];

#endif /* _L_FONT */
