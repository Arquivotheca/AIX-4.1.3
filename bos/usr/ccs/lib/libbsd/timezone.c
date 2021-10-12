static char sccsid[] = "@(#)43	1.8  src/bos/usr/ccs/lib/libbsd/timezone.c, libbsd, bos411, 9428A410j 7/24/92 15:44:53";
/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS: timezone
 *
 * ORIGINS: 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * The arguments are the number of minutes of time
 * you are westward from Greenwich and whether DST is in effect.
 * It returns a string
 * giving the name of the local timezone.
 *
 */

/*
DESIGN NOTE:
Default BSD action passes back timezone name from the environment
variable TZNAME.  If TZNAME is not set, then the arguments are used
to index the array zonetab below.

For more complete compatibility with AIX, the environment variable
TZ is checked as well before going on to use the default name table.
The code for extracting the timezone names is lifted from the AIX-JLS
version of ctime.c.

The functions used, gettzname() and atosec() are identical to those
in ctime.c.  They really should be busted out of this file and the
ctime.c file and shared so that the format of the TZ environment variable
can change without having to change both of these files.
*/

#include <stdlib.h>

static char *atosec();
static char *gettzname();

static struct zone {
	int	offset;
	char	*stdzone;
	char	*dlzone;
} zonetab[] = {
	-1*60, "MET", "MET DST",	/* Middle European */
	-2*60, "EET", "EET DST",	/* Eastern European */
	4*60, "AST", "ADT",		/* Atlantic */
	5*60, "EST", "EDT",		/* Eastern */
	6*60, "CST", "CDT",		/* Central */
	7*60, "MST", "MDT",		/* Mountain */
	8*60, "PST", "PDT",		/* Pacific */
#ifdef notdef
	/* there's no way to distinguish this from WET */
	0, "GMT", 0,			/* Greenwich */
#endif
	0*60, "WET", "WET DST",		/* Western European */
	-10*60, "EST", "EST",		/* Aust: Eastern */
	-10*60+30, "CST", "CST",	/* Aust: Central */
	-8*60, "WST", 0,		/* Aust: Western */
	-1
};

/*
 *  this code borrowed from ctime.c
 *  Initialize tzname fields allowing for timezones up to length NLTZSIZE
 *  This prevents strcpy() in tzset from overflowing.
 */

#define TZNAME_MAX         255
#define	NLTZSIZE           TZNAME_MAX

static char    tzname[2][NLTZSIZE+1];

char *timezone(zone, dst)
{
	register struct zone *zp;
	static char czone[NLTZSIZE+1];
	char *sign;
	register char *p, *q;
	char *index();

	if (p = getenv("TZNAME")) {
		if (q = index(p, ',')) {
			if (dst)
				return(++q);
			else {
				*q = '\0';
				strncpy(czone, p, sizeof(czone)-1);
				czone[sizeof(czone)-1] = '\0';
				*q = ',';
				return (czone);
			}
		}
		return(p);
	}

	/*
	 * this code borrowed from ctime.c
	 */
	if((p = getenv ("TZ")) && *p) {
		char *gettzname(), *atosec();
		int timezoff;
		while (*p == ':') {
			p++;
		}
		p = gettzname(p,tzname[0]);
		if (!dst) {
			return(tzname[0]);
		}
		p = atosec(p, &timezoff);
		p = gettzname(p,tzname[1]);
		return(tzname[1]);
	}

	for (zp=zonetab; zp->offset!=-1; zp++)
		if (zp->offset==zone) {
			if (dst && zp->dlzone)
				return(zp->dlzone);
			if (!dst && zp->stdzone)
				return(zp->stdzone);
		}
	if (zone<0) {
		zone = -zone;
		sign = "+";
	} else
		sign = "-";
	sprintf(czone, "GMT%s%d:%02d", sign, zone/60, zone%60);
	return(czone);
}


/* this code borrowed from ctime.c */
static
char *atosec(p, result)
char *p;
long *result;
{
	register int n, sign;
	register long v;
	/*
	 *  Convert string of form [+-]hh.mm to seconds
	 */
	if ((sign = (*p == '-')) || *p == '+') {
		p++;
	}
	n = 0;
	while(*p >= '0' && *p <= '9')
		n = (n * 10) + *p++ - '0';
	v = ((long)(n * 60)) * 60;
	if(*p == '.') {
		p++;
		n = 0;
		while(*p >= '0' && *p <= '9')
			n = (n * 10) + *p++ - '0';
		v += n * 60;
	}
	if(sign)
		v = -v;
	*result = v;
	return p;
}


/*
 *       gettzname : this code borrowed from ctime.c
 *
 * POSIX 1003.1-1990, 8.1.1 Extensions to Time Functions
 *   std and dst  Indicates no less than three, nor more than
 *                TZNAME_MAX bytes that are the designation
 *                for the standard (std) or summer (dst) time
 *                zone. ... Any characters except a leading
 *                colon (:),or: digits, the comma (,), the
 *                minus (-), the plus (+), and the null
 *                character are permitted to appear in these
 *                fields, ...
 */

static
char *
gettzname(p, dst)
char *p, *dst;
{
	char *q;
	int i, n;

	/*
	 * copy name into destination buffer
	 */
	n = NLTZSIZE;
	q = dst;
	if ( *p && *p != ':')
	{
		while (*p && n-- > 0 && *p != ',' && *p != '-' && *p != '+' &&
				(*p < '0' || *p > '9') )
		{
			*q++ = *p++;
		}
	}
	*q = '\0';

	/*
	 * if the name is too short, pad it to 3 characters
	 */
	if ( (n = strlen(dst)) < 3 )
	{
		while (n < 3)
		{
			dst[n++] = ' ';
		}
		dst[n] = '\0';
	}
	return p;
}

static
char *
getfield(p,s)
char *p, *s;
{
	while( *p && *p != ':' )
	    *s++ = *p++;
	if (*p == ':')
	    p++;
	*s = 0;
	return p;
}
