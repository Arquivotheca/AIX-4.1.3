/* @(#)72       1.2  src/gos/2d/MOTIF/lib/Xm/MwmUtil.h, libxm, gos41J, 9508A_notx 2/20/95 13:40:30 */
/*
 *   COMPONENT_NAME: LIBXM      Motif Toolkit
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 92, 118
 *
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993, 1994 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
ifndef OSF_v1_2_4
 * Motif Release 1.2
else
 * Motif Release 1.2.4
endif
*/ 
/*   $RCSfile: MwmUtil.h,v $ $Revision: 1.21 $ $Date: 94/06/06 07:35:53 $ */
/*
*  (c) Copyright 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmMwmUtil_h
#define _XmMwmUtil_h

#include <X11/Xmd.h>	/* for protocol typedefs */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Contents of the _MWM_HINTS property.
 */

typedef struct
{
#ifndef OSF_v1_2_4
    long	flags;
    long	functions;
    long	decorations;
#else /* OSF_v1_2_4 */
    /* These correspond to XmRInt resources. (VendorSE.c) */
    int	         flags;
    int		 functions;
    int		 decorations;
#endif /* OSF_v1_2_4 */
    int		input_mode;
#ifndef OSF_v1_2_4
    long	status;
#else /* OSF_v1_2_4 */
    int		 status;
#endif /* OSF_v1_2_4 */
} MotifWmHints;

typedef MotifWmHints	MwmHints;

/* bit definitions for MwmHints.flags */
#define MWM_HINTS_FUNCTIONS	(1L << 0)
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_HINTS_INPUT_MODE	(1L << 2)
#define MWM_HINTS_STATUS	(1L << 3)

/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL		(1L << 0)
#define MWM_FUNC_RESIZE		(1L << 1)
#define MWM_FUNC_MOVE		(1L << 2)
#define MWM_FUNC_MINIMIZE	(1L << 3)
#define MWM_FUNC_MAXIMIZE	(1L << 4)
#define MWM_FUNC_CLOSE		(1L << 5)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL		(1L << 0)
#define MWM_DECOR_BORDER	(1L << 1)
#define MWM_DECOR_RESIZEH	(1L << 2)
#define MWM_DECOR_TITLE		(1L << 3)
#define MWM_DECOR_MENU		(1L << 4)
#define MWM_DECOR_MINIMIZE	(1L << 5)
#define MWM_DECOR_MAXIMIZE	(1L << 6)

/* values for MwmHints.input_mode */
#define MWM_INPUT_MODELESS			0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL	1
#define MWM_INPUT_SYSTEM_MODAL			2
#define MWM_INPUT_FULL_APPLICATION_MODAL	3

/* bit definitions for MwmHints.status */
#define MWM_TEAROFF_WINDOW	(1L << 0)

/*
 * The following is for compatibility only. It use is deprecated.
 */
#define MWM_INPUT_APPLICATION_MODAL	MWM_INPUT_PRIMARY_APPLICATION_MODAL


/*
 * Contents of the _MWM_INFO property.
 */

typedef struct
{
    long	flags;
    Window	wm_window;
} MotifWmInfo;

typedef MotifWmInfo	MwmInfo;

/* bit definitions for MotifWmInfo .flags */
#define MWM_INFO_STARTUP_STANDARD	(1L << 0)
#define MWM_INFO_STARTUP_CUSTOM		(1L << 1)



/*
 * Definitions for the _MWM_HINTS property.
 */

typedef struct
{
#ifndef OSF_v1_2_4
    CARD32	flags;
    CARD32	functions;
    CARD32	decorations;
    INT32	inputMode;
    CARD32	status;
#else /* OSF_v1_2_4 */
    /* 32-bit property items are stored as long on the client (whether
     * that means 32 bits or 64).  XChangeProperty handles the conversion
     * to the actual 32-bit quantities sent to the server.
     */
    unsigned long	flags;
    unsigned long	functions;
    unsigned long	decorations;
    long 	        inputMode;
    unsigned long	status;
#endif /* OSF_v1_2_4 */
} PropMotifWmHints;

typedef PropMotifWmHints	PropMwmHints;


/* number of elements of size 32 in _MWM_HINTS */
#define PROP_MOTIF_WM_HINTS_ELEMENTS	5
#define PROP_MWM_HINTS_ELEMENTS		PROP_MOTIF_WM_HINTS_ELEMENTS

/* atom name for _MWM_HINTS property */
#define _XA_MOTIF_WM_HINTS	"_MOTIF_WM_HINTS"
#define _XA_MWM_HINTS		_XA_MOTIF_WM_HINTS

/*
 * Definitions for the _MWM_MESSAGES property.
 */

#define _XA_MOTIF_WM_MESSAGES	"_MOTIF_WM_MESSAGES"
#define _XA_MWM_MESSAGES	_XA_MOTIF_WM_MESSAGES

/* atom that enables client frame offset messages */
#define _XA_MOTIF_WM_OFFSET	"_MOTIF_WM_OFFSET"

/*
 * Definitions for the _MWM_MENU property.
 */

/* atom name for _MWM_MENU property */
#define _XA_MOTIF_WM_MENU	"_MOTIF_WM_MENU"
#define _XA_MWM_MENU		_XA_MOTIF_WM_MENU


/*
 * Definitions for the _MWM_INFO property.
 */

typedef struct
{
#ifndef OSF_v1_2_4
    CARD32 flags;
    CARD32 wmWindow;
#else /* OSF_v1_2_4 */
  long		flags;
  Window	wmWindow;
#endif /* OSF_v1_2_4 */
} PropMotifWmInfo;

typedef PropMotifWmInfo	PropMwmInfo;


/* number of elements of size 32 in _MWM_INFO */
#define PROP_MOTIF_WM_INFO_ELEMENTS	2
#define PROP_MWM_INFO_ELEMENTS		PROP_MOTIF_WM_INFO_ELEMENTS

/* atom name for _MWM_INFO property */
#define _XA_MOTIF_WM_INFO	"_MOTIF_WM_INFO"
#define _XA_MWM_INFO		_XA_MOTIF_WM_INFO


/*
 * Miscellaneous atom definitions
 */

/* atom for motif input bindings */
#define _XA_MOTIF_BINDINGS	"_MOTIF_BINDINGS"

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMwmUtil_h */
