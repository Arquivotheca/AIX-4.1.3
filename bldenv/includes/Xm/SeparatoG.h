/* @(#)19	1.1  src/gos/2d/MOTIF/lib/Xm/SeparatoG.h, libxm, gos411, 9428A410i 11/10/92 16:53:43 */

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
/*   $RCSfile: SeparatoG.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:46:24 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/*  Separator Gadget  */
#ifndef _XmSeparatorGadget_h
#define _XmSeparatorGadget_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsSeparatorGadget
#define XmIsSeparatorGadget(w) XtIsSubclass(w, xmSeparatorGadgetClass)
#endif /* XmIsSeparator */

externalref WidgetClass xmSeparatorGadgetClass;

typedef struct _XmSeparatorGadgetClassRec * XmSeparatorGadgetClass;
typedef struct _XmSeparatorGadgetRec      * XmSeparatorGadget;
typedef struct _XmSeparatorGCacheObjRec   * XmSeparatorGCacheObject;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateSeparatorGadget() ;

#else

extern Widget XmCreateSeparatorGadget( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSeparatorGadget_h */
/* DON'T ADD STUFF AFTER THIS #endif */
