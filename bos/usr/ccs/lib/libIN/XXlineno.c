static char sccsid[] = "@(#)62	1.6  src/bos/usr/ccs/lib/libIN/XXlineno.c, libIN, bos411, 9428A410j 6/10/91 10:23:02";
/*
 * LIBIN: XXlineno
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
 * FUNCTION: Byte-swap a lineno structure according to the given cpu id.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <linenum.h>
extern short XXshort();
extern long XXlong();

XXlineno(line,cpu) register struct lineno *line; unsigned char cpu; {
    line->l_lnno=XXshort(line->l_lnno,cpu);
    line->l_addr.l_paddr=XXlong(line->l_addr.l_paddr,cpu);}
