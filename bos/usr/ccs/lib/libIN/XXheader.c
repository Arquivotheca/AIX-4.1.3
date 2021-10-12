static char sccsid[] = "@(#)52	1.6  src/bos/usr/ccs/lib/libIN/XXheader.c, libIN, bos411, 9428A410j 6/10/91 10:22:57";
/*
 * LIBIN: XXheader
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
 * FUNCTION: Byte-swap an IN.out header according to the cpu id
 *	     contained in it.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <gpoff.h>
extern short XXshort();
extern long XXlong();

XXheader(header) register struct exec *header;{
    register char cpu=header->a_cpu;
    register long *lp;
    header->a_version=XXshort(header->a_version,cpu);
    for (lp = &header->a_text; lp <= &header->a_toffs; lp++) 
	*lp=XXlong(*lp,cpu);}
