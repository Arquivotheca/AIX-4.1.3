static char sccsid[] = "@(#)62  1.4  src/bldenv/bdftosnf/fontutil.c, sysxdisp, bos412, GOLDA411a 2/21/94 15:26:07";
/*
 *   COMPONENT_NAME: sysxdisp
 *
 *   FUNCTIONS: DumpBitmaps
 *		DumpCharInfo
 *		DumpFont
 *		ISBITON
 *		SETBIT
 *		WriteNFont
 *		bitorderinvert
 *		computeInk
 *		computeInkMetrics
 *		computeNaccelerators
 *		computeNbounds
 *		fourbyteinvert
 *		padGlyph
 *		padGlyphsToTE
 *		twobyteinvert
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
#include <stdio.h>
/**#include <X11/Xos.h> **/
/**#include <X11/X.h>        **/
/**#include <X11/Xmd.h>      **/
/**#include <X11/Xproto.h>   **/
#include "misc.h"
#include "snffont.h"
#define  FontInfoPtr aixFontInfoPtr
#define  CharInfoPtr aixCharInfoPtr
#define  FontPropPtr aixFontPropPtr
#define  FontInfoRec aixFontInfo
#define  CharInfoRec aixCharInfo
#define  FontPropRec aixFontProp
#define  chLast      lastCol
#define  chFirst     firstCol

#include "port.h"       /* used by converters only */
#include "bdftosnf.h"   /* used by converters only */

char *  malloc();

static void computeInk(), computeInkMetrics();

/*
 * Computes min and max bounds for a set of glyphs,
 * maxoverlap (right bearing extent beyond character width,
 * and number of nonexistant characters (all zero metrics).
 */
static void
computeNbounds(minbounds, maxbounds, pCI, nchars, maxoverlap, nnonexistchars)
    register CharInfoPtr minbounds;
    register CharInfoPtr maxbounds;
    register CharInfoPtr pCI;
    int nchars;
    register int *maxoverlap;
    int *nnonexistchars;
{
    register int chi;

    *maxoverlap = MINSHORT;
    *nnonexistchars = 0;

    minbounds->ascent = MAXSHORT;
    minbounds->descent = MAXSHORT;
    minbounds->leftSideBearing = MAXSHORT;
    minbounds->rightSideBearing = MAXSHORT;
    minbounds->characterWidth = pCI->characterWidth;
    /* don't touch byteOffset! */
    minbounds->exists = 0;
    minbounds->attributes = 0xFFFF;     /* all bits on */

    maxbounds->ascent = MINSHORT;
    maxbounds->descent = MINSHORT;
    maxbounds->leftSideBearing = MINSHORT;
    maxbounds->rightSideBearing = MINSHORT;
    maxbounds->characterWidth = pCI->characterWidth;
    /* don't touch byteOffset! */
    maxbounds->exists = 0;
    maxbounds->attributes = 0;

    for (chi = 0; chi < nchars; chi++)
        {
        register CharInfoPtr pci = &pCI[chi];

        if (    pci->ascent == 0
             && pci->descent == 0
             && pci->leftSideBearing == 0
             && pci->rightSideBearing == 0
             && pci->characterWidth == 0) {
            (*nnonexistchars)++;
            pci->exists = FALSE;
        }
        else {

#define MINMAX(field) \
        if (minbounds->field > pci->field) \
             minbounds->field = pci->field; \
        if (maxbounds->field < pci->field) \
             maxbounds->field = pci->field;

            pci->exists = TRUE;
            MINMAX(ascent);
            MINMAX(descent);
            MINMAX(leftSideBearing);
            MINMAX(rightSideBearing);
            MINMAX(characterWidth);
            minbounds->attributes &= pci->attributes;
            maxbounds->attributes |= pci->attributes;
            *maxoverlap = MAX(*maxoverlap,
                pci->rightSideBearing-pci->characterWidth);
#undef MINMAX
        }
    }
}

/*
 * Computes accelerators, which by definition can be extracted from
 * the font; therefore a pointer to the font is the only argument.
 *
 * Should also give a final sanity-check to the font, so it can be
 * written to disk.
 *
 * Generates values for the following fields in the FontInfo structure:
 *      version
 *      allExist
 *      noOverlap
 *      constantMetrics
 *      constantWidth
 *      terminalFont
 *      inkMetrics
 *      linear
 *      inkInside
 *      minbounds
 *      maxbounds       - except byteOffset
 *      pInkCI
 *      pInkMin
 *      pInkMax
 */
computeNaccelerators(font, makeTEfonts, inhibitInk, glyphPad)
    TempFont *font;
    int makeTEfonts;
    int inhibitInk;
    int glyphPad;
{
    int         nchars;
    register CharInfoPtr minbounds = &font->pFI->minbounds;
    register CharInfoPtr maxbounds = &font->pFI->maxbounds;

    int         maxoverlap;
    int         nnonexistchars;

    font->pFI->version1 = font->pFI->version2 = FONT_FILE_VERSION;

    nchars = n2dChars(font->pFI);
    computeNbounds(minbounds, maxbounds, font->pCI, nchars,
                   &maxoverlap, &nnonexistchars);

    if ( maxoverlap <= minbounds->leftSideBearing)
        font->pFI->noOverlap = TRUE;
    else
        font->pFI->noOverlap = FALSE;

    if ( nnonexistchars == 0)
        font->pFI->allExist = TRUE;
    else
        font->pFI->allExist = FALSE;

    /*
     * attempt to make this font a terminal emulator font if
     * it isn't already
     */

    if ( makeTEfonts &&
         (minbounds->leftSideBearing >= 0) &&
         (maxbounds->rightSideBearing <= maxbounds->characterWidth) &&
         (minbounds->characterWidth == maxbounds->characterWidth) &&
         (maxbounds->ascent <= font->pFI->fontAscent) &&
         (maxbounds->descent <= font->pFI->fontDescent) &&
         (maxbounds->leftSideBearing != 0 ||
          minbounds->rightSideBearing != minbounds->characterWidth ||
          minbounds->ascent != font->pFI->fontAscent ||
          minbounds->descent != font->pFI->fontDescent) )
    {
        padGlyphsToTE (font, glyphPad);
    }

    if ( (minbounds->ascent == maxbounds->ascent) &&
         (minbounds->descent == maxbounds->descent) &&
         (minbounds->leftSideBearing ==
                maxbounds->leftSideBearing) &&
         (minbounds->rightSideBearing ==
                maxbounds->rightSideBearing) &&
         (minbounds->characterWidth ==
                maxbounds->characterWidth) &&
         (minbounds->attributes == maxbounds->attributes)) {
        font->pFI->constantMetrics = TRUE;
        if ( (maxbounds->leftSideBearing == 0) &&
             (maxbounds->rightSideBearing ==
                        maxbounds->characterWidth) &&
             (maxbounds->ascent == font->pFI->fontAscent) &&
             (maxbounds->descent == font->pFI->fontDescent) )
                 font->pFI->terminalFont = TRUE;
        else
                 font->pFI->terminalFont = FALSE;
    }
    else {
        font->pFI->constantMetrics = FALSE;
        font->pFI->terminalFont = FALSE;
    }

    if (font->pFI->terminalFont && !inhibitInk)
        computeInkMetrics(font);
    else {
        font->pFI->inkMetrics = FALSE;
        font->pInkCI = (CharInfoPtr)NULL;
        font->pInkMin = (CharInfoPtr)NULL;
        font->pInkMax = (CharInfoPtr)NULL;
    }

    if( font->pFI->lastRow == font->pFI->firstRow)
        font->pFI->linear = TRUE;
    else
        font->pFI->linear = FALSE;

    if (minbounds->characterWidth == maxbounds->characterWidth)
        font->pFI->constantWidth = TRUE;
    else
        font->pFI->constantWidth = FALSE;

    if ((minbounds->leftSideBearing >= 0) &&
        (maxoverlap <= 0) &&
        (minbounds->ascent >= -font->pFI->fontDescent) &&
        (maxbounds->ascent <= font->pFI->fontAscent) &&
        (minbounds->descent <= -font->pFI->fontAscent) &&
        (maxbounds->descent <= font->pFI->fontDescent))
        font->pFI->inkInside = TRUE;
    else
        font->pFI->inkInside = FALSE;
}


static void
computeInkMetrics (font)
    TempFont *font;
{
    int nchars, maxoverlap, nnonexistchars;
    register CharInfoPtr pCI, pInkCI;
    register int i;

    nchars = n2dChars(font->pFI);
    pCI = font->pCI;
    pInkCI = (CharInfoPtr) malloc ((unsigned)(nchars + 2) *
                                   sizeof (CharInfoRec));
    font->pInkMin = &pInkCI[0];
    font->pInkMax = &pInkCI[1];
    pInkCI += 2;
    font->pInkCI = pInkCI;
    for (i = 0; i < nchars; i++)
        computeInk(&pCI[i], &pInkCI[i], font->pGlyphs);
    computeNbounds(font->pInkMin, font->pInkMax, pInkCI, nchars,
                   &maxoverlap, &nnonexistchars);
    font->pFI->inkMetrics = TRUE;
}

static unsigned char ink_mask[8] = {
     0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
};

static void
computeInk (bitmap, ink, pglyphBase)
    CharInfoPtr bitmap, ink;
    unsigned char       *pglyphBase;
{
    int                 leftBearing, ascent, descent;
    unsigned char       *pglyph;
    register int        vpos, hpos, bpos;
    int                 bitmapByteWidth, bitmapByteWidthPadded;
    int                 bitmapBitWidth;
    int                 span;
    register unsigned char      *p;
    register int        bmax;
    register unsigned char      charbits;

    ink->characterWidth = bitmap->characterWidth;
    ink->attributes = bitmap->attributes;
    ink->byteOffset = bitmap->byteOffset;

    leftBearing = bitmap->leftSideBearing;
    ascent = bitmap->ascent;
    descent = bitmap->descent;
    bitmapBitWidth = GLYPHWIDTHPIXELS (bitmap);
    bitmapByteWidth = GLYPHWIDTHBYTES (bitmap);
    bitmapByteWidthPadded = GLYPHWIDTHBYTESPADDED (bitmap);
    span = bitmapByteWidthPadded - bitmapByteWidth;
    pglyph = pglyphBase + bitmap->byteOffset;

    p = pglyph;
    for (vpos = descent + ascent; --vpos >= 0;)
    {
        for (hpos = bitmapByteWidth; --hpos >= 0;)
        {
            if (*p++ != 0)
                goto found_ascent;
        }
        p += span;
    }
    /*
     * special case -- font with no bits gets all zeros
     */
    ink->leftSideBearing = leftBearing;
    ink->rightSideBearing = leftBearing;
    ink->ascent = 0;
    ink->descent = 0;
    return;
found_ascent:
    ink->ascent = vpos - descent + 1;

    p = pglyph + bitmapByteWidthPadded * (descent + ascent - 1) + bitmapByteWidth;

    for (vpos = descent + ascent; --vpos >= 0;)
    {
        for (hpos = bitmapByteWidth; --hpos >= 0;)
        {
            if (*--p != 0)
                goto found_descent;
        }
        p -= span;
    }
found_descent:
    ink->descent = vpos - ascent + 1;

    bmax = 8;
    for (hpos = 0; hpos < bitmapByteWidth; hpos++)
    {
        charbits = 0;
        p = pglyph + hpos;
        for (vpos = descent + ascent; --vpos >= 0; p += bitmapByteWidthPadded)
            charbits |= *p;
        if (charbits)
        {
            if (hpos == bitmapByteWidth - 1)
                bmax = bitmapBitWidth - (hpos << 3);
            p = ink_mask;
            for (bpos = bmax; --bpos >= 0;)
            {
                if (charbits & *p++)
                    goto found_left;
            }
        }
    }
found_left:
    ink->leftSideBearing = leftBearing + (hpos << 3) + bmax - bpos - 1;

    bmax = bitmapBitWidth - ((bitmapByteWidth - 1) << 3);
    for (hpos = bitmapByteWidth; --hpos >= 0;)
    {
        charbits = 0;
        p = pglyph + hpos;
        for (vpos = descent + ascent; --vpos >= 0; p += bitmapByteWidthPadded)
            charbits |= *p;
        if (charbits)
        {
            p = ink_mask + bmax;
            for (bpos = bmax; --bpos >= 0;)
            {
                if (charbits & *--p)
                    goto found_right;
            }
        }
        bmax = 8;
    }
found_right:
    ink->rightSideBearing = leftBearing + (hpos << 3) + bpos + 1;
}

/*
 * given a constant width font, convert it to a terminal emulator font
 */

padGlyphsToTE (font, glyphPad)
    TempFont    *font;
    int glyphPad;
{
        unsigned char   *newGlyphs;
        int     numGlyphs;
        int     glyphwidth, glyphascent, glyphdescent, glyphsize;
        int     i, nchars;
        CharInfoPtr     pCI;
        unsigned int    oldminattr, oldmaxattr;

        glyphwidth = GLWIDTHBYTESPADDED (font->pFI->minbounds.characterWidth,
                                        glyphPad);
        glyphascent = font->pFI->fontAscent;
        glyphdescent = font->pFI->fontDescent;
        glyphsize = glyphwidth * (glyphascent + glyphdescent);
        nchars = n2dChars (font->pFI);
        newGlyphs = (unsigned char *) malloc (nchars * glyphsize);
        if (!newGlyphs)
                return;
        pCI = font->pCI;
        for (i = 0; i < nchars; i++) {
                padGlyph (&pCI[i], font->pGlyphs, &newGlyphs[glyphsize*i],
                          glyphwidth, glyphascent, glyphdescent, glyphPad);
                pCI[i].byteOffset = glyphsize * i;
        }
        font->pGlyphs = newGlyphs;
        oldminattr = font->pFI->minbounds.attributes;
        oldmaxattr = font->pFI->maxbounds.attributes;
        font->pFI->minbounds = pCI[0];
        font->pFI->maxbounds = pCI[0];
        font->pFI->minbounds.byteOffset = 0;
        font->pFI->maxbounds.byteOffset = nchars * glyphsize;
        font->pFI->minbounds.attributes = oldminattr;
        font->pFI->maxbounds.attributes = oldmaxattr;
}

# define ISBITON(x, line) ((line)[(x)/8] & (1 << (7-((x)%8))))
# define SETBIT(x, line) ((line)[(x)/8] |= (1 << (7-((x)%8))))

padGlyph (pCI, oldglyphs, newglyph, width, ascent, descent, glyphPad)
        register CharInfoPtr pCI;
        unsigned char   *oldglyphs, *newglyph;
        int     width, ascent, descent;
        int glyphPad;
{
        int     x, y;
        int     dx, dy;
        unsigned char   *inline, *outline;
        int     inwidth;

        dx = pCI->leftSideBearing;
        dy = ascent - pCI->ascent;
        bzero (newglyph, width * (ascent + descent));
        inline = oldglyphs + pCI->byteOffset;
        outline = newglyph + dy * width;
        inwidth = GLWIDTHBYTESPADDED (pCI->characterWidth, glyphPad);
        for (y = 0; y < pCI->ascent + pCI->descent; y++) {
                for (x = 0;
                     x < pCI->rightSideBearing - pCI->leftSideBearing;
                     x++)
                {
                        if (ISBITON (x, inline))
                                SETBIT (x + dx, outline);
                }
                inline += inwidth;
                outline += width;
        }
        pCI->leftSideBearing = 0;
        pCI->rightSideBearing = pCI->characterWidth;
        pCI->ascent = ascent;
        pCI->descent = descent;
}



WriteNFont(pfile, font, pname)
    FILE *      pfile;
    TempFont    *font;
    char *(*pname)();
{
    int         np;
    FontPropPtr fp;
    int         off;            /* running index into string table */
    int         strbytes;       /* size of string table */
    char        *strings;       /* string table */

    /* run through the properties and add up the string lengths */
    strbytes = 0;
    for (fp=font->pFP,np=font->pFI->nProps; np; np--,fp++)
    {
        strbytes += strlen((*pname)(fp->name))+1;
        if (fp->indirect)
                strbytes += strlen((*pname)(fp->value))+1;
    }
    /* round up so ink metrics start aligned */
    strbytes = (strbytes + 3) & ~3;
    font->pFI->lenStrings = strbytes;

    /* build string table and convert pointers to offsets */
    strings = malloc((unsigned)strbytes);
    off = 0;
    for (fp=font->pFP,np=font->pFI->nProps; np; np--,fp++)
    {
        int l;
        l = strlen((*pname)(fp->name))+1; /* include null */
        bcopy((*pname)(fp->name), strings+off, l);
        fp->name = off;
        off += l;
        if (fp->indirect) {
                l = strlen((*pname)(fp->value))+1; /* include null */
                bcopy((*pname)(fp->value), strings+off, l);
                fp->value = off;
                off += l;
        }
    }

    fwrite( (char *)font->pFI, BYTESOFFONTINFO(font->pFI), 1, pfile);

    fwrite( (char *)(font->pCI),
            BYTESOFCHARINFO(font->pFI), 1, pfile);

    fwrite( (char *)font->pGlyphs, 1, BYTESOFGLYPHINFO(font->pFI), pfile);

    fwrite( (char *)font->pFP, 1, BYTESOFPROPINFO(font->pFI), pfile);

    fwrite( strings, 1, BYTESOFSTRINGINFO(font->pFI), pfile);

    if (font->pFI->inkMetrics) {
        fwrite( (char *)font->pInkMin, 1, sizeof(CharInfoRec), pfile);
        fwrite( (char *)font->pInkMax, 1, sizeof(CharInfoRec), pfile);
        fwrite( (char *)font->pInkCI, 1, BYTESOFCHARINFO(font->pFI), pfile);
    }
}



DumpFont( pfont, glyphPad, bVerbose)
    TempFont *  pfont;
    int         glyphPad;
    int         bVerbose;
{
    FontInfoPtr pfi = pfont->pFI;
    int bFailure = 0;
    int i;

    if ( pfont == NULL)
    {
        fprintf( stderr, "DumpFont: NULL FONT pointer passed\n");
        exit( 1);
    }
    if ( pfi == NULL)
    {
        fprintf( stderr, "DumpFont: NULL FontInfo pointer passed\n");
        exit( 1);
    }

    printf("version1: 0x%x (version2: 0x%x)\n", pfi->version1, pfi->version2);
    if ( pfi->version1 != FONT_FILE_VERSION)
        printf( "*** NOT CURRENT VERSION ***\n");
    if ( pfi->noOverlap)
        printf( " noOverlap");
    if ( pfi->allExist)
        printf( " allExist");
    if ( pfi->constantMetrics)
        printf( " constantMetrics");
    if ( pfi->terminalFont)
        printf( " terminalFont");
    if ( pfi->linear)
        printf( " linear");
    if ( pfi->constantWidth)
        printf( " constantWidth");
    if ( pfi->inkInside)
        printf( " inkInside");
    if ( pfi->inkMetrics)
        printf( " inkMetrics");
    printf("\nprinting direction: ");
    switch ( pfi->drawDirection)
    {
      case FontLeftToRight:
        printf( "left-to-right\n");
        break;
      case FontRightToLeft:
        printf( "right-to-left\n");
        break;
    }
    printf("first character: 0x%x\n", pfi->chFirst);
    printf("last characters:  0x%x\n", pfi->chLast);

    printf("number of font properties:  0x%x\n", pfi->nProps);
    for (i=0; i<pfi->nProps; i++) {
        printf("  %-15s  ", pfont->pFP[i].name);
        if (pfont->pFP[i].indirect)
                printf("%s\n", pfont->pFP[i].value);
        else
                printf("%d\n", pfont->pFP[i].value);
    }

    printf("default character: 0x%x\n", pfi->chDefault);

    printf("minbounds:\n");
    DumpCharInfo( -1, &pfi->minbounds, 1);
    printf("maxbounds:\n");
    DumpCharInfo( -1, &pfi->maxbounds, 1);
    if (pfont->pFI->inkMetrics) {
        printf("ink minbounds:\n");
        DumpCharInfo( -1, pfont->pInkMin, 1);
        printf("ink maxbounds:\n");
        DumpCharInfo( -1, pfont->pInkMax, 1);
    }
    printf("FontInfo struct: virtual memory address == %x\tsize == %x\n",
            pfont->pFI, sizeof(FontInfoRec));

    printf("CharInfo array: virtual memory base address == %x\tsize == %x\n",
            pfont->pCI, BYTESOFCHARINFO(pfi));

    if (pfont->pFI->inkMetrics)
        printf("ink CharInfo array: virtual memory base address == %x\tsize == %x\n",
            pfont->pInkCI, BYTESOFCHARINFO(pfi));

    printf("glyph block: virtual memory address == %x\tsize == %x\n",
            pfont->pGlyphs, pfi->maxbounds.byteOffset );

    if ( bVerbose>0) {
        DumpCharInfo( (int)pfont->pFI->chFirst, pfont->pCI,
                      (int)n2dChars(pfont->pFI));
        if (pfont->pFI->inkMetrics) {
            printf( "\nink metrics:\n");
            DumpCharInfo( (int)pfont->pFI->chFirst, pfont->pInkCI,
                          (int)n2dChars(pfont->pFI));
        }
    }

    if ( bVerbose>1)
    {
        if (bFailure)
            fprintf( stderr, "verbosity not possible with failed read");
        else
            DumpBitmaps( pfont, glyphPad);
    }
    printf("\n");
}

DumpCharInfo( first, pxci, count)
    int         first;
    CharInfoPtr pxci;
    int         count;
{
/*
    printf( "\nrbearing\tlbearing\tdescent\tascent\nwidth\tbyteOffset\tbitOffset\tciFlags");
*/
    if (first >= 0) {
        putchar ('\t');
    }
    printf( "rbearing\tdescent\t\twidth\t\texists\n");
    if (first >= 0) printf("number\t");
    printf( "\tlbearing\tascent\t\tbyteOffset\tciFlags\n");

    while( count--)
    {
        if (first >= 0) printf ("%d\t", first++);
        printf( "%4d\t%4d\t%4d\t%4d\t%4d\t0x%x\t%4s\t0x%x\n",
            pxci->rightSideBearing, pxci->leftSideBearing,
            pxci->descent, pxci->ascent,
            pxci->characterWidth, pxci->byteOffset,
            pxci->exists?"yes":"no", pxci->attributes);
        pxci++;
    }
}

DumpBitmaps( pFont, glyphPad)
    TempFont *pFont;
    int glyphPad;
{
    int                 ch;     /* current character */
    int                 r;      /* current row */
    int                 b;      /* current bit in the row */
    CharInfoPtr         pCI = pFont->pCI;
    unsigned char *     bitmap = (unsigned char *)pFont->pGlyphs;
    int                 n = n2dChars(pFont->pFI);

    for (ch = 0; ch < n; ch++)
    {
        int bpr = GLWIDTHBYTESPADDED(pCI[ch].rightSideBearing
                - pCI[ch].leftSideBearing, glyphPad);
        printf("character %d", ch + pFont->pFI->chFirst);
        if ( !pCI[ch].exists || pCI[ch].characterWidth == 0) {
            printf (" doesn't exist\n");
            continue;
        } else {
            putchar('\n');
        }
        for (r=0; r <  pCI[ch].descent + pCI[ch].ascent; r++)
        {
            unsigned char *row = bitmap + pCI[ch].byteOffset+(r*bpr);
            for ( b=0;
                b < pCI[ch].rightSideBearing - pCI[ch].leftSideBearing;
                b++) {
                        putchar((row[b>>3] & (1<<(7-(b&7))))? '#' : '-');
            }
            putchar('\n');
        }
    }
}

unsigned char _reverse_byte[0x100] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

/*
 *      Invert bit order within each BYTE of an array.
 */
void
bitorderinvert(buf, nbytes)
    register unsigned char *buf;
    register int nbytes;
{
    register unsigned char *rev = _reverse_byte;
    for (; --nbytes >= 0; buf++)
        *buf = rev[*buf];
}

/*
 *      Invert byte order within each 16-bits of an array.
 */
void
twobyteinvert(buf, nbytes)
    register unsigned char *buf;
    register int nbytes;
{
    register unsigned char c;

    for (; nbytes > 0; nbytes -= 2, buf += 2) {
        c = *buf;
        *buf = *(buf+1);
        *(buf+1) = c;
    }
}

/*
 *      Invert byte order within each 32-bits of an array.
 */
void
fourbyteinvert(buf, nbytes)
    register unsigned char *buf;
    register int nbytes;
{
    register unsigned char c;

    for (; nbytes > 0; nbytes -= 4, buf += 4) {
        c = *buf;
        *buf = *(buf+3);
        *(buf+3) = c;
        c = *(buf+1);
        *(buf+1) = *(buf+2);
        *(buf+2) = c;
    }
}
