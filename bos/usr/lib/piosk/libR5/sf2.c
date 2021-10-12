static char sccsid[] = "@(#)17  1.2  src/bos/usr/lib/piosk/libR5/sf2.c, cmdpiosk, bos411, 9428A410j 9/14/93 00:50:34";
/*
 *   COMPONENT_NAME: CMDPIOSK
 *
 *   FUNCTIONS: get_Font init_Font open_Font
 *
 *   ORIGINS: 16,27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <locale.h>
#include "X11/X.h"
#include "servermd.h"
#include "fontfilest.h"
#include "bitmap.h"
#include "fontstruct.h"
#include "jfmtrs_msg.h"
#include "lc.h"
#include "ffaccess.h"

static Mask     FontFormat =
#if IMAGE_BYTE_ORDER == LSBFirst
BitmapFormatByteOrderLSB |
#else
BitmapFormatByteOrderMSB |
#endif

#if BITMAP_BIT_ORDER == LSBFirst
BitmapFormatBitOrderLSB |
#else
BitmapFormatBitOrderMSB |
#endif

BitmapFormatImageRectMin |

#if GLYPHPADBYTES == 1
BitmapFormatScanlinePad8 |
#endif

#if GLYPHPADBYTES == 2
BitmapFormatScanlinePad16 |
#endif

#if GLYPHPADBYTES == 4
BitmapFormatScanlinePad32 |
#endif

#if GLYPHPADBYTES == 8
BitmapFormatScanlinePad64 |
#endif

BitmapFormatScanlineUnit8;


typedef struct _BitmapMethods{
	char*	suffix;
	int	(*ReadFont)( /* pFont, file, bit, byte, glyph, scan */ );
} BitmapMethodsRec, *BitmapMethods;


extern int	pcfReadFont(), snfReadFont();
static struct _BitmapMethods	BMPtr[5] = {
	{ ".pcf",	pcfReadFont },
	{ ".snf",	snfReadFont },
#if defined( COMPRESSED_FONTS )
	{ ".pcf.Z",	pcfReadFont },
	{ ".snf.Z",	snfReadFont },
#endif	/* COMPRESSED_FONTS */
	{ 0,		0 }
};

/*
 * NAME: open_Font
 *
 * FUNCTION:	Open the specified font file and return the font file pointer.
 *
 * EXECUTION ENVIRONMENT: Called from init_Font() or get_pFont();
 *
 * NOTES:
 *
 * RETURNS: unsigned long (FontPtr)
 *              FontPtr (Success)
 *              Null    (fail)
 */

unsigned long open_Font( char* fontfile )
{
	FONTinfo	p;
	BitmapMethods	bmf;
	FontPtr		pFont;
	FontPropPtr	ptr;
	int	i;
	int	fmask = BitmapFormatMaskByte |
			BitmapFormatMaskBit |
			BitmapFormatMaskImageRectangle |
			BitmapFormatMaskScanLinePad |
			BitmapFormatMaskScanLineUnit;
	char*	name;
	char*	value;
	char*	cs_reg;
	char*	cs_enc;
	extern int BitmapOpenBitmap(FontPtr* ppFont, 
				    char* fontfile, 
				    fsBitmapFormat format, 
				    fsBitmapFormatMask fmask, 
				    int (*readfont)() );

	if( !fontfile ) 
	    return (NULL);
	for( bmf = BMPtr; bmf->suffix != 0; bmf++ ){
		i = strlen( fontfile ) - strlen( bmf->suffix );
		if( i >= 0 && !strcmp( bmf->suffix, fontfile + i ) ){
			if( BitmapOpenBitmap( &pFont, fontfile, FontFormat, fmask, bmf->ReadFont ) != Successful )
				return (NULL);
			else 
				return ((unsigned long)pFont);
		}
	}
}
	
/*
 * NAME: open_Font
 *
 * FUNCTION:	If the specifed font-file's registory and encoding 
 *              match to one of the required font files, save its font pointer
 *              to the struct FONTinfo.
 *
 * EXECUTION ENVIRONMENT: This funcion is also called from cmdtext module.
 *
 * NOTES:
 *
 * RETURNS: void
 */

void	init_Font( char* fontfile, FONTinfo fip )
{
    FONTinfo	p;
    FontPtr		pFont;
    FontPropPtr	ptr;
    int	i;
    char*	name;
    char*	value;
    char*	cs_reg;
    char*	cs_enc;
    static int strcmp_al( char* str1, char* str2 );
    
    if( !fontfile ) return;
    if (!(pFont = open_Font (fontfile))){
	return;
    }
    cs_reg = cs_enc = 0;
    
    for( i = 0, ptr = pFont->info.props; i < pFont->info.nprops; i++, ptr++ ){
	name = NameForAtom( ptr->name );
	value = pFont->info.isStringProp[i] ?
	    NameForAtom( ptr->value ) : "";
	if( strcmp( CSREGISTRY, name ) == 0 ){
	    cs_reg = value;
	}else if( strcmp( CSENCODING, name ) == 0 ){
	    cs_enc = value;
	}
    }
    if( cs_enc == 0 && cs_reg == 0 ){
	return;
    }
    
    for( p = fip; p->cs_registry || p->cs_encoding; p++ ){
	if( strcmp_al( p->cs_registry, cs_reg ) == 0 &&
	   strcmp_al( p->cs_encoding, cs_enc ) == 0 ){
	    p->pFont = (unsigned long)pFont;
	    p->flags = FOUND;
	    break;
	}
    }
    
    return;
}


/* string compare without distinction of upper/lower */
static int	strcmp_al( char* s1, char* s2 )
{
    register int	c1, c2;
    register int	d0 = 'a' - 'A';
    
    if( s1 == 0 || s2 == 0 )	return( s1 - s2 );
    
    for( ; ( c1 = *s1 ) && ( c2 = *s2 ); s1++, s2++ ){
	if( c1 != c2 ){
	    if( ( c2 < 'A' || c2 > 'Z' || c1 - c2 != d0 ) &&
	       ( c1 < 'A' || c1 > 'Z' || c2 - c1 != d0 ) ){
		return( 1 );
	    }
	}
    }
    
    return( c1 - *s2 );
}

Fontimg	get_Font( unsigned char* chars, unsigned long pfont, int wid, int pad)
{
    int	i,j,bt;
    static Fontimgrec	fontimg;
    CharInfoPtr	pci;
    FontPtr	pFont = (FontPtr)pfont;
    static void rmpad( CharInfoPtr pci, Fontimg f, 
		      int top, int bot, int left, int right );
    
    if( chars == 0 ) return( 0 );
    
    if( pFont == 0 || pFont->get_glyphs == 0 ){
	fontimg.w = SEND_CODEVALUE;
	strcpy( fontimg.img, chars );
	return( &fontimg );
    }
    
    if( chars[1] == 0 ){
	chars[1] = chars[0];
	chars[0] = chars[2] = 0;
    }
    
    
    (*pFont->get_glyphs)( pFont, 1, chars, TwoD16Bit, &i, &pci );
    
    if( i == 0 ){
	fontimg.w = SEND_CODEVALUE;
	for( i = 0; i < wid; i++ )	fontimg.img[i] = ' ';
	fontimg.img[i] = 0;
	return( &fontimg );
    }
    
    fontimg.w = pci->metrics.rightSideBearing - pci->metrics.leftSideBearing;
    if( fontimg.w <= 0 || fontimg.w > MAXFONTSIZE ) return( 0 );
    
    fontimg.baseline = pci->metrics.descent;
    fontimg.h = pci->metrics.ascent + pci->metrics.descent;
    if( fontimg.h <= 0 || fontimg.h > MAXFONTSIZE ) return( 0 );
    
    if (pad){	/* for cmdpiosk */
	if ((24 <= fontimg.h && fontimg.h < 32) || pad == PR_PAD )
	    rmpad( pci, &fontimg, 1, 2, 1, wid == 1 ? 0 : 1 );
	/*
	 * Followings for 55xx exception (pad == GG_PAD)
	 */
	else if ( fontimg.h < 24) 
	    rmpad(pci, &fontimg, fontimg.h-24, 0, 0,
		  wid == 1 ? fontimg.w-12 : fontimg.w-24);
	else 
	    rmpad(pci, &fontimg, fontimg.h-24, 0, 0, 
		  wid == 1 ? fontimg.w-12 : fontimg.w-24);
    }
    else { /* if (pad == NO_PAD)  for cmdtext */
	for ( i = 0; i < (fontimg.h + 1) * (fontimg.bytes_in_line + 1); i++) {
	    fontimg.img[i] = pci->bits[i];
	}
    }
    return( &fontimg );
}



/*
 * NAME: rmpad
 *
 * FUNCTION: Remove unnecessary padded 0's surrounding the font
 *
 * EXECUTION ENVIRONMENT: Called from getfont().
 *
 * (NOTE:)
 *
 * (RECOVERY OPERATION:)
 *
 * (DATA STRUCTURES:)
 *
 * RETURNS: NONE
 *
 */

void rmpad( CharInfoPtr pci, Fontimg f, int top, int bot, int left, int right )
{
    int	c, i, j, k, n;
    int	bpr, bil;
    int	w, h;
    int	xleft;
    int	lastmask;
    int top_pad = 0;
    unsigned char* glyph = pci->bits;
    
    
    if( glyph == 0 || f == 0 ) return;
    if( top < 0 ){
	top_pad = -top;
	top = 0;
    }
    if( bot < 0 )	bot = 0;
    if( left < 0 )	left = 0;
    if( right < 0 )	right = 0;

    w = f->w;
    h = f->h;
    bpr = BYTES_PER_ROW( GLYPHWIDTHPIXELS( pci ), 1 );
    /*
      bpr = BYTES_PER_ROW( GLYPHWIDTHPIXELS( pci ), GLYPHPADOPTIONS );
      bpr = GLWIDTHBYTESPADDED( f->w, DEFAULTGLPAD);
      */
    /* bytes per row of snf glyph data                              */
    /* assume default padding(=1) was used when snftobdf convering. */
    /* bpr = 4 when 24x24 is used */
    
    f->w = f->w - ( left + right );
    f->h = f->h - ( top - top_pad + bot );
    f->baseline -= bot;
    f->bytes_in_line = bil = GLYPHWIDTHBYTES( pci );
    /*
      f->bytes_in_line = bil = ( ( f->w + 7 ) >> 3 );
      */
    /* char width in byte  */
    n = left / 8;
    left %= 8;
    glyph += bpr * top;
    lastmask = 0xff - ( 0xff >> ( f->w % 8 ) );
/*    if( lastmask == 0 )	lastmask = 0xff;*/

    for( j = 0; j < top_pad; j++){
	for( k = 0; k < bil; k++ ){	    
	    f->img[ j * bil + k ] = 0;    
	}
    }

    if( left != 0 ){
	xleft = 8 - left;
	glyph += n;
	for(; j < f->h; j++, glyph += bpr ){
	    for( k = 0; k < bil; k++ ){
		c = 0xff & ( glyph[ k ] << left );
		c |= ( glyph[ k + 1 ] >> xleft );
		f->img[ j * bil + k ] = (unsigned char)c;
	    }
	    f->img[ j * bil + bil - 1 ] &= lastmask;
	    f->img[ j * bil + bil ] = 0;
	}
    }else{
	for(; j < f->h; j++, glyph += bpr ){
	    for( k = 0; k < bil; k++ ){
		f->img[ j*bil + k ] = glyph[ k ];
	    }
	    
	    f->img[ j * bil + bil - 1 ] &= lastmask;
	}
    }
}  /* end of rmpad() */


#if defined( xalloc )
# undef xalloc
#endif
#if defined( xfree )
# undef xfree
#endif
#if defined( xrealloc )
# undef xrealloc
#endif
#if defined( Xalloc )
# undef Xalloc
#endif
#if defined( Xfree )
# undef Xfree
#endif
#if defined( Xrealloc )
# undef Xrealloc
#endif

xalloc( int x )
{
	return( malloc( x ) );
}

Xalloc( int x )
{
	return( malloc( x ) );
}

xrealloc( int x, int y )
{
	return( x ? realloc( x, y ) : malloc( y ) );
}

Xrealloc( int x, int y )
{
	return( x ? realloc( x, y ) : malloc( y ) );
}

xfree( int x )
{
	free( x );
}

Xfree( int x )
{
	free( x );
}
