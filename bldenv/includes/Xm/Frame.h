/* @(#)34	1.1  src/gos/2d/MOTIF/lib/Xm/Frame.h, libxm, gos411, 9428A410i 11/10/92 16:39:10 */

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
/*   $RCSfile: Frame.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:34:10 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmFrame_h
#define _XmFrame_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XmIsFrame
#define XmIsFrame(w) XtIsSubclass(w, xmFrameWidgetClass)
#endif /* XmIsFrame */

/* Class record constants */

externalref WidgetClass xmFrameWidgetClass;

typedef struct _XmFrameClassRec * XmFrameWidgetClass;
typedef struct _XmFrameRec      * XmFrameWidget;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateFrame() ;

#else

extern Widget XmCreateFrame( 
                        Widget parent,
                        char *name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmFrame_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
