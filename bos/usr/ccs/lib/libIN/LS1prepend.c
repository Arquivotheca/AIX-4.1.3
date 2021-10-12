static char sccsid[] = "@(#)64	1.6  src/bos/usr/ccs/lib/libIN/LS1prepend.c, libIN, bos411, 9428A410j 6/10/91 10:20:08";
/*
 * LIBIN: LS1prepend
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
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS1prepend(L,e) register ListSingle *L; register LS1node *e; {
    e->next = L->head;
    L->head = e;
    if (L->tail==NULL) L->tail=e;}
