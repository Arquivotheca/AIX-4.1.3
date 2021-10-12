/* @(#)18	1.5  src/bos/kernext/lft/inc/sys/aixfont.h, lftdd, bos411, 9428A410j 4/22/94 13:24:47 */
/*
 *   COMPONENT_NAME: LFTDD
 *
 *   FUNCTIONS: BYTESOFCHARINFO
 *		BYTESOFFONTINFO
 *		BYTESOFGLYPHINFO
 *		BYTESOFINKINFO
 *		BYTESOFPROPINFO
 *		BYTESOFSTRINGINFO
 *		NUMCHARS
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 */


#ifndef __aixFONT__
#define __aixFONT__

#ifndef Bool
#define Bool       unsigned
#endif

/*
 * This file describes the AIX Font format.
 *
 * This file is used by:
 *      X11 server
 *      font converters
 *
 * Each font file contains the following data structures.  There is no
 * padding between these data structures.
 *
 *      1)  The FontInfo structure
 *              maxbounds.byteoffset is the total number of bytes in the
 *                      glpyh array
 *
 *      2)  The CharInfo array
 *              character bounds (metrics)
 *              offsets to glyphs
 *              indexed directly with character codes
 *
 *      3)  Character glyphs
 *              most significant bit/byte first
 *              end of glyphs padded to 32-bit boundary.
 *              raster format:
 *                  each scanline padded to a byte boundary
 *              stroke format:
 *                  2 byte number of x,y coordinates
 *                  2 byte pairs of x,y coordinates
 *                      sxxxxxx1syyyyyyf
 *                  where
 *                      s - sign bit
 *                      x - relative x coordinate
 *                      y - relative y coordinate
 *                      f - flag: 0 = draw, 1 = move
 *
 *      4)  Properties (optional)
 *              named properties
 *              property is a value or null-terminated strings
 *              character encodings are stored as properties
 *
 *      5)  Ink metrics (optional)
 *              character "ink" metrics
 *                  minimum bounds followed by
 *                  maximum bounds followed by
 *                  per character bounds
 *              indexed directly with character codes
 *
 *
 * Note:  An X11 Release 3 SNF font file (version 4)
 *        will map exactly onto an aixfont file.
 *                     - and -
 *        A non-stroke aixfont file will map onto an
 *        X11 Release 3 SNF font file.
 *
 */

#define FONT_FILE_VERSION       4

typedef struct _aixFontProp {
    unsigned   name;              /* offset of string                      */
    int        value;             /* a number or a string offset           */
    Bool       indirect;          /* value is a string offset   */
 } aixFontProp;

typedef struct _aixCharInfo {
    short      leftSideBearing;    /*                                      */
    short      rightSideBearing;   /*                                      */
    short      characterWidth;     /*                                      */
    short      ascent;             /*                                      */
    short      descent;            /*                                      */
    unsigned short attributes;     /* must be zero                         */
    unsigned   byteOffset:24;      /* byte offset of raster from pGlyphs   */
    Bool        exists:1;       /* true iff glyph exists for this char  */
    unsigned   pad:7;              /* reserved - must be zero              */
} aixCharInfo;

typedef struct _aixFontInfo {
    unsigned     version1;         /* version stamp                        */
    unsigned     allExist;         /*                                      */
    unsigned     drawDirection;    /*                                      */
    unsigned     noOverlap;        /* see note (1)                         */
    unsigned     constantMetrics;  /*                                      */
    unsigned     terminalFont;     /* see note (2)                         */
    unsigned     linear:1;         /* true if firstRow == lastRow          */
    unsigned     constantWidth:1;  /* see note (3)                         */
    unsigned     inkInside:1;      /* see note (4)                         */
    unsigned     inkMetrics:1;     /* see note (5)                         */
    unsigned     strokes:1;        /* glyphs are strokes, not rasters.     */
    unsigned     padding:27;       /*                                      */
    unsigned     firstCol;         /*                                      */
    unsigned     lastCol;          /*                                      */
    unsigned     firstRow;         /*                                      */
    unsigned     lastRow;          /*                                      */
    unsigned     nProps;           /*                                      */
    unsigned     lenStrings;       /* length in bytes of string table      */
    unsigned     chDefault;        /* default character                    */
    int          fontDescent;      /* minimum for quality typography       */
    int          fontAscent;       /* minimum for quality typography       */
    aixCharInfo  minbounds;        /* MIN of glyph metrics over all chars  */
    aixCharInfo  maxbounds;        /* MAX of glyph metrics over all chars  */
    unsigned     pixDepth;         /* intensity bits per pixel             */
    unsigned     glyphSets;        /* number of glyph sets (see note 6)    */
    unsigned     version2;         /* version stamp double-check           */
} aixFontInfo;

/*
 *  note (1)   noOverlap      true if max(rightSideBearing-characterWidth)
 *                               <= minbounds->metrics.leftSideBearing
 *
 *  note (2)   terminalFont   Should be deprecated!  true if:
 *                            constant metrics && leftSideBearing == 0 &&
 *                            rightSideBearing == characterWidth &&
 *                            ascent == fontAscent && descent == fontDescent
 *
 *  note (3)   constantWidth  true if minbounds->metrics.characterWidth
 *                                 == maxbounds->metrics.characterWidth
 *
 *  note (4)   inkInside      true if for all defined glyphs:
 *                            leftSideBearing >= 0 &&
 *                            rightSideBearing <= characterWidth &&
 *                            -fontDescent <= ascent <= fontAscent &&
 *                            -fontAscent <= descent <= fontDescent
 *
 *  note (5)   inkMetrics     ink metrics != bitmap metrics used with
 *                            terminalFont.  See font's pInk{CI,Min,Max}
 *
 *  note (6)   glyphSets      used for sub-pixel positioning
 */

typedef struct _aixFontInfo *aixFontInfoPtr;
typedef struct _aixFontProp *aixFontPropPtr;
typedef struct _aixCharInfo *aixCharInfoPtr;
typedef char                *aixGlyphPtr;

#define NUMCHARS(pfi)          (((pfi)->lastCol - (pfi)->firstCol + 1) * \
                                ((pfi)->lastRow - (pfi)->firstRow + 1))
#define BYTESOFFONTINFO(pfi)   (sizeof(aixFontInfo))
#define BYTESOFCHARINFO(pfi)   (sizeof(aixCharInfo) * NUMCHARS(pfi))
#define BYTESOFPROPINFO(pfi)   (sizeof(aixFontProp) * (pfi)->nProps)
#define BYTESOFSTRINGINFO(pfi) ((pfi)->lenStrings)
#define BYTESOFGLYPHINFO(pfi)  (((pfi)->maxbounds.byteOffset+3) & ~0x3)
#define BYTESOFINKINFO(pfi)    (sizeof(aixCharInfo) * (2 + NUMCHARS(pfi)))

#endif /* __aixFONT__ */
