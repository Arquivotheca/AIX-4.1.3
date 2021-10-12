/* @(#)61       1.3  src/bldenv/bdftosnf/bdftosnf.h, sysxdisp, bos412, GOLDA411a 2/21/94 15:25:30 */
/*
 *   COMPONENT_NAME: sysxdisp
 *
 *   FUNCTIONS: GLWIDTHBYTESPADDED
 *		MAX
 *		MIN
 *
 *   ORIGINS: 27,18,40,42,16
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1987,1990
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (c) Copyright 1989, OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
 * (c) Copyright 1987, 1988, 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY
 */
#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

typedef struct _GlyphMap {
    char        *bits;
    int         h;
    int         w;
    int         widthBytes;
} GlyphMap;

/*
 * a structure to hold all the pointers to make it easy to pass them all
 * around. Much like the FONT structure in the server.
 */

typedef struct _TempFont {
    FontInfoPtr pFI;
    CharInfoPtr pCI;
    unsigned char *pGlyphs;
    FontPropPtr pFP;
    CharInfoPtr pInkCI;
    CharInfoPtr pInkMin;
    CharInfoPtr pInkMax;
} TempFont; /* not called font since collides with type in X.h */

#   if  (AIXrt || AIXrios)
#       define DEFAULTGLPAD     1               /* default padding for glyphs */
#       define DEFAULTBITORDER  MSBFirst        /* default bitmap bit order */
#       define DEFAULTBYTEORDER MSBFirst        /* default bitmap byte order */
#       define DEFAULTSCANUNIT  1               /* default bitmap scan unit */
#   else
#   ifdef  AIXps2
#       define DEFAULTGLPAD     1               /* default padding for glyphs */
#       define DEFAULTBITORDER  LSBFirst        /* default bitmap bit order */
#       define DEFAULTBYTEORDER MSBFirst        /* default bitmap byte order */
#       define DEFAULTSCANUNIT  1               /* default bitmap scan unit */
#   else
#       define DEFAULTGLPAD     1               /* default padding for glyphs */
#       define DEFAULTBITORDER MSBFirst         /* default bitmap bit order */
#       define DEFAULTBYTEORDER MSBFirst        /* default bitmap byte order */
#       define DEFAULTSCANUNIT  1               /* default bitmap scan unit */
#   endif
#   endif

#define GLWIDTHBYTESPADDED(bits,nbytes) \
        ((nbytes) == 1 ? (((bits)+7)>>3)        /* pad to 1 byte */ \
        :(nbytes) == 2 ? ((((bits)+15)>>3)&~1)  /* pad to 2 bytes */ \
        :(nbytes) == 4 ? ((((bits)+31)>>3)&~3)  /* pad to 4 bytes */ \
        :(nbytes) == 8 ? ((((bits)+63)>>3)&~7)  /* pad to 8 bytes */ \
        : 0)

