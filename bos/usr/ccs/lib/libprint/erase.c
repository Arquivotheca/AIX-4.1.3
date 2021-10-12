static char sccsid[] = "@(#)25	1.1  src/bos/usr/ccs/lib/libprint/erase.c, libprint, bos411, 9428A410j 9/30/89 15:38:13";
/*
 * COMPONENT_NAME: libplot
 *
 * FUNCTIONS: erase
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


#define SIZEACR     480
#define SIZEDOWN    (72 * 6)
#define SIZEBITS    (SIZEACR / 16 * SIZEDOWN)
extern      short   bitarea[SIZEBITS];
extern      int     printempty;

erase (){
	register    short   *iptr;


	if (!printempty) {
	    doscreen ();
	    for (iptr = &bitarea[SIZEBITS - 1]; iptr >= bitarea; iptr--)
		*iptr = 0;
	    printempty = 0;
	    }
}
