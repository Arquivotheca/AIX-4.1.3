/* @(#)21       1.1  src/bos/usr/lib/piosk/incR5/fontxlfd.h, cmdpiosk, bos411, 9428A410j 7/31/92 23:30:51 */
/*
 *  COMPONENT_NAME: 
 *
 *  FUNCTIONS: 
 *
 *  ORIGINS: 16,27 
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1992
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/
/*
 * $XConsortium: fontxlfd.h,v 1.1 91/05/11 09:12:07 rws Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#ifndef _FONTXLFD_H_
#define _FONTXLFD_H_


typedef struct _FontScalable {
    int         pixel,
                point,
                x,
                y,
                width;
}           FontScalableRec, *FontScalablePtr;

extern Bool FontParseXLFDName();

#define FONT_XLFD_REPLACE_NONE	0
#define FONT_XLFD_REPLACE_STAR	1
#define FONT_XLFD_REPLACE_ZERO	2
#define FONT_XLFD_REPLACE_VALUE	3

#endif				/* _FONTXLFD_H_ */
