/* @(#)46	1.2  src/gos/2d/XTOP/lib/Xt/RectObj.h, xlib, gos411, 9428A410i 2/10/94 15:30:25 */
/*
 *   COMPONENT_NAME: XTOOLKIT
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 16,27,
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1992, 1994.
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* $XConsortium: RectObj.h,v 1.8 89/09/11 17:39:45 swick Exp $ */
/* $oHeader: RectObj.h,v 1.2 88/08/18 17:39:17 asente Exp $ */
/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#ifndef _XtRect_h
#define _XtRect_h

typedef struct _RectObjRec *RectObj;
typedef struct _RectObjClassRec *RectObjClass;

#ifndef RECTOBJ
externalref WidgetClass rectObjClass;
#endif
#endif /* _XtRect_h */
/* DON'T ADD STUFF AFTER THIS #endif */
