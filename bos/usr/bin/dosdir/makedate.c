static char sccsid[] = "@(#)13	1.4  src/bos/usr/bin/dosdir/makedate.c, cmdpcdos, bos411, 9428A410j 6/16/90 01:59:31";
/*
 * COMPONENT_NAME: CMDDOS  routines to read dos floppies
 *
 * FUNCTIONS: _DFmakedate 
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

#include "pcdos.h"
/*
 *              gets and converts date and time to PC-DOS format
 *              writes it into timestamp pointed to by input parameter
 *              returns 0
 */
_DFmakedate(dir,clock)
pc_dirent *dir;
long clock;
{
struct tm *tm;
register int timefield, datefield;
register byte *stamp;

	tm = localtime(&clock);

	timefield = ( ( (tm->tm_hour << 11) & 0xf800)
		    | ( (tm->tm_min  <<  5) & 0x07e0)
		    | ( (tm->tm_sec  >>  1) & 0x001f) );

	datefield = ( (( (tm->tm_year - 80) << 9) & 0xfe00)
		    | (( (tm->tm_mon  +  1) << 5) & 0x01e0)
		    |    (tm->tm_mday             & 0x001f) );

	stamp    = dir->df_time;

	*stamp++ =  timefield       & 0xff;
	*stamp++ = (timefield >> 8) & 0xff;
	*stamp++ =  datefield       & 0xff;
	*stamp   = (datefield >> 8) & 0xff;
}
