/* @(#)50	1.1  src/gos/2d/MOTIF/lib/Xm/ArrowB.h, libxm, gos411, 9428A410i 11/10/92 16:25:57 */

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
/*   $RCSfile: ArrowB.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:22:45 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowButton_h
#define _XmArrowButton_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsArrowButton
#define XmIsArrowButton(w) XtIsSubclass(w, xmArrowButtonWidgetClass)
#endif /* XmIsArrowButton */

externalref WidgetClass xmArrowButtonWidgetClass;

typedef struct _XmArrowButtonClassRec * XmArrowButtonWidgetClass;
typedef struct _XmArrowButtonRec      * XmArrowButtonWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateArrowButton() ;

#else

extern Widget XmCreateArrowButton( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmArrowButton_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
