static char sccsid[] = "@(#)78	1.6  src/bos/usr/ccs/lib/libIN/LS2rmvfirs.c, libIN, bos411, 9428A410j 6/10/91 10:20:56";
/*
 * LIBIN: LS2rmvfirst
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
 *	Removes the first element from list L and returns a pointer to it,
 *	or NULL if L was empty.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the next element.
 */

#include <IN/LSdefs.h>

LS2node *LS2rmvfirst(L) register ListDouble *L; {
    register LS2node *p = L->head;
    if (p==NULL) return(NULL);
    if ((L->head = p->next)==NULL) L->tail=NULL;
       else L->head->prev=NULL;
    return(p);}
