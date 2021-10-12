/* @(#)63	1.1  src/gos/2d/MOTIF/lib/Xm/MenuShell.h, libxm, gos411, 9428A410i 11/10/92 16:44:14 */

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
/*   $RCSfile: MenuShell.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:38:36 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMenuShell_h
#define _XmMenuShell_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmMenuShellWidgetClass;

typedef struct _XmMenuShellClassRec       * XmMenuShellWidgetClass;
typedef struct _XmMenuShellWidgetRec      * XmMenuShellWidget;

#ifndef XmIsMenuShell
#define XmIsMenuShell(w) XtIsSubclass(w, xmMenuShellWidgetClass)
#endif /* XmIsMenuShell */


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateMenuShell() ;

#else

extern Widget XmCreateMenuShell( 
                        Widget parent,
                        char *name,
                        ArgList al,
                        Cardinal ac) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMenuShell_h */
/* DON'T ADD STUFF AFTER THIS #endif */
