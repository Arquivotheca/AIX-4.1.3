static char sccsid[] = "@(#)94  1.3  src/bos/usr/lib/piosk/libR5/sf.c, cmdpiosk, bos411, 9428A410j 9/14/93 00:50:27";
/*
 *   COMPONENT_NAME: CMDPIOSK
 *
 *   FUNCTIONS: 
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
    
static FontDirectoryPtr*	pdir = 0;

/*
 * NAME: init_Fontpath
 *
 * FUNCTION: Parse the specified list of fontpath and 
 *           call fontpathel() to initialize
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: void
 */

void	init_Fontpath( char* fontpath )
{
    static int	init_Fontpathel( char* fontpathel );
    
    FontTablePtr tbl;
    int d, e;

    if( fontpath ){
	char*	p;
	char*	x;
	int	i = strlen( fontpath ) + 1;
	
	if( i <= 0 || ( p = calloc( sizeof( char ), i ) ) == 0 ){
	    return;
	}
	
	strncpy( p, fontpath, i );
	
	for( x = p; *x; x++ ){
	    if( *x == ',' ){
		if( x != p ){
		    *x = 0;
		    init_Fontpathel( p );
		}
		p = x + 1;
	    }
	}
	if( x != p )
	    init_Fontpathel( p );
    }


}


/*
 * NAME: init_Fontpathel
 *
 * FUNCTION: Initialize the specified 
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RETURNS: int NULL (failure)
 *              1 (success) 
 */

static int init_Fontpathel( char* fontpathel )
{
    FontDirectoryPtr	p;
    int	i = 0;
	
    if (FontFileReadDirectory (fontpathel, &p) != Successful){
	return NULL;
	/*
	  errorexit (MSG_BADUSERFONT1, "");
	  */
    }
    
    if( pdir == 0 ){
	if ((pdir = calloc (sizeof (FontDirectoryPtr), 2)) == 0){
	    return NULL;
	    /*
	      errorexit(MSG_MEMALLOCERR, "");
	      */
	}
    }
    else{
	for (i = 0; pdir[i]; i++);
	if ((pdir = realloc (pdir, sizeof (FontDirectoryPtr) * (i + 1))) == 0){
	    /*
	      errorexit( MSG_MEMALLOCERR, "" );
	      */
	}
	
    }
    pdir[i] = p;
    pdir[i+1] = 0;
}

static char* get_FontFileName (FontDirectoryPtr fndir,
			       char *pattern, char* filepat)
{
    int         i, ff, d, ret,j, ast, fstart, fstop,
                nn, res, s, fprivate, max=100;
    char	*fontfilename, *pstr;
    FontNamePtr name;
    FontTableRec tmptable;
    FontNameRec     pat, fpat,ffn;
    FontEntryPtr	tmpentry;	

    static int start, stop, private;
    static FontTablePtr table;
    static FontDirectoryPtr tmpfndir = NULL;
    
    CopyISOLatin1Lowered (pattern, pattern, strlen(pattern));
    pat.name = pattern;
    pat.length = strlen(pattern);
    pat.ndashes = CountDashes (pat.name, pat.length);
    
    DEBUGPR((stderr, "get_FontFileName\n"));
    
    if (fndir != tmpfndir){
	tmpfndir = fndir;
	DEBUGPR((stderr, "font directory = %s\n", fndir->directory));
	table = &fndir->nonScalable;
	if ((i = SetupWildMatch(table, &pat, &start, &stop, &private)) >= 0){
	    start = stop = i;
	}
    }

    for (i = start; i < stop; i++) {
	/*if (&table->entries[i].type != 2)
	  continue;*/
	name = &table->entries[i].name;
	res = PatternMatch(pat.name, private, name->name, name->ndashes);
	if (res > 0){
	    tmptable.used = 1;
	    tmptable.size = 1;
	    tmptable.sorted = 1;
	    tmptable.entries = &table->entries[i];
	    
	    CopyISOLatin1Lowered (filepat, filepat, strlen(filepat));
	    fpat.name = filepat;
	    fpat.length = strlen(filepat);
	    fpat.ndashes = CountDashes (fpat.name, fpat.length);

	    while(1){
		for (ast = 0, pstr = fpat.name; *pstr; pstr++)
		    if (*pstr != '*')
			break;
		if (*pstr){
		    for (ast = 0, pstr = fpat.name; *pstr; pstr++){
			if (*pstr == '*'){
			    ast = 1;
			    break;
			}
		    }
		    
		    if (ast){
			FontNamesRec names;
			names.nnames = 0;
			names.size = 0;
			names.length = 0;
			names.names = 0;
			
			FontFileFindNamesInDir(table, &fpat, max, &names);
			for (ff = 0; ff < names.nnames; ff++){
			    if(pstr = get_FontFileName(fndir, pattern,
						       names.names[ff]))
				return(pstr);
			}
			break;
		    }
		    else{
			if (tmpentry = FontFileFindNameInDir(table, &fpat)){
			    if (tmpentry->type == 3 ){
				fpat.name = tmpentry->u.alias.resolved;
				fpat.length = strlen(filepat);
				fpat.ndashes = CountDashes (fpat.name, fpat.length);
				continue;
			    }
			    else
				break;
			}
			else
			    break;
		    }
		}
		else {
		    fpat.name[1] = 0;
		    break;
		}
	    }
		
	    if (tmpentry = FontFileFindNameInDir(&tmptable, &fpat)){
		if (tmpentry->type == 2){
		    DEBUGPR((stderr, ">>>> XLFD Name = %s\n\n",
			     table->entries[i].name.name));
		    DEBUGPR((stderr, "File Name = %s\n",
			     table->entries[i].u.bitmap.fileName));
		    if (strlen(fndir->directory) &&
			strlen(table->entries[i].u.bitmap.fileName) &&
			(fontfilename = (char *)malloc(strlen(fndir->directory) + strlen(table->entries[i].u.bitmap.fileName) + 1)) != 0){
			
			strcpy (fontfilename, fndir->directory);
			strcat (fontfilename, 
				table->entries[i].u.bitmap.fileName);
			return (fontfilename);
		    }
		} 
	    }	
	}
	if (res < 0)
	    break;
    }
    return NULL;
}

/*
 * NAME:
 *
 * FUNCTION:	
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */

void	init_Fontlist( char* fontlist, FONTinfo fip )
{
    FONTinfo	tmpfip;
    char**	flist;
    char*       pat;
    char*	p;
    char*	x;
    int	d, flnum, j, f, i = strlen( fontlist ) + 1;
    
    if( fontlist ){
	i = strlen( fontlist ) + 1;
	if( i < 0 || ( p = calloc( sizeof( char ), i ) ) == 0 ){
	    return;
	}
	strncpy( p, fontlist, i );
	
	for( flnum = 0, x = p; *x; x++ ){
	    if( *x == ',' ){
		if( x != p ){
		    *x = 0;
		    flnum++;
		}
		p = x + 1;
	    }
	}
	if( x != p ){
	    flnum++;
	}
	
	if ((flist = (char **) calloc(sizeof(char *), flnum + 1)) == 0){
	    return;
	}
	for( x -= (i - 1), i = 0, j = 0; i < flnum; i++, x+=(strlen(x) + 1)){
	    if (x[0] == '/'){
		init_Font( x, fip );
	    }
	    else{
		flist[j++] = x;
	    }
	}
	flnum = j;
    }
    if (flnum){
	for (tmpfip = fip; tmpfip; tmpfip++){
	    if(tmpfip->flags == FOUND)
		continue;
	    if(!(strlen(tmpfip->cs_registry) && strlen(tmpfip->cs_encoding)) ||
	       (pat = malloc (strlen(tmpfip->cs_registry)
			    +strlen(tmpfip->cs_encoding) + 3)) == 0)
		return;
	    sprintf (pat, "*%s-%s", tmpfip->cs_registry, tmpfip->cs_encoding);


	    for(d = 0; pdir[d] && tmpfip->flags != FOUND; d++){	    
		for (f = 0; f < flnum; f++){
		    tmpfip->filename = get_FontFileName(pdir[d], 
							pat, flist[f]);	
		    if (tmpfip->filename){
			tmpfip->flags = FOUND;
			DEBUGPR((stderr, "tmpfip->filename = %s\n", 
				 tmpfip->filename));
			break;
		    }
		}
	    }
	}
    }
}

/*
Fontimg	lc_get_Font( unsigned char* chars, CURcodeset ccsp )
{
    int	csi = xcsid( chars, ccsp );
    
    if( csi < 0 ) return( (Fontimg)-1 );
    return (get_Font (get_iFont (chars, ccsp, csi), 
		      get_pFont( ccsp, csi ), 
		      get_wid( csi, ccsp ) ,0) );
}
*/

#if defined (DEBUG)

void show_Font( Fontimg x )
{
    int	i,j,bt;
    char* p;
    
    if( x == 0 ) return;
    
    if( x->w == SEND_CODEVALUE ){
	fprintf( stderr, "code:" );
	for( p = x->img; *p; p++ ) fprintf( stderr, "%02x", *p );
	fprintf( stderr, "\n" );
	return;
    }
    
    fprintf( stderr, "x->w = %d\n", x->w );
    for( i = 0; i < x->h; i++ ){
	for( j = 0; j < x->bytes_in_line; j++ ){
	    for( bt = 0x80; bt > 0; bt >>= 1 ){
		fprintf( stderr, 
			bt & x->img[i*x->bytes_in_line+j] ? "**" : ".." );
	    }
	}
	fprintf( stderr, "\n" );
    }
}
#endif /* DEBUG */

void C_Fontimg( Fontimg f_i, Fontimg f_o )
{
	int	wb_i, bil_i, bits_i, mask_i;
	int	wb_o, h_o, xh_o, xb_o, bits_o, mask_o;
	int	i, j, c;
	unsigned char*	pi;
	unsigned char*	pi0;
	unsigned char*	po;


	if( !f_o || f_o->w <= 0 || f_o->bytes_in_line <= 0
	    || f_o->h <= 0 || !f_o->img ||
	    !f_i || f_i->w <= 0 || f_i->bytes_in_line <= 0
	    || f_i->h <= 0 || !f_i->img ){
		return;
	}

	wb_o = f_o->w / 8;
	bits_o = f_o->w - wb_o * 8;
	wb_i = f_i->h / 8;
	bits_i = f_i->h - wb_i * 8;

	if( wb_o > wb_i ){
		wb_o = wb_i;
		bits_o = bits_i;
	}else if( wb_o == wb_i && bits_o > bits_i ){
		bits_o = bits_i;
	}

	xb_o = f_o->bytes_in_line - wb_o - 1;
	bil_i = f_i->bytes_in_line;
	pi0 = f_i->img;
	mask_i = 0x80;
	po = f_o->img;
	h_o = ( f_i->w > f_o->h ) ? f_o->h : f_i->w;
	xh_o = f_o->h - h_o;

	for( i = 0; i < h_o; i++ ){
		for( j = 0, pi = pi0; j < wb_o; j++ ){
			for( c = 0, mask_o = 0x80; mask_o; mask_o >>= 1 ){
				if( mask_i & *pi ) c |= mask_o;
				pi += bil_i;
			}
			*po++ = c;
		}

		for( j = 0, mask_o = 0x80, c=0; j < bits_o; j++, mask_o >>= 1 ){
			if( mask_i & *pi ) c |= mask_o;
			pi += bil_i;
		}
		if( bits_o )	*po++ = c;

		for( j = 0; j < xb_o; j++ ) *po++ = 0;

		mask_i >>= 1;

		if( !mask_i ){
			mask_i = 0x80;
			pi0++;
		}
	}

	for( i = 0; i < xh_o * f_o->bytes_in_line; i++ ) *po++ = 0;
}

#if defined( NO_C_IMAGE_US )
void C_Fontimg_us( Fontimg f_i, Fontimg f_o )
{
	int	wb_i, bil_i, bits_i, mask_i;
	int	wb_o, bil_o, h_o, xh_o, xb_o, bits_o, mask_o;
	int	i, j, c;
	unsigned char*	pi;
	unsigned char*	pi0;
	unsigned char*	po;
	unsigned char*	po0;


	if( !f_o || f_o->w <= 0 || f_o->bytes_in_line <= 0
	    || f_o->h <= 0 || !f_o->img ||
	    !f_i || f_i->w <= 0 || f_i->bytes_in_line <= 0
	    || f_i->h <= 0 || !f_i->img ){
		return;
	}

	wb_o = f_o->w / 8;
	bits_o = f_o->w - wb_o * 8;
	wb_i = f_i->h / 8;
	bits_i = f_i->h - wb_i * 8;

	if( wb_o > wb_i ){
		wb_o = wb_i;
		bits_o = bits_i;
	}else if( wb_o == wb_i && bits_o > bits_i ){
		bits_o = bits_i;
	}

	xb_o = f_o->bytes_in_line - wb_o - 1;
	bil_i = f_i->bytes_in_line;
	bil_o = f_o->h;
	pi0 = f_i->img;
	mask_i = 0x80;
	po0 = f_o->img;
	h_o = ( f_i->w > f_o->h ) ? f_o->h : f_i->w;
	xh_o = f_o->h - h_o;

	for( i = 0; i < h_o; i++, po0++ ){
		po = po0;
		for( j = 0, pi = pi0; j < wb_o; j++ ){
			for( c = 0, mask_o = 0x80; mask_o; mask_o >>= 1 ){
				if( mask_i & *pi ) c |= mask_o;
				pi += bil_i;
			}
			*po = c;
			po += bil_o;
		}

		for( j = 0, mask_o = 0x80, c=0; j < bits_o; j++, mask_o >>= 1 ){
			if( mask_i & *pi ) c |= mask_o;
			pi += bil_i;
		}
		if( bits_o ){
			*po = c;
			po += bil_o;
		}

		for( j = 0; j < xb_o; j++ ){
			*po = 0;
			po += bil_o;
		}

		mask_i >>= 1;

		if( !mask_i ){
			mask_i = 0x80;
			pi0++;
		}
	}

	po0 += h_o;
	for( j = 0; j < f_o->bytes_in_line; j++, po0 += bil_o ){
		for( i = 0, po = po0; i < xh_o; i++ ) *po++ = 0;
	}
}


#if defined (DEBUG)

show_fontdir()
{
    FontTablePtr tbl;
    int d, e;

    for (d = 0; pdir[d]; d++){
	for (e = 0, tbl = &pdir[d]->nonScalable; e < tbl->used ; e++){
	    fprintf(stderr, "type:%d ", tbl->entries[e].type);
	    fprintf(stderr, "name:%s \n", tbl->entries[e].name.name);
	    if( tbl->entries[e].type == 2)
            fprintf(stderr, "        name:%s \n", 
		    tbl->entries[e].u.bitmap.fileName);
	    if( tbl->entries[e].type == 3)
            fprintf(stderr, "        name:%s \n", 
		    tbl->entries[e].u.alias.resolved);
	}
    }
}

#endif /* DEBUG */

#endif /* NO_C_IMAGE_US */

