static char sccsid[] = "@(#)83	1.4  src/bos/usr/lib/nls/loc/jim/jmnt/HostCode.c, libKJI, bos411, 9428A410j 9/29/93 21:45:22";
/*
 * COMPONENT_NAME : (libKJI) Japanese Input Method (JIM)
 *
 * FUNCTIONS : SjisToEuc
 *
 * ORIGINS : 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991, 1992, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <iconv.h>
#include <errno.h>
#include "kmerror.h"

#define	JIS_CS_IBM930	"IBM-930"
#define	JIS_CS_IBM932	"IBM-932"
#define	SI		0x0e
#define	SO		0x0f

/*----------------------------------------------------------------------*
 *	iconv converter pointer for Host (IBM Kanji) -> SJIS (IBM-932)
 *----------------------------------------------------------------------*/
static	iconv_t		cd = (iconv_t)NULL;
static	int		cd_count = 0;


/************************************************************************
 *	Get iconv converter for Host (IBM Kanji) -> SJIS (IBM-932)
 ************************************************************************/
iconv_t	GetHostCodeConverter()	{ return (cd); }


/************************************************************************
 *	Open iconv converter for Host (IBM Kanji) -> SJIS (IBM-932)
 ************************************************************************/
iconv_t	OpenHostCodeConverter()
{
	cd_count++;
	if (cd == NULL) {
		cd = iconv_open(JIS_CS_IBM932, JIS_CS_IBM930);
	}
	return (cd);
}


/************************************************************************
 *	Close iconv converter for Host (IBM Kanji) -> SJIS (IBM-932)
 ************************************************************************/
void	CloseHostCodeConverter()
{
	cd_count--;
	if (cd_count <= 0) {
		iconv_close(cd);
		cd = (iconv_t)NULL;
	}
}


/************************************************************************
 *	Convert a string from Host (IBM Kanji) into SJIS (IBM-932)
 ************************************************************************/
int	ConvItoS(unsigned char *srcb, unsigned char *dstb)
{
	unsigned char	*srcbuf = srcb, *dstbuf = dstb;
	int		srclen = 0, dstlen = srclen * 2;

	/*
	 *	iconv is not opened
	 */
	if (cd == NULL) {
		return (IMFAIL);
	}

	/*
	 *	reset internal state of iconv
	 */
	iconv(cd, NULL, (size_t *)&srclen, &dstbuf, (size_t *)&dstlen);

	/*
	 *	convert the string
	 */
	srclen = strlen(srcb);
	dstlen = srclen * 2;
	dstbuf = dstb;
	if (iconv(cd, &srcbuf, (size_t *)&srclen,
			&dstbuf, (size_t *)&dstlen) != -1 ) {
		return (IMSUCC);
	}

	return (IMFAIL);
}


/************************************************************************
 *	Convert a number string (IBM Kanji) into Host Code (IBM code)
 ************************************************************************/
int	ConvAtoI(unsigned char *srcb, unsigned char *dstb)
{
	int		i;

	/*
	 *	convert a string to an integer
	 */
	i = atoi(srcb);
	if (i <= 0) {
		return (IMFAIL);
	}
	i += 0x4000;

	/*
	 *	add SI & SO to the string
	 */
	dstb[0] = SI;
	dstb[1] = i >> 8;
	dstb[2] = i & 0xff;
	dstb[3] = SO;
	dstb[4] = '\0';

	return (IMSUCC);
}
