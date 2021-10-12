static char sccsid[] = "@(#)43	1.2  src/bos/kernel/lib/libsysp/sgetl.c, libsysp, bos411, 9428A410j 6/16/90 02:40:13";
/*
 * LIBLD: sgetl
 *
 * ORIGIN: ATT, IBM
 *
 * Copyright International Business Machines Corp. 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */

/*
 * The intent here is to provide a means to make the value of
 * bytes in an io-buffer correspond to the value of a long
 * in the memory while doing the io a `long' at a time.
 * Files written and read in this way are machine-independent.
 *
 */
#include <values.h>

long
sgetl(register char *buffer)
{
	register long w = 0;
	register int i = BITSPERBYTE * sizeof(long);

	while ((i -= BITSPERBYTE) >= 0)
		w |= (long) ((unsigned char) *buffer++) << i;

	return(w);
}
