static char sccsid[] = "@(#)07	1.6  src/bos/usr/ccs/lib/libIN/LS2concat.c, libIN, bos411, 9428A410j 6/10/91 10:20:24";
/*
 * LIBIN: LS2concat
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
 *	Services for doubly linked lists with link fields in the first
 *	and second positions in the record.  End of list in both
 *	directions NULL.  Both ends accessible.
 *
 * RETURN VALUE DESCRIPTION: void
 */

#include <IN/LSdefs.h>

LS2concat(L1,L2) register ListDouble *L1,*L2; {
    if (!LS2isempty(L2)) {
       if (LS2isempty(L1)) L1->head=L2->head;
	  else {L1->tail->next=L2->head; L2->head->prev=L1->tail;}
       L1->tail=L2->tail;
       L2->head=L2->tail=NULL;}}
