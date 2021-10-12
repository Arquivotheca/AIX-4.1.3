static char sccsid[] = "@(#)42	1.6  src/bos/usr/ccs/lib/libIN/LS1insert.c, libIN, bos411, 9428A410j 6/10/91 10:19:58";
/*
 * LIBIN: LS1insert
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
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS1insert(L,e,after) register ListSingle *L; register LS1node *e,*after; {
    if (after==NULL) LS1prepend(L,e);
    else if (after==L->tail) LS1append(L,e);
    else {e->next=after->next; after->next=e;}}
