static char sccsid[] = "@(#)85  1.1  src/bos/usr/lib/piosk/libR5/bitmapfuncs.c, cmdpiosk, bos411, 9428A410j 7/31/92 21:21:31";
#if !defined( X11R5BOS )
#ifndef lint
static char sccsid[] = "@(#)00	1.1  com/XTOP/fonts/lib/font/bitmap/bitmapfuncs.c, xfont, gos323, X11R5a 5/13/92 17:06:37";
#endif
#endif /* X11R5BOS */
/*
 *  COMPONENT_NAME: 
 *
 *  FUNCTIONS: 
 *
 *  ORIGINS: 16,27 
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
 * $XConsortium: bitmapfuncs.c,v 1.3 91/06/12 14:35:17 keith Exp $
 *
 * Copyright 1991 Massachusetts Institute of Technology
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

#ifdef COMPRESSED_FONTS
#include <sys/wait.h>
#include <errno.h>
#endif /* COMPRESSED_FONTS */
#if !defined( X11R5BOS )
typedef struct _BitmapFileFunctions {
    int         (*ReadFont) ( /* pFont, file, bit, byte, glyph, scan */ );
    int         (*ReadInfo) ( /* pFontInfo, file */ );
}           BitmapFileFunctionsRec, *BitmapFileFunctionsPtr;

extern int  pcfReadFont(), pcfReadFontInfo();
extern int  snfReadFont(), snfReadFontInfo();
extern int  bdfReadFont(), bdfReadFontInfo();
int	    BitmapOpenBitmap ();
extern int  BitmapOpenScalable ();
int	    BitmapGetInfoBitmap ();
extern int  BitmapGetInfoScalable ();

/*
 * these two arrays must be in the same order
 */
static BitmapFileFunctionsRec readers[] = {
    pcfReadFont, pcfReadFontInfo,

#ifdef COMPRESSED_FONTS
/* compressed fonts have to be split up this way because of precedence rules */
    pcfReadFont, pcfReadFontInfo,
#endif /* COMPRESSED_FONTS */

    snfReadFont, snfReadFontInfo,

#ifdef COMPRESSED_FONTS
    snfReadFont, snfReadFontInfo,
#endif /* COMPRESSED_FONTS */

    bdfReadFont, bdfReadFontInfo,

#ifdef COMPRESSED_FONTS
/*
compressed bdf files are not in the design, so hamstring this code
    bdfReadFont, bdfReadFontInfo,
*/
#endif /* COMPRESSED_FONTS */
};

static FontRendererRec	renderers[] = {
    ".pcf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,

#ifdef COMPRESSED_FONTS
/* compressed fonts have to be split up this way because of precedence rules */
    ".pcf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
#endif /* COMPRESSED_FONTS */

    ".snf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,

#ifdef COMPRESSED_FONTS
    ".snf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
#endif /* COMPRESSED_FONTS */

    ".bdf", 4,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,

#ifdef COMPRESSED_FONTS
/*
compressed bdf files are not in the design, so hamstring this code
    ".bdf.Z", 6,
    BitmapOpenBitmap, BitmapOpenScalable,
	BitmapGetInfoBitmap, BitmapGetInfoScalable, 0,
*/
#endif /* COMPRESSED_FONTS */
};
#endif /* X11R5BOS */
#ifdef COMPRESSED_FONTS
/*
	A chunk of this uncompress code was modified
	from the X11R4 version of mkfontdir.c

	FUNCTIONS: mkfontdir.c
	ORIGINS: 27, 16
*/
#ifndef UNCOMPRESSFILT
#define UNCOMPRESSFILT "/usr/bin/uncompress"
#endif

/* gotta choose some name */
#define TMPPATTERN	"/tmp/enya.%05d.XXXXXX"

static char * filter[] = {UNCOMPRESSFILT, NULL};

char * uncompress_this_file(FILE *file)
{
    int fdtmp;
    int pid;
    char *tmpname;

    tmpname = xalloc(sizeof(TMPPATTERN)+5+1);
    if( ! tmpname )
    {
	fclose(file);
	return NULL;
    }
    sprintf(tmpname,TMPPATTERN,getpid());
    if( ! mktemp(tmpname) )
    {
	xfree(tmpname);
	fclose(file);
	return NULL;
    }
    if( ! *tmpname )
    {
	xfree(tmpname);
	fclose(file);
	return NULL;
    }
    fdtmp = open(tmpname,(O_RDWR|O_EXCL|O_CREAT),0600);
    if( fdtmp < 0 )
    {
	xfree(tmpname);
	fclose(file);
	return NULL;
    }

    pid=fork();
    if( pid < 0 )
    {
	/* an error */
	close(fdtmp);
	unlink(tmpname);
	xfree(tmpname);
	fclose(file);
	return NULL;
    }
    if( pid == 0 )
    {
	/* this is the child */
	dup2(fileno(file), 0);	/* Point the fd of stdin */
				/* at the file opened by the caller. */
				/* This will cause reads of stdin */
				/* by uncompress to read from the */
				/* file. */
	close(fileno(file));	/* No longer needed. Close the fd */
				/* for the file pointer. */
	dup2(fdtmp,1);	/* Point the fd of stdout at the */
			/* temporary file.  This will cause */
			/* the output of uncompress to be put */
			/* into the temporary file. */
	close(fdtmp);	/* No longer needed. */
	execvp(filter[0], filter);	/* polymorph into uncompress */
	_exit(127);	/* in case of an error */
    }

	/* this is the parent */

	/* wait until the child is done */
    {
	int status;	/* return value, not used */
	int result;

	while( (result=waitpid(pid, &status, 0)) != pid )
	{
	    if( result < 0 && errno != EINTR )
	    {
		/* Oh, no!  Some error we can't handle. */
		close(fdtmp);
		unlink(tmpname);
		xfree(tmpname);
		fclose(file);
		return NULL;
	    }
	}
    }

/*
	The child process has its own copy of the parent process's file descriptors.  However, each
	of the child's file descriptors shares a common file pointer with the corresponding file
	descriptor of the parent process.
*/
    lseek(fdtmp,0,SEEK_SET);

    dup2(fdtmp, fileno(file));	/* Point the fd of the file pointer at */
				/* the temporary file.  This will */
				/* cause reads on the file pointer */
				/* to read from the temporary file. */
    close(fdtmp);	/* No longer needed. */
    return tmpname;
}
#endif /* COMPRESSED_FONTS */
#if defined( X11R5BOS )
BitmapOpenBitmap( FontPtr* ppFont, char* fileName, fsBitmapFormat format, fsBitmapFormatMask fmask, int (*ReadFont)() )
#else /* X11R5BOS */
BitmapOpenBitmap (fpe, ppFont, flags, entry, fileName, format, fmask)
    FontPathElementPtr	fpe;
    FontPtr		*ppFont;
    int			flags;
    FontEntryPtr	entry;
    char		*fileName;
    fsBitmapFormat	format;
    fsBitmapFormatMask	fmask;
#endif /* X11R5BOS */
{
    FILE       *file;
    FontPtr     pFont;
    int         i;
    int         ret;
    int         bit,
                byte,
                glyph,
                scan,
		image;
#ifdef COMPRESSED_FONTS
    char *tmp_file_name = 0;
#endif /* COMPRESSED_FONTS */
#if !defined( X11R5BOS )
    /*
     * compute offset into renderers array - same offset is
     * useful in the file functions array
     */
    i = entry->u.bitmap.renderer - renderers;
#endif	/* X11R5BOS */
    file = fopen(fileName, "r");
    if (!file)
	return BadFontName;
    pFont = (FontPtr) xalloc(sizeof(FontRec));
    if (!pFont) {
	fclose(file);
	return AllocError;
    }

#ifdef COMPRESSED_FONTS
    {
	int l = strlen(fileName);
	if( l > 2 && fileName[l-2] == '.' && fileName[l-1] == 'Z' )
	{
	    tmp_file_name = uncompress_this_file(file);
	    if( ! tmp_file_name )
	    {
		/* Something bad happened. Oh, well. */
		/* assume that the file was fclose()ed */
		return BadFontName;
	    }
	}
    }
#endif /* COMPRESSED_FONTS */

    /* set up default values */
    FontDefaultFormat(&bit, &byte, &glyph, &scan);
    /* get any changes made from above */
    ret = CheckFSFormat(format, fmask, &bit, &byte, &scan, &glyph, &image);

    /* Fill in font record. Data format filled in by reader. */
    pFont->refcnt = 0;
    pFont->maxPrivate = -1;
    pFont->devPrivates = (pointer *) 0;

#if defined( X11R5BOS )
    ret = (*ReadFont) (pFont, file, bit, byte, glyph, scan);
#else /* X11R5BOS */
    ret = (*readers[i]->ReadFont) (pFont, file, bit, byte, glyph, scan);
#endif /* X11R5BOS */
    fclose(file);

#ifdef COMPRESSED_FONTS
    if( tmp_file_name )
    {
	unlink(tmp_file_name);
	xfree(tmp_file_name);
    }
#endif /* COMPRESSED_FONTS */

    if (ret != Successful)
	xfree(pFont);
    else
	*ppFont = pFont;
    return ret;
}
#if !defined( X11R5BOS )
BitmapGetInfoBitmap (fpe, pFontInfo, entry, fileName)
    FontPathElementPtr	fpe;
    FontInfoPtr		pFontInfo;
    FontEntryPtr	entry;
    char		*fileName;
{
    FILE    *file;
    int	    i;
    int	    ret;
    FontRendererPtr renderer;
#ifdef COMPRESSED_FONTS
    char *tmp_file_name = 0;
#endif /* COMPRESSED_FONTS */

    renderer = FontFileMatchRenderer (fileName);
    if (!renderer)
	return BadFontName;
    i = renderer - renderers;
    file = fopen (fileName, "r");
    if (!file)
	return BadFontName;

#ifdef COMPRESSED_FONTS
    {
	int l = strlen(fileName);
	if( l > 2 && fileName[l-2] == '.' && fileName[l-1] == 'Z' )
	{
	    tmp_file_name = uncompress_this_file(file);
	    if( ! tmp_file_name )
	    {
		/* Something bad happened. Oh, well. */
		/* assume that the file was fclose()ed */
		return BadFontName;
	    }
	}
    }
#endif /* COMPRESSED_FONTS */

    ret = (*readers[i].ReadInfo) (pFontInfo, file);
    fclose (file);

#ifdef COMPRESSED_FONTS
    if( tmp_file_name )
    {
	unlink(tmp_file_name);
	xfree(tmp_file_name);
    }
#endif /* COMPRESSED_FONTS */

    return ret;
}

#define numRenderers	(sizeof renderers / sizeof renderers[0])

BitmapRegisterFontFileFunctions ()
{
    int	    i;

    for (i = 0; i < numRenderers; i++)
	FontFileRegisterRenderer (&renderers[i]);
}
#endif /* X11R5BOS */
