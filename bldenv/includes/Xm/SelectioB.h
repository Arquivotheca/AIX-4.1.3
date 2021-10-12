/* @(#)16	1.1  src/gos/2d/MOTIF/lib/Xm/SelectioB.h, libxm, gos411, 9428A410i 11/10/92 16:53:24 */

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
/*   $RCSfile: SelectioB.h,v $ $Revision: 1.7 $ $Date: 92/03/13 16:46:06 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmSelectionBox_h
#define _XmSelectionBox_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass xmSelectionBoxWidgetClass;

typedef struct _XmSelectionBoxClassRec * XmSelectionBoxWidgetClass;
typedef struct _XmSelectionBoxRec      * XmSelectionBoxWidget;


#ifndef XmIsSelectionBox
#define XmIsSelectionBox(w)  (XtIsSubclass (w, xmSelectionBoxWidgetClass))
#endif



/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmSelectionBoxGetChild() ;
extern Widget XmCreateSelectionBox() ;
extern Widget XmCreateSelectionDialog() ;
extern Widget XmCreatePromptDialog() ;

#else

extern Widget XmSelectionBoxGetChild( 
                        Widget sb,
#if NeedWidePrototypes
                        unsigned int which) ;
#else
                        unsigned char which) ;
#endif /* NeedWidePrototypes */
extern Widget XmCreateSelectionBox( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;
extern Widget XmCreateSelectionDialog( 
                        Widget ds_p,
                        String name,
                        ArgList sb_args,
                        Cardinal sb_n) ;
extern Widget XmCreatePromptDialog( 
                        Widget ds_p,
                        String name,
                        ArgList sb_args,
                        Cardinal sb_n) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmSelectionBox_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
