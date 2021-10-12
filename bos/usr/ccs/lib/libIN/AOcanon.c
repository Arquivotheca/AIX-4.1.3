static char sccsid[] = "@(#)04	1.8  src/bos/usr/ccs/lib/libIN/AOcanon.c, libIN, bos411, 9428A410j 7/29/92 20:55:51";
/*
 * LIBIN: AOcanon
 *
 * ORIGIN: 9
 *
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1985, 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 *
 * FUNCTION: Returns FALSE if the header has a bad magic number.
 *
 * RETURN VALUE DESCRIPTION: boolean
 */

#include <xcoff.h>

AOcanon(hdr)
	register struct xcoffhdr *hdr;
{
#ifdef _IBMRT
	if (hdr->filehdr.f_magic == U800TOCMAGIC)
		return(1);
#endif
#ifdef _POWER
	if (hdr->filehdr.f_magic == U802TOCMAGIC)
		return(1);
#endif
	return(0);
}
