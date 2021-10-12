static char sccsid[] = "@(#)21	1.6  src/bos/usr/ccs/lib/libIN/LS1concat.c, libIN, bos411, 9428A410j 6/10/91 10:19:48";
/*
 * LIBIN: LS1concat
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
 *	Moves the elements of L2 to the end of L1, and leaves L2 empty.
 *
 *	Services for singly linked lists with link fields in the first
 *	position in the record.  End of list NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS1concat(L1,L2) register ListSingle *L1,*L2; {
    if (L1==NULL || L2==NULL) return;
    if (!LS1isempty(L2)) {
       if (LS1isempty(L1)) L1->head=L2->head; else L1->tail->next=L2->head;
       L1->tail=L2->tail;
       L2->head=L2->tail=NULL;}}
