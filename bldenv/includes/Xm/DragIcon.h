/* @(#)02	1.2  src/gos/2d/MOTIF/lib/Xm/DragIcon.h, libxm, gos41J, 9508A 2/14/95 14:52:25 */
/*
 *   COMPONENT_NAME: LIBXM      Motif Toolkit
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 92, 118
 *
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: DragIcon.h,v $ $Revision: 1.20 $ $Date: 94/07/21 14:11:41 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmDragIcon_h
#define _XmDragIcon_h

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif


#define XmIsDragIconObjectClass(w) (XtIsSubclass(w, xmDragIconObjectClass))

enum {
	XmATTACH_NORTH_WEST,
	XmATTACH_NORTH,
	XmATTACH_NORTH_EAST,
	XmATTACH_EAST,
	XmATTACH_SOUTH_EAST,
	XmATTACH_SOUTH,
	XmATTACH_SOUTH_WEST,
	XmATTACH_WEST,
	XmATTACH_CENTER,
	XmATTACH_HOT
};

typedef struct _XmDragIconRec *XmDragIconObject;
typedef struct _XmDragIconClassRec *XmDragIconObjectClass;
externalref WidgetClass xmDragIconObjectClass;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget XmCreateDragIcon() ;

#else

extern Widget XmCreateDragIcon( 
                        Widget parent,
                        String name,
                        ArgList argList,
                        Cardinal argCount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmDragIcon_h */
