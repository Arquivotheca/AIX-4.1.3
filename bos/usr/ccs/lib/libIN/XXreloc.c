static char sccsid[] = "@(#)73	1.6  src/bos/usr/ccs/lib/libIN/XXreloc.c, libIN, bos411, 9428A410j 6/10/91 10:23:06";
/*
 * LIBIN: XXreloc
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
 * FUNCTION: Byte-swap a reloc structure according to the given cpu id.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <gpoff.h>
extern short XXshort();
extern long XXlong();

XXreloc(r,cpu)register struct reloc *r; register cpu;{
    r->r_vaddr=XXlong(r->r_vaddr,cpu);
    r->r_symndx=XXshort(r->r_symndx,cpu);
    r->r_type=XXshort(r->r_type,cpu);}
