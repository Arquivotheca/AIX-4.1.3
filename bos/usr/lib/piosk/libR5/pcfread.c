static char sccsid[] = "@(#)93  1.1  src/bos/usr/lib/piosk/libR5/pcfread.c, cmdpiosk, bos411, 9428A410j 7/31/92 21:38:57";
/*
 *  COMPONENT_NAME: 
 *
 *  FUNCTIONS: 
 *
 *  ORIGINS: 16,27,42 
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1992
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
*/
/*
 * $XConsortium: pcfread.c,v 1.8 92/02/11 13:16:44 eswu Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include    "fontfilest.h"
#include    "bitmap.h"
#include    "pcf.h"

/* Read PCF font files */

#ifdef  R4DDX
extern       int  use_conversion;
#endif
void        pcfUnloadFont();
static int  position;

static int
pcfGetLSB32(file)
    FontFilePtr file;
{
    int         c;

    c = FontFileGetc(file);
    c |= FontFileGetc(file) << 8;
    c |= FontFileGetc(file) << 16;
    c |= FontFileGetc(file) << 24;
    position += 4;
    return c;
}

static int
pcfGetINT32(file, format)
    FontFilePtr file;
    CARD32      format;
{
    int         c;

    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	c = FontFileGetc(file) << 24;
	c |= FontFileGetc(file) << 16;
	c |= FontFileGetc(file) << 8;
	c |= FontFileGetc(file);
    } else {
	c = FontFileGetc(file);
	c |= FontFileGetc(file) << 8;
	c |= FontFileGetc(file) << 16;
	c |= FontFileGetc(file) << 24;
    }
    position += 4;
    return c;
}

static int
pcfGetINT16(file, format)
    FontFilePtr file;
    CARD32      format;
{
    int         c;

    if (PCF_BYTE_ORDER(format) == MSBFirst) {
	c = FontFileGetc(file) << 8;
	c |= FontFileGetc(file);
    } else {
	c = FontFileGetc(file);
	c |= FontFileGetc(file) << 8;
    }
    position += 2;
    return c;
}

#define pcfGetINT8(file, format) (position++, FontFileGetc(file))

static      PCFTablePtr
pcfReadTOC(file, countp)
    FontFilePtr file;
    int        *countp;
{
    CARD32      version;
    PCFTablePtr tables;
    int         count;
    int         i;

    position = 0;
    version = pcfGetLSB32(file);
    if (version != PCF_FILE_VERSION)
	return (PCFTablePtr) NULL;
    count = pcfGetLSB32(file);
    tables = (PCFTablePtr) xalloc(count * sizeof(PCFTableRec));
    if (!tables)
	return (PCFTablePtr) NULL;
    for (i = 0; i < count; i++) {
	tables[i].type = pcfGetLSB32(file);
	tables[i].format = pcfGetLSB32(file);
	tables[i].size = pcfGetLSB32(file);
	tables[i].offset = pcfGetLSB32(file);
    }
    *countp = count;
    return tables;
}

/*
 * PCF supports two formats for metrics, both the regular
 * jumbo size, and 'lite' metrics, which are useful
 * for most fonts which have even vaguely reasonable
 * metrics
 */

static
pcfGetMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    metric->leftSideBearing = pcfGetINT16(file, format);
    metric->rightSideBearing = pcfGetINT16(file, format);
    metric->characterWidth = pcfGetINT16(file, format);
    metric->ascent = pcfGetINT16(file, format);
    metric->descent = pcfGetINT16(file, format);
    metric->attributes = pcfGetINT16(file, format);
}

static
pcfGetCompressedMetric(file, format, metric)
    FontFilePtr file;
    CARD32      format;
    xCharInfo  *metric;
{
    metric->leftSideBearing = pcfGetINT8(file, format) - 0x80;
    metric->rightSideBearing = pcfGetINT8(file, format) - 0x80;
    metric->characterWidth = pcfGetINT8(file, format) - 0x80;
    metric->ascent = pcfGetINT8(file, format) - 0x80;
    metric->descent = pcfGetINT8(file, format) - 0x80;
    metric->attributes = 0;
}

/*
 * Position the file to the begining of the specified table
 * in the font file
 */
static Bool
pcfSeekToType(file, tables, ntables, type, formatp, sizep)
    FontFilePtr file;
    PCFTablePtr tables;
    int         ntables;
    CARD32      type;
    CARD32     *formatp;
    CARD32     *sizep;
{
    int         i;

    for (i = 0; i < ntables; i++)
	if (tables[i].type == type) {
	    if (position > tables[i].offset)
		abort ();
	    if (!FontFileSkip(file, tables[i].offset - position))
		return FALSE;
	    position = tables[i].offset;
	    *sizep = tables[i].size;
	    *formatp = tables[i].format;
	    return TRUE;
	}
    return FALSE;
}

static Bool
pcfHasType (tables, ntables, type)
    PCFTablePtr tables;
    int         ntables;
    CARD32      type;
{
    int         i;

    for (i = 0; i < ntables; i++)
	if (tables[i].type == type)
	    return TRUE;
    return FALSE;
}

/*
 * pcfGetProperties 
 *
 * Reads the font properties from the font file, filling in the FontInfo rec
 * supplied.  Used by by both ReadFont and ReadFontInfo routines.
 */

static Bool
pcfGetProperties(pFontInfo, file, tables, ntables)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
    PCFTablePtr tables;
    int         ntables;
{
    FontPropPtr props = 0;
    int         nprops;
    char       *isStringProp = 0;
    CARD32      format;
    int         i;
    int         size;
    int         string_size;
    char       *strings;

    /* font properties */

    if (!pcfSeekToType(file, tables, ntables, PCF_PROPERTIES, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;
    nprops = pcfGetINT32(file, format);
    props = (FontPropPtr) xalloc(nprops * sizeof(FontPropRec));
    if (!props)
	goto Bail;
    isStringProp = (char *) xalloc(nprops * sizeof(char));
    if (!isStringProp)
	goto Bail;
    for (i = 0; i < nprops; i++) {
	props[i].name = pcfGetINT32(file, format);
	isStringProp[i] = pcfGetINT8(file, format);
	props[i].value = pcfGetINT32(file, format);
    }
    /* pad the property array */
    /*
     * clever here - nprops is the same as the number of odd-units read, as
     * only isStringProp are odd length
     */
    if (nprops & 3)
    {
	i = 4 - (nprops & 3);
	FontFileSkip(file, i);
	position += i;
    }
    string_size = pcfGetINT32(file, format);
    strings = (char *) xalloc(string_size);
    if (!strings) {
	goto Bail;
    }
    FontFileRead(file, strings, string_size);
    position += string_size;
    for (i = 0; i < nprops; i++) {
	props[i].name = MakeAtom(strings + props[i].name,
				 strlen(strings + props[i].name), TRUE);
	if (isStringProp[i]) {
	    props[i].value = MakeAtom(strings + props[i].value,
				      strlen(strings + props[i].value), TRUE);
	}
    }
    xfree(strings);
    pFontInfo->isStringProp = isStringProp;
    pFontInfo->props = props;
    pFontInfo->nprops = nprops;
    return TRUE;
Bail:
    xfree(isStringProp);
    xfree(props);
    return FALSE;
}


/*
 * pcfReadAccel
 *
 * Fill in the accelerator information from the font file; used
 * to read both BDF_ACCELERATORS and old style ACCELERATORS
 */

static Bool
pcfGetAccel(pFontInfo, file, tables, ntables, type)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
    PCFTablePtr	tables;
    int		ntables;
    CARD32	type;
{
    CARD32      format;
    int		size;

    if (!pcfSeekToType(file, tables, ntables, type, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
	!PCF_FORMAT_MATCH(format, PCF_ACCEL_W_INKBOUNDS)) 
    {
	goto Bail;
    }
    pFontInfo->noOverlap = pcfGetINT8(file, format);
    pFontInfo->constantMetrics = pcfGetINT8(file, format);
    pFontInfo->terminalFont = pcfGetINT8(file, format);
    pFontInfo->constantWidth = pcfGetINT8(file, format);
    pFontInfo->inkInside = pcfGetINT8(file, format);
    pFontInfo->inkMetrics = pcfGetINT8(file, format);
    pFontInfo->drawDirection = pcfGetINT8(file, format);
    pFontInfo->anamorphic = FALSE;
     /* natural alignment */ pcfGetINT8(file, format);
    pFontInfo->fontAscent = pcfGetINT32(file, format);
    pFontInfo->fontDescent = pcfGetINT32(file, format);
    pFontInfo->maxOverlap = pcfGetINT32(file, format);
    pcfGetMetric(file, format, &pFontInfo->minbounds);
    pcfGetMetric(file, format, &pFontInfo->maxbounds);
    if (PCF_FORMAT_MATCH(format, PCF_ACCEL_W_INKBOUNDS)) {
	pcfGetMetric(file, format, &pFontInfo->ink_minbounds);
	pcfGetMetric(file, format, &pFontInfo->ink_maxbounds);
    } else {
	pFontInfo->ink_minbounds = pFontInfo->minbounds;
	pFontInfo->ink_maxbounds = pFontInfo->maxbounds;
    }
    return TRUE;
Bail:
    return FALSE;
}

int
pcfReadFont(pFont, file, bit, byte, glyph, scan)
    FontPtr     pFont;
    FontFilePtr file;
    int         bit,
                byte,
                glyph,
                scan;
{
    CARD32      format;
    CARD32      size;
    BitmapFontPtr  bitmapFont = 0;
    int         i;
    PCFTablePtr tables = 0;
    int         ntables;
    int         nmetrics;
    int         nbitmaps;
    int         sizebitmaps;
    int         nink_metrics;
    CharInfoPtr metrics = 0;
    xCharInfo  *ink_metrics = 0;
    char       *bitmaps = 0;
    CharInfoPtr *encoding = 0;
    int         nencoding;
    int         encodingOffset;
    CARD32      bitmapSizes[GLYPHPADOPTIONS];
    CARD32     *offsets = 0;
    Bool	hasBDFAccelerators;

    pFont->info.props = 0;
    if (!(tables = pcfReadTOC(file, &ntables)))
	goto Bail;

    /* properties */

    if (!pcfGetProperties(&pFont->info, file, tables, ntables))
	goto Bail;

    /* Use the old accelerators if no BDF accelerators are in the file */

    hasBDFAccelerators = pcfHasType (tables, ntables, PCF_BDF_ACCELERATORS);
    if (!hasBDFAccelerators)
	if (!pcfGetAccel (&pFont->info, file, tables, ntables, PCF_ACCELERATORS))
	    goto Bail;

    /* metrics */

    if (!pcfSeekToType(file, tables, ntables, PCF_METRICS, &format, &size)) {
	goto Bail;
    }
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
	    !PCF_FORMAT_MATCH(format, PCF_COMPRESSED_METRICS)) {
	goto Bail;
    }
    if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	nmetrics = pcfGetINT32(file, format);
    else
	nmetrics = pcfGetINT16(file, format);
    metrics = (CharInfoPtr) xalloc(nmetrics * sizeof(CharInfoRec));
    if (!metrics) {
	goto Bail;
    }
    for (i = 0; i < nmetrics; i++)
	if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	    pcfGetMetric(file, format, &(metrics + i)->metrics);
	else
	    pcfGetCompressedMetric(file, format, &(metrics + i)->metrics);

    /* bitmaps */

    if (!pcfSeekToType(file, tables, ntables, PCF_BITMAPS, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;

    nbitmaps = pcfGetINT32(file, format);
    if (nbitmaps != nmetrics)
	goto Bail;

    offsets = (CARD32 *) xalloc(nbitmaps * sizeof(CARD32));
    if (!offsets)
	goto Bail;

    for (i = 0; i < nbitmaps; i++)
	offsets[i] = pcfGetINT32(file, format);

    for (i = 0; i < GLYPHPADOPTIONS; i++)
	bitmapSizes[i] = pcfGetINT32(file, format);
    sizebitmaps = bitmapSizes[PCF_GLYPH_PAD_INDEX(format)];
    bitmaps = (char *) xalloc(sizebitmaps);
    if (!bitmaps)
	goto Bail;
    FontFileRead(file, bitmaps, sizebitmaps);
    position += sizebitmaps;

    if (PCF_BIT_ORDER(format) != bit)
	BitOrderInvert(bitmaps, sizebitmaps);
    if ((PCF_BYTE_ORDER(format) == PCF_BIT_ORDER(format)) != (bit == byte)) {
	switch (bit == byte ? PCF_SCAN_UNIT(format) : scan) {
	case 1:
	    break;
	case 2:
	    TwoByteSwap(bitmaps, sizebitmaps);
	    break;
	case 4:
	    FourByteSwap(bitmaps, sizebitmaps);
	    break;
	}
    }
    if (PCF_GLYPH_PAD(format) != glyph) {
	char       *padbitmaps;
	int         sizepadbitmaps;
	int         old,
	            new;
	xCharInfo  *metric;

	sizepadbitmaps = bitmapSizes[PCF_SIZE_TO_INDEX(glyph)];
	padbitmaps = (char *) xalloc(sizepadbitmaps);
	if (!padbitmaps) {
	    goto Bail;
	}
	new = 0;
	for (i = 0; i < nbitmaps; i++) {
	    old = offsets[i];
	    metric = &metrics[i].metrics;
	    offsets[i] = new;
	    new += RepadBitmap(bitmaps + old, padbitmaps + new,
			       PCF_GLYPH_PAD(format), glyph,
			  metric->rightSideBearing - metric->leftSideBearing,
			       metric->ascent + metric->descent);
	}
	xfree(bitmaps);
	bitmaps = padbitmaps;
    }
    for (i = 0; i < nbitmaps; i++)
	metrics[i].bits = bitmaps + offsets[i];

    xfree(offsets);

    /* ink metrics ? */

    ink_metrics = NULL;
    if (pcfSeekToType(file, tables, ntables, PCF_INK_METRICS, &format, &size)) {
	format = pcfGetLSB32(file);
	if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT) &&
		!PCF_FORMAT_MATCH(format, PCF_COMPRESSED_METRICS)) {
	    goto Bail;
	}
	if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	    nink_metrics = pcfGetINT32(file, format);
	else
	    nink_metrics = pcfGetINT16(file, format);
	if (nink_metrics != nmetrics)
	    goto Bail;
	ink_metrics = (xCharInfo *) xalloc(nink_metrics * sizeof(xCharInfo));
	if (!ink_metrics)
	    goto Bail;
	for (i = 0; i < nink_metrics; i++)
	    if (PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
		pcfGetMetric(file, format, ink_metrics + i);
	    else
		pcfGetCompressedMetric(file, format, ink_metrics + i);
    }

    /* encoding */

    if (!pcfSeekToType(file, tables, ntables, PCF_BDF_ENCODINGS, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;

    pFont->info.firstCol = pcfGetINT16(file, format);
    pFont->info.lastCol = pcfGetINT16(file, format);
    pFont->info.firstRow = pcfGetINT16(file, format);
    pFont->info.lastRow = pcfGetINT16(file, format);
    pFont->info.defaultCh = pcfGetINT16(file, format);

    nencoding = (pFont->info.lastCol - pFont->info.firstCol + 1) *
	(pFont->info.lastRow - pFont->info.firstRow + 1);

    encoding = (CharInfoPtr *) xalloc(nencoding * sizeof(CharInfoPtr));
    if (!encoding)
	goto Bail;

    pFont->info.allExist = TRUE;
    for (i = 0; i < nencoding; i++) {
	encodingOffset = pcfGetINT16(file, format);
	if (encodingOffset == 0xFFFF) {
	    pFont->info.allExist = FALSE;
	    encoding[i] = 0;
	} else
	    encoding[i] = metrics + encodingOffset;
    }

    /* BDF style accelerators (i.e. bounds based on encoded glyphs) */

    if (hasBDFAccelerators)
	if (!pcfGetAccel (&pFont->info, file, tables, ntables, PCF_BDF_ACCELERATORS))
	    goto Bail;

    bitmapFont = (BitmapFontPtr) xalloc(sizeof *bitmapFont);
    if (!bitmapFont)
	goto Bail;

    bitmapFont->version_num = PCF_FILE_VERSION;
    bitmapFont->num_chars = nmetrics;
    bitmapFont->num_tables = ntables;
    bitmapFont->metrics = metrics;
    bitmapFont->ink_metrics = ink_metrics;
    bitmapFont->bitmaps = bitmaps;
    bitmapFont->encoding = encoding;
    bitmapFont->pDefault = (CharInfoPtr) 0;
    if (pFont->info.defaultCh != (unsigned short) NO_SUCH_CHAR) {
	int         r,
	            c,
	            cols;

	r = pFont->info.defaultCh >> 8;
	c = pFont->info.defaultCh & 0xFF;
	if (pFont->info.firstRow <= r && r <= pFont->info.lastRow &&
		pFont->info.firstCol <= c && c <= pFont->info.lastCol) {
	    cols = pFont->info.lastCol - pFont->info.firstCol + 1;
	    r = r - pFont->info.firstRow;
	    c = c - pFont->info.firstCol;
	    bitmapFont->pDefault = encoding[r * cols + c];
	}
    }
    bitmapFont->bitmapExtra = (BitmapExtraPtr) 0;
    pFont->fontPrivate = (pointer) bitmapFont;
#if defined( X11R5BOS )
    pFont->get_glyphs = bitmapGetGlyphs;
#else /* X11R5BOS */
    pFont->get_bitmaps = bitmapGetBitmaps;
    pFont->get_extents = bitmapGetExtents;
    pFont->get_glyphs = bitmapGetGlyphs;
    pFont->get_metrics = bitmapGetMetrics;
#endif /* X11R5BOS */
    pFont->unload_font = pcfUnloadFont;
    pFont->bit = bit;
    pFont->byte = byte;
    pFont->glyph = glyph;
    pFont->scan = scan;
    xfree(tables);
    return Successful;
Bail:
    xfree(ink_metrics);
    xfree(encoding);
    xfree(bitmaps);
    xfree(offsets);
    xfree(metrics);
    xfree(pFont->info.props);
    pFont->info.props = 0;
    xfree(bitmapFont);
    xfree(tables);
    return AllocError;
}
#if !defined( X11R5BOS )
int
pcfReadFontInfo(pFontInfo, file)
    FontInfoPtr pFontInfo;
    FontFilePtr file;
{
    PCFTablePtr tables;
    int         ntables;
    CARD32      format;
    CARD32      size;
    int         nencoding;
    Bool	hasBDFAccelerators;

    if (!(tables = pcfReadTOC(file, &ntables)))
	goto Bail;

    /* properties */

    if (!pcfGetProperties(pFontInfo, file, tables, ntables))
	goto Bail;

    /* Use the old accelerators if no BDF accelerators are in the file */

    hasBDFAccelerators = pcfHasType (tables, ntables, PCF_BDF_ACCELERATORS);
    if (!hasBDFAccelerators)
	if (!pcfGetAccel (pFontInfo, file, tables, ntables, PCF_ACCELERATORS))
	    goto Bail;

    /* encoding */

    if (!pcfSeekToType(file, tables, ntables, PCF_BDF_ENCODINGS, &format, &size))
	goto Bail;
    format = pcfGetLSB32(file);
    if (!PCF_FORMAT_MATCH(format, PCF_DEFAULT_FORMAT))
	goto Bail;

    pFontInfo->firstCol = pcfGetINT16(file, format);
    pFontInfo->lastCol = pcfGetINT16(file, format);
    pFontInfo->firstRow = pcfGetINT16(file, format);
    pFontInfo->lastRow = pcfGetINT16(file, format);
    pFontInfo->defaultCh = pcfGetINT16(file, format);

    nencoding = (pFontInfo->lastCol - pFontInfo->firstCol + 1) *
	(pFontInfo->lastRow - pFontInfo->firstRow + 1);

    pFontInfo->allExist = TRUE;
    while (nencoding--) {
	if (pcfGetINT16(file, format) == 0xFFFF)
	    pFontInfo->allExist = FALSE;
    }

    /* BDF style accelerators (i.e. bounds based on encoded glyphs) */

    if (hasBDFAccelerators)
	if (!pcfGetAccel (pFontInfo, file, tables, ntables, PCF_BDF_ACCELERATORS))
	    goto Bail;

    xfree(tables);
    return Successful;
Bail:
    xfree(tables);
    return AllocError;
}
#endif /* X11R5BOS */
void
pcfUnloadFont(pFont)
    FontPtr     pFont;
{
    BitmapFontPtr  bitmapFont;

    bitmapFont = (BitmapFontPtr) pFont->fontPrivate;
    xfree(bitmapFont->ink_metrics);
    xfree(bitmapFont->encoding);
#ifdef R4DDX
    if (!use_conversion)
    xfree(bitmapFont->bitmaps);
#else
    xfree(bitmapFont->bitmaps);
#endif
    xfree(bitmapFont->metrics);
    xfree(pFont->info.isStringProp);
    xfree(pFont->info.props);
    xfree(bitmapFont);
    xfree(pFont);
}
