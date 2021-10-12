/* @(#)75	1.1  src/gos/2d/MOTIF/lib/Xm/PanedW.h, libxm, gos411, 9428A410i 11/10/92 16:46:17 */

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
/*   $RCSfile: PanedW.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:40:05 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/****************************************************************
 *
 * Vertical Paned Widget (SubClass of CompositeClass)
 *
 ****************************************************************/
#ifndef _XmPanedW_h
#define _XmPanedW_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constant */
externalref WidgetClass xmPanedWindowWidgetClass;

#ifndef XmIsPanedWindow
#define XmIsPanedWindow(w)	XtIsSubclass(w, xmPanedWindowWidgetClass)
#endif /* XmIsPanedWindow */

typedef struct _XmPanedWindowClassRec  *XmPanedWindowWidgetClass;
typedef struct _XmPanedWindowRec	*XmPanedWindowWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreatePanedWindow() ;

#else

extern Widget XmCreatePanedWindow( 
                        Widget parent,
                        char *name,
                        ArgList args,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPanedWindow_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
