static char sccsid[] = "@(#)39	1.6  src/bos/usr/ccs/lib/libIN/LS2insbef.c, libIN, bos411, 9428A410j 6/10/91 10:20:36";
/*
 * LIBIN: LS2insbefore
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
 *	Inserts element e into list before element before.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS2insbefore(L,e,before) ListDouble *L; LS2node *e,*before; {
    if (before==NULL) LS2append(L,e);
    else if (before==L->head) LS2prepend(L,e);
    else {
       e->next=before; e->prev=before->prev;
       before->prev->next=e; before->prev=e;}}
