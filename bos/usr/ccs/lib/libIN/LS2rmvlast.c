static char sccsid[] = "@(#)89	1.6  src/bos/usr/ccs/lib/libIN/LS2rmvlast.c, libIN, bos411, 9428A410j 6/10/91 10:21:05";
/*
 * LIBIN: LS2rmvlast
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
 *	Removes the last element in list L and returns a pointer to it,
 *	or NULL if L was empty.
 *
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the next element.
 */

#include <IN/LSdefs.h>

LS2node *LS2rmvlast(L) register ListDouble *L; {
    register LS2node *p = L->tail;
    if (p==NULL) return(NULL);
    if ((L->tail = p->prev)==NULL) L->head=NULL;
       else L->tail->next=NULL;
    return(p);}

