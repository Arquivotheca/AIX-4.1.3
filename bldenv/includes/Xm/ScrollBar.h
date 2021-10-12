/* @(#)10	1.1  src/gos/2d/MOTIF/lib/Xm/ScrollBar.h, libxm, gos411, 9428A410i 11/10/92 16:52:37 */

/*
 *   COMPONENT_NAME: LIBXM	Motif Toolkit
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 73
 *
 */

/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: ScrollBar.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:45:14 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScrollBar_h
#define _XmScrollBar_h


#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


/*  ScrollBar Widget  */

externalref WidgetClass xmScrollBarWidgetClass;

typedef struct _XmScrollBarClassRec * XmScrollBarWidgetClass;
typedef struct _XmScrollBarRec      * XmScrollBarWidget;

/* ifndef for Fast Subclassing  */

#ifndef XmIsScrollBar
#define XmIsScrollBar(w)	XtIsSubclass(w, xmScrollBarWidgetClass)
#endif  /* XmIsScrollBar */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateScrollBar() ;
extern void XmScrollBarGetValues() ;
extern void XmScrollBarSetValues() ;

#else

extern Widget XmCreateScrollBar( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;
extern void XmScrollBarGetValues( 
                        Widget w,
                        int *value,
                        int *slider_size,
                        int *increment,
                        int *page_increment) ;
extern void XmScrollBarSetValues( 
                        Widget w,
                        int value,
                        int slider_size,
                        int increment,
                        int page_increment,
#if NeedWidePrototypes
                        int notify) ;
#else
                        Boolean notify) ;
#endif /* NeedWidePrototypes */

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScrollBar_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
