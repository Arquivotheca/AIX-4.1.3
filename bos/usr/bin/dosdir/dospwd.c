static char sccsid[] = "@(#)90	1.4  src/bos/usr/bin/dosdir/dospwd.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:58:10";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: dospwd _doscdisk _doscpath 
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
#include "doserrno.h"
char    *malloc();

char    *dospwd(drive)
char *drive;

{
	register char *x,*y,*s;
	register LD ldev;
	register int disk,i;
	char diskname[3];
	APATH a;

	if (drive==0 || *drive==0)
		disk = _e.current_disk;
	else {
		strncpy(diskname,drive,2);
		diskname[2]=0;
		_upcase(diskname);
		for (ldev=0; ldev<NUM_LOGICAL; ldev++)
		  if (strcmp(ld(ldev).device_id,diskname)==0) break;
		if (ldev==NUM_LOGICAL) {
			doserrno = DE_NODEV;
			return(0);
		}
		disk = ldev;
	}
	x = ld( disk ).device_id;

	_analyze( x, &a );

	i = ld( disk ).assigned_ld;
	if (i) disk = i;
	y = ld( disk ).current_dir;
	if (*y==0) y = "\\";

	s = malloc( strlen(x)+strlen(y)+1 );
	strcpy( s, x );
	strcat( s, y );
	TRACE(( "dospwd: disk %d path %s\n",disk,s ));
	return(s);
}
char _doscdisk()
{
	return( toupper((int)*(ld( _e.current_disk).device_id)) );
}
char *_doscpath()
{
static char cpathstr[256];
register char *tp;
register int d;

		d = _e.current_disk;
		tp = ld( d ).device_id;
		cpathstr[0] = toupper((int) *tp);
		cpathstr[1] = ':';
		if (ld( d ).assigned_ld)
			d = ld( d ).assigned_ld;
		tp = ld( d ).current_dir;
		if (*tp==0) tp = "\\";
		strcpy(&cpathstr[2],tp);
		return(cpathstr);
}
