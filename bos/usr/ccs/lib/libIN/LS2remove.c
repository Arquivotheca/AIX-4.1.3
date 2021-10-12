static char sccsid[] = "@(#)67	1.6  src/bos/usr/ccs/lib/libIN/LS2remove.c, libIN, bos411, 9428A410j 6/10/91 10:20:47";
/*
 * LIBIN: LS2remove
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
 *	Remove element e from list L and returns a pointer to the next
 *	element in L, or NULL if element e was the last element.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the next element.
 */

#include <IN/LSdefs.h>

LS2node *LS2remove(L,e) register ListDouble *L; register LS2node *e; {
    if (e==L->head) LS2rmvfirst(L);
    else if (e==L->tail) LS2rmvlast(L);
    else {e->next->prev=e->prev; e->prev->next=e->next;}
    return(e);}

