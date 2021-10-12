static char sccsid[] = "@(#)10	1.6  src/bos/usr/ccs/lib/libIN/LS1append.c, libIN, bos411, 9428A410j 6/10/91 10:19:44";
/*
 * LIBIN: LS1append
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
 *	Puts element e at the end of list L.
 *
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible. 
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS1append(L,e) register ListSingle *L; register LS1node *e; {
    e->next=NULL;
    if (L->head==NULL) L->head=e; else L->tail->next=e;
    L->tail=e;}

