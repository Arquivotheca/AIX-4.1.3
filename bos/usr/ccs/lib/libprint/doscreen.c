static char sccsid[] = "@(#)23	1.1  src/bos/usr/ccs/lib/libprint/doscreen.c, libprint, bos411, 9428A410j 9/30/89 15:37:57";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: doscreen
 *
 * ORIGINS: 4,10,27
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
 */


#include    <stdio.h>
#define SIZEACR     480
#define SIZEDOWN    (72 * 6)
#define SIZEBITS    (SIZEACR / 16 * SIZEDOWN)
extern  short   bitarea[SIZEBITS];

doscreen ()
{
	register    int     bytecount;
	register    int     shiftcount;
	register    char    *cptr;
	register    int     x;
	register    int     y;
	register    char    ch;
	char        line[SIZEACR];
	static      int     firsttime = 1;

	if (firsttime)
	    firsttime = 0;
	else
	    putc ('\014', stdout);

	for (y = 0; y < SIZEDOWN; y += 8) {
	    for (x = 0, cptr = line; x < SIZEACR; x++, cptr++) {
		bytecount = y * (SIZEACR / 16) + x / 16;
		shiftcount = 15 - (x % 16);
		ch  = ((bitarea[bytecount + (SIZEACR/16) * 0] >> shiftcount) & 1) << 7;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 1] >> shiftcount) & 1) << 6;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 2] >> shiftcount) & 1) << 5;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 3] >> shiftcount) & 1) << 4;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 4] >> shiftcount) & 1) << 3;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 5] >> shiftcount) & 1) << 2;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 6] >> shiftcount) & 1) << 1;
		ch |= ((bitarea[bytecount + (SIZEACR/16) * 7] >> shiftcount) & 1) << 0;
		*cptr = ch;
		}
	    for (cptr--; cptr >= line; cptr--)
		if (*cptr != 0)
		    break;

	    x = cptr - line + 1;
	    if (x > 0) {
		fprintf (stdout, "\033K%c%c", x & 0377, (x >> 8) & 0377);
		for (cptr = line; cptr < &line[x]; cptr++)
		    putc (*cptr, stdout);
		}

	    putc ('\n', stdout);
	    }

	fflush (stdout);
	}


