/* @(#)98     1.9  src/bos/kernext/rcm/inc/gai/gai.h, rcm, bos41J, 9519B_all 5/10/95 16:35:24 */

/*
 *
 * COMPONENT_NAME: rcm
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1995
 * All Rights Reserved
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

#ifndef __H_GAI__
#define __H_GAI__
 
#include <sys/types.h>
 
#ifndef gPointer
#define gPointer char *
#endif
#ifndef gBool
#define gBool unsigned int
#endif
 
/*
 * GAI users will use gResource to get resource type.
 * GAI internals will define and undefine gResource as required.
 */
#ifndef gINTERNALS
#define gResource struct { ushort type; }
#endif
 
/*
 * Wait options
 */
#define gDoNotWait      0
#define gWait           1
 
/*
 * GAI Resource Types, in range 0x0000 - 0x00ff
 */
#define gAdapterType      0x0001
#define gWindowType       0x0002
#define gColormapType     0x0003
#define gCursorType       0x0004
#define gFontType         0x0005
#define gMonitorType      0x0006
#define gWinGeomType      0x0007
#define gFIFOType         0x0008
 
/*
 * Continuation of Resource Types, for general
 * category of GROUP
 *
 *                         range 0x0100 - 0x02ff
 */
#define gStandardGroup    0x0101
#define gFrameBufferGroup gStandardGroup
#define gOverlayGroup     0x0102
#define gUnderlayGroup    0x0103
#define gZBufferGroup     0x0104
#define gClipGroup        0x0105
#define gMaskGroup        0x0106
#define gCursorGroup      0x0107
#define gFIFOGroup        0x0109
#define gOverUnderGroup   0x010A
 
/*
 * Continuation of Resource Types, for general
 * category of MODEL & GC
 *
 *                        range 0x0300 - 0x04ff
 */
#define g2dModelType      0x0300
#define g2dGCType         0x0301
 
#define g3dM1ModelType    0x0310
#define g3dM1GCType       0x0311
 
#define g3dM2ModelType    0x0320
#define g3dM2GCType       0x0321
 
#define g3dM5ModelType    0x0350
#define g3dM5GCType       0x0351
 
#define gAncillaryType    0x0400
#define gMbxType          0x0401
/*
 * Extension Resource Types start at 0x1000
 *
 *                        range 0x1000 - 0x1fff
 */
#define gConnExtType      0x1100
#define gConnectionType   0x1101
#define gRcxType          0x1102

#define gClClipExtType	  0x1200
#define gClientClipType	  0x1201
 
/*
 * User Defined Resources and Groups start at 0x8000
 */
 
/*
 * Clipping capabilities
 */
#define gNoClip         0
#define gClipBuffer1    1
#define gClipBuffer2    2
#define gClip           gClipBuffer1
 
/*
 * Masking capabilities
 */
#define gNoMask         0
#define gMaskBuffer1    1
#define gMaskBuffer2    2
#define gMask           gMaskBuffer1
 
/*
 * Multi-buffering
 */
#define gBuffer1        1
#define gBuffer2        2
 
/*
 * Coordinate origins
 */
#define gUpperLeft      1     /* 1st Quadrant  */
#define gLowerLeft      2     /* 4th Quadrent  */
 
#define gCWallWaFlags       (0x00000e7f)
#define gCWallWgFlags       (0x0000007f)
 
/*
 * Window change masks
 */
#define gCWwahandle         (1L<<0)
#define gCWnumGroups        (1L<<1)
#define gCWpActiveGroup     (1L<<2)
#define gCWgroup            (1L<<3)
#define gCWmaskOrg          (1L<<4)
#define gCWpMask            (1L<<5)
#define gCWpRegion          (1L<<6)
#define gCWdrawBuf          (1L<<7)
#define gCWdispBuf          (1L<<8)
#define gCWpalette          (1L<<9)
#define gCWorigin           (1L<<10)
#define gCWallgroups        (1L<<11)
 
/*
 * Window geometry change masks
 */
#define gCWwghandle         (1L<<0)
#define gCWwinOrg           (1L<<1)
#define gCWwidth            (1L<<2)
#define gCWheight           (1L<<3)
#define gCWclip             (1L<<4)
#define gCWrealize          (1L<<5)
#define gCWcolormap         (1L<<6)
#define gCWdepth            (1L<<7)
#define gCWcolorClass       (1L<<8)
#define gCWlayer            (1L<<9)
#define gCWvisibility       (1L<<10)
#define gCWcmapID           (1L<<11)
#define gCWcurrDispBuff     (1L<<12)
#define gCWtransparent      (1L<<13)

 
/*
 * Specifying the mode of SelectCRTMode
 */
#define gEnableRGB          1
#define gEnableNTSC         2
#define gEnableSECAM        3
#define gEnablePAL          4
 
/*
 * Specifying the mode of OverUnderGroup
 */
#define gSetOverlay         1
#define gSetUnderlay        2
 
/*
 * FIFO types
 */
#define gAnyFIFO          0
#define gByteFIFO         1
#define gShortFIFO        2
#define gIntegerFIFO      3
#define gFloatFIFO        4
#define gDoubleFloatFIFO  5
 
/*
 * Transparency and other palette flags
 */
#define gTransparent            0x80    /* OR'ed into palette class       */
#define gPseudoRGB              0x40    /* OR'ed into palette class       */
                                        /* - applied to gDirectColor      */
#define gPaletteClassMask       0x3f    /* Removes the two flags          */
 
/*
 * Malloc and free
 */
#define gMalloc         malloc
#define gFree           free
#define gRealloc        realloc
 
/*
 * Cursor Type Definitions
 *
 * GAI Cursors can have OR'ed combo of glyph and of cross hair.
 * The cross hair can be of three types.
 */
#define gGlyphCurs              0x0080  /* Set means glyph, 0 means not   */
#define gCrossHairCurs          0x0010  /* least significant four bits    */
                                        /* are width of crosshair         */
                                        /* range is 0x0010 - 0x001f       */
 
/*
 * Cross hair clipping attributes
 */
#define gCrossHairWidthMask     (0x000f)
#define gCrossHairTypeMask      (0x0030)
 
#define gFullScreenCross        (1<<5)
#define gWindowCross            (2<<5)
#define gClippedCross           (3<<5)
 
/*
 * Adapter constants
 */
#define gMaxNumAdapters           8
#define gGammaRange             256
 
/*
 * Cursor change masks
 */
#define gCChot          (1L<<1)
#define gCCpixmap       (1L<<2)
#define gCCcolormap     (1L<<3)
#define gCCtype         (1L<<4)
#define gCChwcursor     (1L<<5)
#define gCCvisible      (1L<<6)
#define gCCcrosshair    (1L<<7)
 
typedef struct _gExtension              *gExtensionPtr;
typedef union  _gPixmapFormat           *gPixmapFmtPtr;
typedef union  _gColor                  *gColorPtr;
typedef struct _gPalette                *gPalettePtr;
typedef struct _gArc                    *gArcPtr;
typedef struct _gRectangle              *gRectPtr;
typedef struct _gFIFOInfo               *gFIFOInfoPtr;
typedef struct _gWinGroup               *gWinGroupPtr;
 
typedef struct _gAdapter                *gAdapterPtr;
typedef struct _gAdapterAttributes      *gAdapterAttrPtr;
typedef struct _gAdapterProcedures      *gAdapterProcPtr;
typedef struct _gAdapterClientClipExtensionProcedures *gClClipExtProcPtr;
 
typedef struct _gClientClipExtension            *gClientClipExtPtr;
typedef struct _gClientClipExtensionAttributes  *gClientClipExtAttrPtr;
typedef struct _gClientClipExtensionProcedures  *gClientClipExtProcPtr;
 
typedef struct _gClientClip             *gClientClipPtr;
typedef struct _gClientClipAttributes   *gClientClipAttrPtr;
typedef struct _gClientClipProcedures   *gClientClipProcPtr;
 
typedef struct _gGroup                  *gGroupPtr;
typedef struct _gGroupAttributes        *gGroupAttrPtr;
typedef struct _gGroupProcedures        *gGroupProcPtr;
 
typedef struct _gMonitor                 *gMonitorPtr;
typedef struct _gMonitorAttributes      *gMonitorAttrPtr;
typedef struct _gMonitorProcedures      *gMonitorProcPtr;
 
typedef struct _gFIFO                   *gFIFOPtr;
typedef struct _gFIFOAttributes         *gFIFOAttrPtr;
typedef struct _gFIFOProcedures         *gFIFOProcPtr;
 
typedef struct _gWindow                 *gWindowPtr;
/*             _gWindowAttributes       defined in <sys/rcm_win.h> */
typedef struct _gWindowProcedures       *gWindowProcPtr;
 
typedef struct _gWinGeom                *gWinGeomPtr;
/*             _gWinGeomAttributes      defined in <sys/rcm_win.h> */
typedef struct _gWinGeomProcedures      *gWinGeomProcPtr;
 
typedef struct _gCursor                 *gCursorPtr;
typedef struct _gCursorAttributes       *gCursorAttrPtr;
typedef struct _gCursorProcedures       *gCursorProcPtr;
 
typedef struct _gColormap               *gColormapPtr;
typedef struct _gColormapAttributes     *gColormapAttrPtr;
typedef struct _gColormapProcedures     *gColormapProcPtr;
 
typedef struct _gFont                   *gFontPtr;
typedef struct _gFontAttributes         *gFontAttrPtr;
typedef struct _gFontProcedures         *gFontProcPtr;
 
typedef struct _gConnExtension          *gConnExtPtr;
typedef struct _gConnExtAttributes      *gConnExtAttrPtr;
typedef struct _gConnExtProcedures      *gConnExtProcPtr;
 
typedef struct _gConnection             *gConnectionPtr;
typedef struct _gConnectionAttributes   *gConnectionAttrPtr;
typedef struct _gConnectionProcedures   *gConnectionProcPtr;
 
typedef struct _gRcx                    *gRcxPtr;
typedef struct _gRcxAttributes          *gRcxAttrPtr;
typedef struct _gRcxProcedures          *gRcxProcPtr;
 
typedef struct _gGenericGC              *gGenericGCPtr;
 
#define         gModelPtr       gPointer
#define         gGCPtr          gPointer
 
/* ------Declare an entry point for CreateAdapter------ */
extern gAdapterPtr      CreateAdapter();
 
#endif /* __H_GAI__ */

