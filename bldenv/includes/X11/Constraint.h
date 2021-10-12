/* @(#)07	1.2  src/gos/2d/XTOP/lib/Xt/Constraint.h, xlib, gos411, 9428A410i 2/10/94 15:06:14 */
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

/* $XConsortium: Constraint.h,v 1.9 89/06/16 18:08:57 jim Exp $ */
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

#ifndef _XtConstraint_h
#define _XtConstraint_h

typedef struct _ConstraintClassRec *ConstraintWidgetClass;

#ifndef CONSTRAINT
externalref WidgetClass constraintWidgetClass;
#endif

#endif /* _XtConstraint_h */
/* DON'T ADD STUFF AFTER THIS #endif */
