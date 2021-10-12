static char sccsid[] = "@(#)50	1.6  src/bos/usr/ccs/lib/libIN/LS2killall.c, libIN, bos411, 9428A410j 6/10/91 10:20:39";
/*
 * LIBIN: LS2killall
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
 *	Calls free() on each element of L, leaving L empty.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS2killall(L) register ListDouble *L; {
    while (!LS2isempty(L)) LS2killfirst(L);}
