static char sccsid[] = "@(#)58	1.5  src/bos/usr/bin/dosdir/analyze.c, cmdpcdos, bos411, 9428A410j 6/23/93 14:41:36";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _analyze _flip _upcase _hidename _simplify _striplast 
 *
 * ORIGINS: 10,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include "tables.h"
#include <doserrno.h>

int _analyze(path,a)
char     *path;
APATH    *a;
{
    register char *fp,*s;
    char     temp[256];
    register LD ldev,i;

    TRACE(( "analyze: path %s ", path ));

    /*
     *   Isolate device name if specified
     */


    if (path[1]==':' && path[2]!=0) {
       a->device[0] = path[0];
       a->device[1] = path[1];
       a->device[2] = 0;
       path = path+2;
    }
    else *a->device = 0;

    strcpy( a->fpath, path );
    if (_e.pathtype == unix_paths) _flip(a->fpath,'/','\\');

    /*
     *   Scan through pathname for base filename
     */

    a->basename = a->fpath;
    for( fp=a->fpath; *fp!=0; fp++ )
       if (*fp=='\\') a->basename = fp+1;

    /*
     *   Determine path kind
     */

    if ( *--fp == ':' ) {
	 if (a->basename != a->fpath ) err_return( DE_FNAME );
	 strcpy(a->device,a->fpath);
	 *a->fpath = 0;
	 a->pkind = k_device;
    }
    else if ( *a->fpath == '\\' )
	 a->pkind = k_absolute;

    else a->pkind = k_relative;

    /*
     *   Find logical and physical devices
     */

    if ( *a->device==0 && a->basename==a->fpath  
	&& strlen(a->fpath)<5 && strlen(a->fpath)>1)
    {
   	/* don't allow 1 char pathnames to be confused w/devices */
	  /*   Is this a device name in disguise? */
	 strcpy(a->device,a->fpath);
	 _upcase(a->device);
	 strcat(a->device,":");
	 for (ldev=0; ldev<NUM_LOGICAL; ldev++ )
		if(strcmp(ld(ldev).device_id,a->device)==0) {
		    a->pkind = k_device;
		    *a->fpath = 0;
		    goto found;
		}
	 *a->device = 0;
    }

    if ( *a->device==0 ) {      /* none specified, use current disk */
    	strcpy(a->device,ld(_e.current_disk).device_id);
	ldev = _e.current_disk;
    }
    else {                      /* search device table for device */
	_upcase(a->device);
	for (ldev=0; ldev<NUM_LOGICAL; ldev++)
	   if (strcmp(ld(ldev).device_id,a->device)==0) break;
	if (ldev==NUM_LOGICAL) err_return( DE_NODEV );
    }

found:
    a->ldev = ldev;
    if (i=ld(ldev).assigned_ld) a->ldev = i;
    a->pdev = ld(a->ldev).pdevice;

    TRACE(("\tldev=%2d assign ldev=%2d pdev=%2d\n", ldev, a->ldev, a->pdev));

    /*
     *   Mount physical device if necessary
     */

    if ( _mount(a->ldev)<0 ) return(-1);

    /*
     *   Form full DOS pathname
     */

    if( a->pkind == k_relative ) {
	 strcpy( temp, a->fpath );
	 fp = a->fpath;
	 s = ld(a->ldev).current_dir;
	 while (*s) *fp++ = *s++;
	 *fp++ = '\\';
	 s = temp;
	 while (*s) *fp++ = *s++;
	 *fp = 0;
    }
    strcpy(temp,a->fpath);           /* take care of dot and dotdot entries */
    if (_simplify(a->fpath,temp,pd(a->pdev).contents))
	    err_return(DE_FNAME);
    if (*a->fpath=='\0')
	    strcpy(a->fpath,"\\");

    /*
     *   Scan through pathname for base filename
     */

    a->basename = a->fpath;
    for( fp=a->fpath; *fp!=0; fp++ )
       if (*fp=='\\') a->basename = fp+1;

    /*
     *   Special case processing depends on Filesystem type
     *   For UNIX files (is_unix & is_io), form unix pathname
     */

    switch (pd(a->pdev).contents) {

	default:
	     err_return( DE_NODEV );

	case is_dos:
	     _upcase(a->fpath);
	     break;

	case is_io:

	     _upcase(a->fpath);
	     if ( a->pkind != k_device ) err_return( DE_NODEV );
	     strcpy( a->upath, str(pd(a->pdev).attached_file) );
	     TRACE(( "\tupath '%s'\n", a->upath ));
	     break;

	case is_unix:

	     if ( a->pkind == k_device ) err_return( DE_FNAME );

	     /*
	      *   Form full UNIX filename
	      */

	     fp = a->upath;
	     s = str(pd(a->pdev).mount_dir);
	     while(*s) *fp++ = *s++;

	     s = a->fpath;
	     a->basename += fp - s;
	     while(*s) *fp++ = *s++;
	     *fp = 0;
	     _flip(a->upath,'\\','/');

	     if (_e.pathtype == dos_paths) {    /* eliminate trailing "." */
		 if (*--fp == '.')
		    if (*--fp != '/' && *fp != '.') *++fp = 0;
	     }
	     TRACE(( "\tupath '%s'\n", a->upath ));
    }
    TRACE(( "device='%s' fpath='%s' base='%s' pkind=%d\n",
	   a->device,  a->fpath,  a->basename, a->pkind ));

    return(0);


}

_flip(s,c,d)
register char *s,c,d;
{
	while(*s){
		if (*s==c) *s=d;
		s++;
	}
}

_upcase(s)
register char *s;
{
	register char c;
	while(c = *s) {
		if( c>='a' && c<='z') *s += ('A'-'a');
		s++;
	}
}

_hidename(a)
APATH    *a;
{
	/*    modify APATH to identify a UNIX hidden file.
	 *    Return 0 if there is no corresponding hidden file,
	 *    because the basename already begins with a "."
	 */

	register char c,temp,*s;

	s = a->basename;
	if (*s == '.') return(0);          /* name is already hidden       */
	TRACE(("hide: %s\n",s));

	c = '.';                           /* shove it over and insert dot */
	while(c) {
		temp = *s;
		*s++ = c;
		c = temp;
	}
	*s = 0;
	return( 1 ) ;
}

_simplify(d,s,c)
register char *d,*s;
dstate c;
{
	register char *x,*t;
	char *_striplast();
	*d = 0;
	t = d;
	while(1) {
		while (*s=='\\') s++;                /* cat pref(x) to x   */
		if (*s==0) return(0);
		*t++ = '\\';
		x = t;
		while( *s!=0 && *s!='\\' )
			if (c == is_dos)
				*t++ = toupper((int)(*s++));
			else
				*t++ = *s++;
		*t = 0;
		if (strcmp(x,".")==0) t = _striplast(d,t);
		else if (strcmp(x,"..")==0) {
			t = _striplast(d,t);          /* throw away dots    */
			if (*d=='\0')
				return(-1);
			t = _striplast(d,t);          /* throw away prior   */
		}
	}
}

char *_striplast(d,t)
char *d,*t;
{
	/* strip of last part of name of form \\a\\b\\c...       */
	register char *s;
	for( s=t-1; s>=d; --s)
		if (*s=='\\') { *s=0; break; }
	return(s);
}
