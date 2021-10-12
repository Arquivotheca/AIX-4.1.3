static char sccsid[] = "@(#)84	1.6  src/bos/usr/ccs/lib/libIN/XXsyment.c, libIN, bos411, 9428A410j 6/10/91 10:23:09";
/*
 * LIBIN: XXsyment
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
 * FUNCTION: Byte-swap a symbol entry according to the given cpu id.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <gpoff.h>
extern short XXshort();
extern long XXlong();

XXsyment(sym,cpu) register struct syment *sym; register cpu; {
    sym->n_value=XXlong(sym->n_value,cpu);
    sym->n_type=XXshort(sym->n_type,cpu);
    if (sym->n_zeroes==0) sym->n_offset=XXlong(sym->n_offset,cpu);}
