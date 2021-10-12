static char sccsid[] = "@(#)74	1.6  src/bos/usr/ccs/lib/libIN/LS1rmvfirs.c, libIN, bos411, 9428A410j 6/10/91 10:20:12";
/*
 * LIBIN: LS1rmvfirst
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
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: Returns pointer to head of list.
 */

#include <IN/LSdefs.h>

LS1node *LS1rmvfirst(L) register ListSingle *L; {
    register LS1node *p = L->head;
    if (p!=NULL && (L->head=L->head->next)==NULL) L->tail=NULL;
    return(p);}
