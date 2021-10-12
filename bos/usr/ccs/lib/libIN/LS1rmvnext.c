static char sccsid[] = "@(#)85	1.6  src/bos/usr/ccs/lib/libIN/LS1rmvnext.c, libIN, bos411, 9428A410j 6/10/91 10:20:16";
/*
 * LIBIN: LS1rmvnext
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
 *	Removes from list L the element after e and returns a pointer to
 *	it, or NULL if there was no element after e.
 *
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the next element.
 */

#include <IN/LSdefs.h>

LS1node *LS1rmvnext(L,e) register ListSingle *L; register LS1node *e; {
    register LS1node *p;
    if (e==NULL) return(LS1rmvfirst(L));
    p = e->next;
    if (p!=NULL && (e->next=p->next)==NULL) L->tail=e;
    return(p);}
