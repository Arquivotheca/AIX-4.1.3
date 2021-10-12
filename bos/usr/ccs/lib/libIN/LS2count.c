static char sccsid[] = "@(#)18	1.6  src/bos/usr/ccs/lib/libIN/LS2count.c, libIN, bos411, 9428A410j 6/10/91 10:20:29";
/*
 * LIBIN: LS2count
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
 * FUNCTION: 
 *	Returns the number of elements in list L.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION:
 *	Returns the number of elements in list L.
 */

#include <IN/LSdefs.h>

int LS2count(L) register ListDouble *L; {
    register int count=0;
    register LS2node *e;
    if (L==NULL) return(0);
    LS2generate(L,e,LS2node) count++;
    return(count);}
