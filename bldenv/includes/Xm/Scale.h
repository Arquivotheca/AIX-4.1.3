/* @(#)04	1.1  src/gos/2d/MOTIF/lib/Xm/Scale.h, libxm, gos411, 9428A410i 11/10/92 16:51:46 */

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
/*   $RCSfile: Scale.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:44:29 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmScale_h
#define _XmScale_h


#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmScaleWidgetClass;

/* fast XtIsSubclass define */
#ifndef XmIsScale
#define XmIsScale(w) XtIsSubclass (w, xmScaleWidgetClass)
#endif

typedef struct _XmScaleClassRec * XmScaleWidgetClass;
typedef struct _XmScaleRec      * XmScaleWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void XmScaleSetValue() ;
extern void XmScaleGetValue() ;
extern Widget XmCreateScale() ;

#else

extern void XmScaleSetValue( 
                        Widget w,
                        int value) ;
extern void XmScaleGetValue( 
                        Widget w,
                        int *value) ;
extern Widget XmCreateScale( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmScale_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
