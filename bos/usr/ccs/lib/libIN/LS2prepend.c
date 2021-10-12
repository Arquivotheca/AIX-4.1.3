static char sccsid[] = "@(#)56	1.6  src/bos/usr/ccs/lib/libIN/LS2prepend.c, libIN, bos411, 9428A410j 6/10/91 10:20:42";
/*
 * LIBIN: LS2prepend
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
 *	Puts e at the front of list L.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS2prepend(L,e) register ListDouble *L; register LS2node *e; {
    e->next = L->head;
    e->prev = NULL;
    if (L->tail==NULL) L->tail = e; else L->head->prev = e;
    L->head = e;}

