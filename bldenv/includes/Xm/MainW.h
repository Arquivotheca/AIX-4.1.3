/* @(#)56	1.2  src/gos/2d/MOTIF/lib/Xm/MainW.h, libxm, gos411, 9428A410i 3/5/93 11:36:11 */

/*
 *   COMPONENT_NAME: LIBXM	Motif Toolkit
 *
 *   FUNCTIONS: 
 *
 *   ORIGINS: 73
 *
 */

/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: MainW.h,v $ $Revision: 1.8.2.2 $ $Date: 92/11/12 17:39:27 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMainWindow_h
#define _XmMainWindow_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsMainWindow
#define XmIsMainWindow(w)	XtIsSubclass(w, xmMainWindowWidgetClass)
#endif /* XmIsMainWindow */

externalref WidgetClass xmMainWindowWidgetClass;

typedef struct _XmMainWindowClassRec * XmMainWindowWidgetClass;
typedef struct _XmMainWindowRec      * XmMainWindowWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void XmMainWindowSetAreas() ;
extern Widget XmMainWindowSep1() ;
extern Widget XmMainWindowSep2() ;
extern Widget XmMainWindowSep3() ;
extern Widget XmCreateMainWindow() ;

#else

extern void XmMainWindowSetAreas( 
                        Widget w,
                        Widget menu,
                        Widget command,
                        Widget hscroll,
                        Widget vscroll,
                        Widget wregion) ;
extern Widget XmMainWindowSep1( 
                        Widget w) ;
extern Widget XmMainWindowSep2( 
                        Widget w) ;
extern Widget XmMainWindowSep3( 
                        Widget w) ;
extern Widget XmCreateMainWindow( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMainWindow_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
