/* @(#)52	1.1  src/gos/2d/MOTIF/lib/Xm/ArrowBG.h, libxm, gos411, 9428A410i 11/10/92 16:26:11 */

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
/*   $RCSfile: ArrowBG.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:22:54 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmArrowButtonGadget_h
#define _XmArrowButtonGadget_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsArrowButtonGadget
#define XmIsArrowButtonGadget(w) XtIsSubclass(w, xmArrowButtonGadgetClass)
#endif /* XmIsArrowButtonGadget */

externalref WidgetClass xmArrowButtonGadgetClass;

typedef struct _XmArrowButtonGadgetClassRec * XmArrowButtonGadgetClass;
typedef struct _XmArrowButtonGadgetRec      * XmArrowButtonGadget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateArrowButtonGadget() ;

#else

extern Widget XmCreateArrowButtonGadget( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmArrowButtonGadget_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
