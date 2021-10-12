static char sccsid[] = "@(#)28	1.6  src/bos/usr/ccs/lib/libIN/LS2insaft.c, libIN, bos411, 9428A410j 6/10/91 10:20:33";
/*
 * LIBIN: LS2insafter
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
 *	Inserts e into list L after element a.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS2insafter(L,e,after) register ListDouble *L; register LS2node *e,*after;{
    if (after==NULL) LS2prepend(L,e);
    else if (after==L->tail) LS2append(L,e);
    else {
       e->prev=after; e->next=after->next;
       after->next->prev=e; after->next=e;}}
