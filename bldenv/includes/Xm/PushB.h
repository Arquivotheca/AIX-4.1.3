/* @(#)83	1.1  src/gos/2d/MOTIF/lib/Xm/PushB.h, libxm, gos411, 9428A410i 11/10/92 16:47:55 */

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
/*   $RCSfile: PushB.h,v $ $Revision: 1.8 $ $Date: 92/03/13 16:40:59 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */
/*
*  (c) Copyright 1988 MICROSOFT CORPORATION */
/***********************************************************************
 *
 * PushButton Widget
 *
 ***********************************************************************/

#ifndef _XmPButton_h
#define _XmPButton_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsPushButton
#define XmIsPushButton(w) XtIsSubclass(w, xmPushButtonWidgetClass)
#endif /* XmIsPushButton */

/* PushButton Widget */

externalref WidgetClass xmPushButtonWidgetClass;

typedef struct _XmPushButtonClassRec *XmPushButtonWidgetClass;
typedef struct _XmPushButtonRec      *XmPushButtonWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreatePushButton() ;

#else

extern Widget XmCreatePushButton( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmPButton_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
