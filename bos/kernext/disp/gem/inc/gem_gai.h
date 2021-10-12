/* @(#)69	1.4.2.3  src/bos/kernext/disp/gem/inc/gem_gai.h, sysxdispgem, bos411, 9428A410j 1/19/93 12:42:44 */
/*
 *   COMPONENT_NAME: SYSXDISPGEM
 *
 *   FUNCTIONS: *		
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */



/*
 * window change masks
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
 * window geometry change masks
 */
#define gCWwghandle         (1L<<0)
#define gCWwinOrg           (1L<<1)
#define gCWwidth            (1L<<2)
#define gCWheight           (1L<<3)
#define gCWclip             (1L<<4)
#define gCWrealize          (1L<<5)
#define gCWcolormap         (1L<<6)

/*
 * Group defines
 */
    
#define gStandardGroup    0x0101
#define gFrameBufferGroup gStandardGroup
#define gOverlayGroup     0x0102
#define gUnderlayGroup    0x0103
#define gZBufferGroup     0x0104
#define gClipGroup        0x0105
#define gMaskGroup        0x0106
#define gCursorGroup      0x0107
#define gDataGroup        0x0108
#define gFIFOGroup        0x0109
#define gOverUnderGroup   0x010A

typedef struct _gRectangle {           /* --- exactly like IBM X server ---*/
    gPoint      ul;                    /* upper left rectangle coordinates */
    ushort      width;                 /* rectangle width in pixels        */
    ushort      height;                /* rectangle height in pixels       */
} gRectangle;
